
#include <QCoreApplication>
#include <QFile>
#include "cameraproxy.h"
#include "encoder_jpeg.h"
#include "settings.h"
#include "viewfinder2d.h"
#include "viewfinder3d.h"
#include "viewfindergl.h"

QDebug operator<< (QDebug d, const libcamera::Size &sz) {
    d << "Size:" << sz.width << "x" << sz.height;
    return d;
}

QDebug operator<< (QDebug d, const std::vector<libcamera::Size> &vs) {
    for(const libcamera::Size s : vs) {
        d << "Size:" << s.width << "x" << s.height;
    }
    return d;
}

QDebug operator<< (QDebug d, const std::map<libcamera::PixelFormat, std::vector<libcamera::Size> > &fm) {
    for(const auto &pf : fm) {
        d << "Format:" << QString::fromStdString(pf.first.toString()) << pf.second;
    }
    return d;
}

CameraProxy::CameraProxy(QObject *parent)
    : QObject{parent}
{
    qDebug() << Q_FUNC_INFO;
}

CameraProxy::~CameraProxy()
{
    qDebug() << Q_FUNC_INFO;
    m_currentCamera->stop();
    m_cameraManager.reset();

    if (m_viewFinderStream) m_allocator->free(m_viewFinderStream);
    if (m_stillStream) m_allocator->free(m_stillStream);
    delete m_allocator;
}

bool CameraProxy::event(QEvent *e)
{
    //qDebug() << Q_FUNC_INFO;
    if (e->type() == CaptureEvent::type()) {
        processCapture();
        return true;
    }

    return QObject::event(e);
}

void CameraProxy::setCameraManager(std::shared_ptr<libcamera::CameraManager> cm)
{
    qDebug() << Q_FUNC_INFO;
    m_cameraManager = cm;
}

void CameraProxy::setSettings(Settings *settings)
{
    qDebug() << Q_FUNC_INFO;
    m_settings = settings;
}

QStringList CameraProxy::supportedFormats() const
{
    qDebug() << Q_FUNC_INFO;

    QStringList f;
    for(const auto &pf : m_stillFormats) {
        f << QString::fromStdString(pf.first.toString());
    }
    return f;
}


void CameraProxy::setStillFormat(const QString &format)
{
    qDebug() << Q_FUNC_INFO << format;

    CameraState oldstate = state();

    stop();

    m_currentStillFormat = format;

    Q_EMIT formatChanged();

    if (oldstate == CapturingViewFinder) {
        startViewFinder();
    }
}

QString CameraProxy::currentStillFormat() const
{
    qDebug() << Q_FUNC_INFO;

    return m_currentStillFormat;
}

void CameraProxy::setResolution(const QSize &res)
{
    qDebug() << Q_FUNC_INFO << res;

    CameraState oldstate = state();

    stop();

    m_currentStillResolution = libcamera::Size(res.width(), res.height());

    Q_EMIT resolutionChanged();

    if (oldstate == CapturingViewFinder) {
        startViewFinder();
    }
}

void CameraProxy::setFaceDetectionEnabled(bool enabled)
{
    m_enableFaceDetection = enabled;

    if (!m_enableFaceDetection) {
        m_rects.clear();
    }
}

std::vector<libcamera::Size> CameraProxy::supportedResoluions(QString format)
{
    return m_stillFormats[libcamera::PixelFormat::fromString(format.toStdString())];
}

libcamera::ControlInfoMap CameraProxy::supportedControls() const
{
    if (m_currentCamera) {
        return m_currentCamera->controls();
    }
    return libcamera::ControlInfoMap();
}

void CameraProxy::setCameraIndex(QString id)
{
    qDebug() << Q_FUNC_INFO;
    stop();
    if (m_cameraManager && !id.isEmpty()) {
        m_currentCameraId = id;
        const std::shared_ptr<libcamera::Camera> &cam = m_cameraManager->get(id.toStdString());

        if (!cam || cam->acquire()) {
            qInfo() << "Failed to acquire camera" << id;
            return;
        }

        qInfo() << "Switching to camera" << cam->id().c_str();

        if (m_currentCamera) {
            m_currentCamera->release();
        }

        m_currentCamera = cam;
        cacheFormats(libcamera::StreamRole::Viewfinder);
        cacheFormats(libcamera::StreamRole::StillCapture);

        //Print controls
        qDebug() << "Controls:";
        auto controls = m_currentCamera->controls();

        for(const auto &control: controls) {
            qDebug() << "Control:" << control.first->id() << control.first->type() << QString::fromStdString(control.first->name()) <<  QString::fromStdString(control.second.toString());
            for (const auto &val : control.second.values()) {
                qDebug() << "Value: " << QString::fromStdString(val.toString());
            }
        }

        Q_EMIT cameraChanged();
    }
}

bool CameraProxy::buildConfiguration(std::initializer_list<libcamera::StreamRole> roles, bool configure)
{
    qDebug() << Q_FUNC_INFO << roles.size();

    if (!m_currentCamera) {
        return false;
    }

    //Configure the camera for view finder
    m_config = m_currentCamera->generateConfiguration(roles);

    if (!m_config) {
        //Configure for viewfinder only
        m_config = m_currentCamera->generateConfiguration({libcamera::StreamRole::Viewfinder});
    }
    bool multiConfig = m_config->size() == 2;

    if (multiConfig) {
        qDebug() << "Multi-stream config....";
        m_stillStreamConfig = &m_config->at(0);
        m_vfStreamConfig = &m_config->at(1);
    } else {
        if (roles.size() == 2) {
            m_singleStream = true;
            qDebug() << "Device can only handle a single stream";
            return false;
        }
        if (roles.begin()[0] == libcamera::StreamRole::Viewfinder) {
            m_vfStreamConfig = &m_config->at(0);
            m_stillStreamConfig = nullptr;
        } else {
            m_vfStreamConfig = nullptr;
            m_stillStreamConfig = &m_config->at(0);
        }
    }

    if (!configure) {
        return true;
    }

    if (!configureCamera()) {
        qInfo() << "Failed to configure camera";
        return false;
    }

    m_viewFinderStream = m_vfStreamConfig ? m_vfStreamConfig->stream() : nullptr;
    m_stillStream = m_stillStreamConfig ? m_stillStreamConfig->stream() : nullptr;

    /* Allocate and map buffers. */
    m_allocator = new libcamera::FrameBufferAllocator(m_currentCamera);

    qDebug() << Q_FUNC_INFO << m_vfStreamConfig << m_viewFinderStream << m_stillStreamConfig << m_stillStream;

    return true;
}

bool CameraProxy::configureCamera()
{
    qDebug() << Q_FUNC_INFO << m_config->size();

    if (m_stillStreamConfig) {
        m_stillStreamConfig->size = m_currentStillResolution;
        m_stillStreamConfig->pixelFormat = libcamera::PixelFormat::fromString(m_currentStillFormat.toStdString());
    }

    if (m_vfStreamConfig) {
        m_vfStreamConfig->size = bestViewfinderResolution(m_vfStreamConfig->pixelFormat, m_currentStillResolution);
        m_vfStreamConfig->pixelFormat = bestViewFinderFormat(libcamera::PixelFormat::fromString(m_currentStillFormat.toStdString()), m_viewFinder->nativeFormats());
    }

    libcamera::CameraConfiguration::Status validation = m_config->validate();

    if (validation == libcamera::CameraConfiguration::Invalid) {
        qWarning() << "Failed to create valid camera configuration";
        return false;
    }

    if (validation == libcamera::CameraConfiguration::Adjusted) {
        qInfo() << "Stream configuration adjusted to "
                << (m_vfStreamConfig ? m_vfStreamConfig->toString().c_str() : "")
                << (m_stillStreamConfig ? m_stillStreamConfig->toString().c_str() : "");
    }

    if (m_currentCamera->configure(m_config.get()) < 0) {
        qInfo() << "Failed to configure camera";
        return false;
    }

    return true;
}

void CameraProxy::cacheFormats(libcamera::StreamRole role)
{
    qDebug() << Q_FUNC_INFO << (int)role;

    if (role == libcamera::StreamRole::Viewfinder) {
        m_viewFinderFormats.clear();
        if (!buildConfiguration({libcamera::StreamRole::Viewfinder})) {
            return;
        }

        for (const libcamera::PixelFormat format : m_vfStreamConfig->formats().pixelformats()) {
            m_viewFinderFormats[format] = m_vfStreamConfig->formats().sizes(format);
        }
        qDebug() << "VF Formats:" << m_viewFinderFormats;
    }

    if (role == libcamera::StreamRole::StillCapture) {
        m_stillFormats.clear();
        if (!buildConfiguration({libcamera::StreamRole::StillCapture})) {
            return;
        }

        for (const libcamera::PixelFormat format : m_stillStreamConfig->formats().pixelformats()) {
            m_stillFormats[format] = m_stillStreamConfig->formats().sizes(format);
        }
        qDebug() << "Still Formats:" << m_stillFormats;
    }
}

libcamera::Size CameraProxy::bestViewfinderResolution(const libcamera::PixelFormat format, const libcamera::Size stillSize)
{
    std::vector<libcamera::Size> sizesForFormat;
    libcamera::Size bestSize;
    libcamera::Size backupSize;

    size_t minMpx = 307200; //640x480
    float stillAspect = (float)stillSize.width / (float)stillSize.height;

    qDebug() << "Looking for best VF resolution for " << stillSize.toString().c_str();

    //Try to find the best VF resolution for the given still resolution
    //Criteria:
    //  Needs to match (or close) the aspect ration
    //  Needs to be at-least above the minimum pixel count
    if (auto search = m_viewFinderFormats.find(format); search != m_viewFinderFormats.end()) {
        sizesForFormat = search->second;

        for (auto sz : sizesForFormat) {
            qDebug() << "Checking " << sz.toString().c_str();

            float aspect = (float)sz.width / (float)sz.height;
            size_t mpx = sz.width * sz.height;

            if (mpx >= minMpx && abs(aspect - stillAspect) < 0.1) {
                bestSize = sz;
                goto end;
            }
            if (mpx >= minMpx && abs(aspect - stillAspect) < 0.3) {
                backupSize = sz;
            }
        }
    }

    if (bestSize.isNull()) {
        bestSize = backupSize;
    }
end:
    qDebug() << "Picked " << bestSize.toString().c_str();

    return bestSize;
}

libcamera::PixelFormat CameraProxy::bestViewFinderFormat(const libcamera::PixelFormat preferred, const QList<libcamera::PixelFormat> supported)
{
    qDebug() << Q_FUNC_INFO << preferred << supported;

    bool match = supported.contains(preferred);
    if (match) {
        return preferred;
    }

    for (const libcamera::PixelFormat &format : supported) {
        qDebug() << "Checking if format is supported " << format.toString().c_str();

        auto match = m_viewFinderFormats.find(format);
        if (match != m_viewFinderFormats.end()) {
            return format;
        }
    }

    return preferred;
}

void CameraProxy::setViewFinder(ViewFinder *vf)
{
    qDebug() << Q_FUNC_INFO << vf;
    m_viewFinder = vf;

    ViewFinder2D *vf2d = dynamic_cast<ViewFinder2D*>(vf);
    if (vf2d) {
        connect(vf2d, &ViewFinder2D::renderComplete, this, &CameraProxy::renderComplete);
    }

    ViewFinder3D *vf3d = dynamic_cast<ViewFinder3D*>(vf);
    if (vf3d) {
        connect(vf3d, &ViewFinder3D::renderComplete, this, &CameraProxy::renderComplete);
    }

    ViewFinderGL *vfgl = dynamic_cast<ViewFinderGL*>(vf);
    if (vfgl) {
        connect(vfgl, &ViewFinderGL::renderComplete, this, &CameraProxy::renderComplete);
    }
}

void CameraProxy::startViewFinder()
{
    qDebug() << Q_FUNC_INFO << m_vfStreamConfig;
    int ret;

    if (m_state == CapturingViewFinder) {
        qDebug() << "Already capturing viewfinder";
        return;
    }

    if (m_state != Stopped) {
        stop();
    }
    setState(ConfiguringViewFinder);

    qDebug() << "View finder formats: ";
    for (auto const &f : m_viewFinderFormats) {
        qDebug() << f.first.toString().c_str();
    }

    if (!buildConfiguration({libcamera::StreamRole::StillCapture, libcamera::StreamRole::Viewfinder}, true)) {
        if (!buildConfiguration({libcamera::StreamRole::Viewfinder}, true)) {
            qInfo() << "Failed to build configuration";
            return;
        }
    }

    if (!m_viewFinder) {
        return;
    }

    // Configure the viewfinder. If no color space is reported, default to sYCC.
    ret = m_viewFinder->setFormat(m_vfStreamConfig->pixelFormat,
                                  QSize(m_vfStreamConfig->size.width, m_vfStreamConfig->size.height),
                                  m_vfStreamConfig->colorSpace.value_or(libcamera::ColorSpace::Sycc),
                                  m_vfStreamConfig->stride);
    if (ret < 0) {
        qInfo() << "Failed to set viewfinder format";
        return;
    }

    for (libcamera::StreamConfiguration &config : *m_config) {
        libcamera::Stream *stream = config.stream();
        if (!stream) {
            return;
        }
        qDebug() << "Allocating buffer for stream " << stream->configuration().toString().c_str() << stream << "VF Ptr:" << m_viewFinderStream << "Still Ptr:" << m_stillStream;

        ret = m_allocator->allocate(stream);
        if (ret < 0) {
            qWarning() << "Failed to allocate capture buffers";
            return;
        }

        for (const std::unique_ptr<libcamera::FrameBuffer> &buffer : m_allocator->buffers(stream)) {
            qDebug() << "Mapping buffer " << buffer.get() << " for " << stream->configuration().toString().c_str();
            /* Map memory buffers and cache the mappings. */
            std::unique_ptr<Image> image = Image::fromFrameBuffer(buffer.get(), Image::MapMode::ReadOnly);
            assert(image != nullptr);
            m_mappedBuffers[buffer.get()] = std::move(image);

            /* Store buffers on the free list. */
            m_freeBuffers[stream].enqueue(buffer.get());
        }
    }

    m_requests.clear();

    qDebug() << "VF Stream Ptr:" << m_viewFinderStream;

    for (const auto &s : m_freeBuffers) {
        qDebug() << s.first->configuration().toString().c_str();
    }

    /* Create requests and fill them with buffers from the viewfinder. */
    while (!m_freeBuffers[m_viewFinderStream].isEmpty()) {
        qDebug() << "Creating vf requests...";
        libcamera::FrameBuffer *buffer = m_freeBuffers[m_viewFinderStream].dequeue();
        qDebug() << "Adding buffer" << buffer << "to VF requests";

        std::unique_ptr<libcamera::Request> request = m_currentCamera->createRequest();
        if (!request) {
            qWarning() << "Can't create request";
            ret = -ENOMEM;
            return;
        }

        ret = request->addBuffer(m_viewFinderStream, buffer);
        if (ret < 0) {
            qWarning() << "Can't set vf buffer for request";
        }
        if (!m_singleStream) {
            qDebug() << "Can handle 2 streams";
            libcamera::FrameBuffer *stillBuffer = m_freeBuffers[m_stillStream].dequeue();
            if (stillBuffer) {
                qDebug() << "Adding buffer" << stillBuffer << "to VF requests";
                ret = request->addBuffer(m_stillStream, stillBuffer);
                if (ret < 0) {
                    qWarning() << "Can't set still buffer for request";
                }
            }
        }
        m_requests.push_back(std::move(request));
    }

    ret = m_currentCamera->start();
    if (ret) {
        qInfo() << "Failed to start capture";
        return;
    }
    setState(CapturingViewFinder);

    m_currentCamera->requestCompleted.connect(this, &CameraProxy::requestComplete);

    /* Queue all requests. */
    for (std::unique_ptr<libcamera::Request> &request : m_requests) {
        ret = m_currentCamera->queueRequest(request.get());
        if (ret < 0) {
            qWarning() << "Can't queue request";
            return;
        }
    }
}

void CameraProxy::stop()
{
    qDebug() << Q_FUNC_INFO;
    if (m_currentCamera) {
        qDebug() << "stopping";
        setState(Stopping);

        m_currentCamera->stop();
        m_viewFinder->stop();

        m_currentCamera->requestCompleted.disconnect(this);

        m_mappedBuffers.clear();
        m_requests.clear();
        m_freeQueue.clear();

        delete m_allocator;
        m_allocator = nullptr;

        m_freeBuffers.clear();
        m_doneQueue.clear();
        setState(Stopped);
    }
}

void CameraProxy::stillCapture(const QString &filename)
{
    qDebug() << Q_FUNC_INFO;

    m_saveFileName = filename;

    if (!m_singleStream) {
        m_captureStill = true;
    } else {
        m_frame = 0;
        int ret;

        stop();

        buildConfiguration({libcamera::StreamRole::StillCapture}, true);

        libcamera::CameraConfiguration::Status validation = m_config->validate();
        if (validation == libcamera::CameraConfiguration::Invalid) {
            qWarning() << "Failed to create valid camera configuration";
            return;
        }

        if (validation == libcamera::CameraConfiguration::Adjusted) {
            qInfo() << "Stream configuration adjusted to "
                    << m_stillStreamConfig->toString().c_str();
        }

        ret = m_allocator->allocate(m_stillStream);
        if (ret < 0) {
            qWarning() << "Failed to allocate still capture buffers";
            //TODO got error;
            return;
        }

        qDebug() << "Creating still buffers";
        for (const std::unique_ptr<libcamera::FrameBuffer> &buffer : m_allocator->buffers(m_stillStream)) {
            qDebug() << "Still Mapping buffer " << buffer.get();
            /* Map memory buffers and cache the mappings. */
            std::unique_ptr<Image> image = Image::fromFrameBuffer(buffer.get(), Image::MapMode::ReadOnly);
            assert(image != nullptr);
            m_mappedBuffers[buffer.get()] = std::move(image);

            /* Store buffers on the free list. */
            m_freeBuffers[m_stillStream].enqueue(buffer.get());
        }

        m_requests.clear();

        /* Create requests and fill them with buffers from the still stream. */
        while (!m_freeBuffers[m_stillStream].isEmpty()) {
            qDebug() << "Creating still request...";
            libcamera::FrameBuffer *buffer = m_freeBuffers[m_stillStream].dequeue();

            std::unique_ptr<libcamera::Request> request = m_currentCamera->createRequest();
            if (!request) {
                qWarning() << "Can't create request";
                ret = -ENOMEM;
                return;
            }

            ret = request->addBuffer(m_stillStream, buffer);
            if (ret < 0) {
                qWarning() << "Can't set buffer for request";
                return;
            }

            m_requests.push_back(std::move(request));
        }

        ret = m_currentCamera->start();
        if (ret) {
            qInfo() << "Failed to start capture";
            return;
        }

        setState(CapturingStill);

        m_currentCamera->requestCompleted.connect(this, &CameraProxy::requestComplete);

        /* Queue all requests. */
        for (std::unique_ptr<libcamera::Request> &request : m_requests) {
            ret = m_currentCamera->queueRequest(request.get());
            if (ret < 0) {
                qWarning() << "Can't queue request";
                return;
            }
        }
    }}

bool CameraProxy::controlExists(CameraProxy::Control c)
{
    qDebug() << Q_FUNC_INFO << c;
    if (m_currentCamera && m_currentCamera->controls().size() > 0) {
        qDebug() << (m_currentCamera->controls().find(c) != m_currentCamera->controls().end());
        return m_currentCamera->controls().find(c) != m_currentCamera->controls().end();
    }
    return false;
}

float CameraProxy::controlMin(CameraProxy::Control c)
{
    if (!controlExists(c)) {
        return 0;
    }
    auto control = m_currentCamera->controls().find(c);

    if (control == m_currentCamera->controls().end()) {
        return 0;
    }
    switch(control->first->type()) {
    case libcamera::ControlTypeFloat:
        return control->second.min().get<float>();
    case libcamera::ControlTypeInteger32:
        return control->second.min().get<int32_t>();
    case libcamera::ControlTypeInteger64:
        return control->second.min().get<int64_t>();
    }

    return 0;
}

float CameraProxy::controlMax(CameraProxy::Control c)
{
    if (!controlExists(c)) {
        return 0;
    }
    auto control = m_currentCamera->controls().find(c);

    if (control == m_currentCamera->controls().end()) {
        return 0;
    }
    switch(control->first->type()) {
    case libcamera::ControlTypeFloat:
        return control->second.max().get<float>();
    case libcamera::ControlTypeInteger32:
        return control->second.max().get<int32_t>();
    case libcamera::ControlTypeInteger64:
        return control->second.max().get<int64_t>();
    }

    return 0;
}

float CameraProxy::controlValue(CameraProxy::Control c)
{
    if (!controlExists(c)) {
        return 0;
    }
    float v = 0;
    for (std::unique_ptr<libcamera::Request> &request : m_requests) {
        auto controllist = request->controls();
        v = controllist.get(c).get<float>();
    }
    qDebug() << Q_FUNC_INFO << c << v;

    return v;
}

void CameraProxy::setControlValue(CameraProxy::Control c, ControlType type, QVariant val)
{
    qDebug() << Q_FUNC_INFO << c << type << val << controlMin(c) << controlMax(c);

    float val_f = val.toFloat();
    libcamera::ControlValue v;
    if (controlExists(c)) {
        if (type == ControlTypeFloat && val_f <= controlMax(c) && val_f >= controlMin(c)) {
            qDebug() << "Setting float value";
            v.set<float>(val.toFloat());
            m_controlValues[c] = v;
        } else if (type == ControlTypeInteger32 && val_f <= controlMax(c) && val_f >= controlMin(c)) {
            qDebug() << "Setting int32 value";
            v.set<int32_t>(val.toInt());
            m_controlValues[c] = v;
        } else if (type == ControlTypeInteger64 && val_f <= controlMax(c) && val_f >= controlMin(c)) {
            qDebug() << "Setting int64 value";
            v.set<int64_t>(val.toInt());
            m_controlValues[c] = v;
        } else if (type == ControlTypeBool && val.canConvert<bool>()) {
            qDebug() << "Setting bool value";
            v.set<bool>(val.toBool());
            m_controlValues[c] = v;
        }
    } else {
        qWarning() << "Control " <<  c << " doesnt exist";
    }
}

void CameraProxy::removeControlValue(Control c)
{
    m_controlValues.erase(c);
}

CameraProxy::CameraState CameraProxy::state() const
{
    return m_state;
}

void CameraProxy::setState(CameraState newState)
{
    qDebug() << Q_FUNC_INFO << newState;
    m_state = newState;
    Q_EMIT stateChanged();
}

//==================== Request / event handling ===========================

void CameraProxy::requestComplete(libcamera::Request *request)
{
    qDebug() << Q_FUNC_INFO;

    if (request->status() == libcamera::Request::RequestCancelled) {
        qDebug() << "Request cancelled";
        return;
    }

    /*
     * We're running in the libcamera thread context, expensive operations
     * are not allowed. Add the buffer to the done queue and post a
     * CaptureEvent for the application thread to handle.
     */
    {
        QMutexLocker locker(&m_mutex);
        m_doneQueue.enqueue(request);
    }

    QCoreApplication::postEvent(this, new CaptureEvent);
}

void CameraProxy::processCapture()
{
    //qDebug() << Q_FUNC_INFO;
    if (m_state == Stopped) {
        qDebug() << "dont process event if camera stopped";
        return;
    }
    /*
     * Retrieve the next buffer from the done queue. The queue may be empty
     * if stopCapture() has been called while a CaptureEvent was posted but
     * not processed yet. Return immediately in that case.
    */
    libcamera::Request *request;
    {
        QMutexLocker locker(&m_mutex);
        if (m_doneQueue.isEmpty())
            return;

        request = m_doneQueue.dequeue();
    }

    // Process buffer
    qDebug() << "VF Buffers" << request->buffers().count(m_viewFinderStream) << " Still buffers " << request->buffers().count(m_stillStream);
    processStill(request->findBuffer(m_stillStream));
    processViewfinder(request->findBuffer(m_viewFinderStream));

    if (m_state <= Stopping) {
        return;
    }

    request->reuse();
    QMutexLocker locker(&m_mutex);
    m_freeQueue.enqueue(request);
}

void CameraProxy::processViewfinder(libcamera::FrameBuffer *buffer)
{
    qDebug() << Q_FUNC_INFO << buffer;
    if (!buffer) return;

    //qDebug() << Q_FUNC_INFO << "Buffer request:" << buffer << buffer->request();//->toString().c_str();

    Image *i = m_mappedBuffers[buffer].get();
    QList<QRectF> rects;

    if (m_enableFaceDetection) {
        rects = m_fd.detect(m_viewFinder->getCurrentImage());
        if (rects.length() > 0) {
            m_rects = rects;
            m_rectDelay = 30;
        } else {
            if (m_rectDelay > 0) {
                m_rectDelay--;
            }
            if (m_rectDelay == 0) {
                m_rects.clear();
            }
        }
    }

    m_viewFinder->render(buffer, i, m_rects);
    m_freeBuffers[m_viewFinderStream].enqueue(buffer);
}

void CameraProxy::processStill(libcamera::FrameBuffer *buffer)
{
    qDebug() << Q_FUNC_INFO << m_saveFileName << m_frame << buffer;

    if (!buffer) {
        m_saveFileName.clear();
        return;
    }

    if (m_state == CapturingStill && m_singleStream && m_frame < 4) {
        qDebug() << "Skipping frame " << m_frame;
        m_frame++;
        if (buffer) {
            m_freeBuffers[m_stillStream].enqueue(buffer);
        }
        renderComplete();
        return;
    }

    if (!m_saveFileName.isEmpty()) {
        QFile file(m_saveFileName);
        if (!file.open(QIODevice::WriteOnly)) {
            return;
        }

        size_t totalSize = 0;
        for (uint plane = 0; plane < buffer->metadata().planes().size(); ++plane) {
            totalSize += buffer->metadata().planes()[plane].bytesused;
        }

        file.write((const char*)m_mappedBuffers[buffer].get()->data(0).data(), totalSize);
        file.close();

        {
            QMutexLocker locker(&m_mutex);
            EncoderJpeg jpeg;
            libcamera::StreamConfiguration *config = m_stillStreamConfig;//m_config->at(m_singleStream ? 0 : 1);
            bool ok = jpeg.encode(*config, buffer, m_mappedBuffers[buffer].get(), QString(m_saveFileName + QStringLiteral(".jpg")).toStdString());
            if (!ok) {
                qDebug() << "Unable to save jpeg file";
            }
            qDebug() << "Saved JPEG as " << QString(m_saveFileName + QStringLiteral(".jpg"));
        }
        Q_EMIT stillCaptureFinished(m_saveFileName + QStringLiteral(".jpg"));
        m_saveFileName.clear();
    }
    m_freeBuffers[m_stillStream].enqueue(buffer);
}

void CameraProxy::renderComplete()
{
    qDebug() << Q_FUNC_INFO << m_state << m_viewFinderStream << m_stillStream;

    static bool firstStill = true;
    bool requestUsed = false;

    if (m_state == Stopped || m_state == Stopping) {
        return;
    }

    // Create a new request for future buffers
    libcamera::Request *newRequest;
    {
        QMutexLocker locker(&m_mutex);
        if (m_freeQueue.isEmpty()) {
            qDebug() << "Free queue empty";
            return;
        }

        newRequest = m_freeQueue.dequeue();
    }

    libcamera::FrameBuffer *vfBuffer = nullptr;
    {
        QMutexLocker locker(&m_mutex);
        if (!m_freeBuffers[m_viewFinderStream].isEmpty()) {
            vfBuffer = m_freeBuffers[m_viewFinderStream].dequeue();
        }
    }

    libcamera::FrameBuffer *stillBuffer = nullptr;
    {
        QMutexLocker locker(&m_mutex);
        if (!m_freeBuffers[m_stillStream].isEmpty()) {
            stillBuffer = m_freeBuffers[m_stillStream].dequeue();
        }
    }

    if (m_state == CapturingViewFinder && vfBuffer) {
        newRequest->addBuffer(m_viewFinderStream, vfBuffer);
        for(auto c : m_controlValues) {
            if (c.first) {
                newRequest->controls().set(c.first, c.second);
            }
        }
        requestUsed = true;
    }

    if (m_singleStream) {
        if ( m_state == CapturingStill && m_frame < 4) {
            newRequest->addBuffer(m_stillStream, stillBuffer);
            requestUsed = true;
        }
    } else if (m_state == CapturingViewFinder) {
        qDebug() << "Submitting request for still image " << m_stillStream->configuration().toString().c_str();

        if (stillBuffer) {
            int ret = newRequest->addBuffer(m_stillStream, stillBuffer);
            requestUsed = true;
            if (ret < 0) {
                qWarning() << "Can't set buffer for request, will try again";
            }

        } else {
            qWarning() << "No free buffer available for Still capture";
        }

        if (firstStill) {
            qInfo() << "First still frame, will queue another buffer";
            if (!m_freeBuffers[m_stillStream].isEmpty()) {
                stillBuffer = m_freeBuffers[m_stillStream].dequeue();
            }

            if (stillBuffer) {
                int ret = newRequest->addBuffer(m_stillStream, stillBuffer);
                requestUsed = true;
                if (ret < 0) {
                    qWarning() << "Can't set buffer for request";
                }
            } else {
                qWarning() << "No free buffer available for Still capture";
            }
            firstStill = false;
        }

        m_captureStill = false;
    }

    if (!requestUsed) {
        m_freeQueue.enqueue(newRequest);
    } else {
        m_currentCamera->queueRequest(newRequest);
    }
}
