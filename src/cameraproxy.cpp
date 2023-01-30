#include "cameraproxy.h"
#include <QDebug>
#include <QCoreApplication>

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
    for(auto pf : fm) {
        d << "Format:" << QString::fromStdString(pf.first.toString()) << pf.second;
    }
    return d;
}

CameraProxy::CameraProxy(QObject *parent)
    : QObject{parent}
{
    qDebug() << Q_FUNC_INFO;
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

QStringList CameraProxy::supportedFormats()
{
    qDebug() << Q_FUNC_INFO;

    QStringList f;
    for(auto pf : m_stillFormats) {
        f << QString::fromStdString(pf.first.toString());
    }
    return f;
}

void CameraProxy::setStillFormat(const QString &format)
{
    qDebug() << Q_FUNC_INFO;

    m_currentStillFormat = format;
    formatChanged();
}

QString CameraProxy::currentStillFormat()
{
    qDebug() << Q_FUNC_INFO;

    return m_currentStillFormat;
}

void CameraProxy::setResolution(const QSize &res)
{
    qDebug() << Q_FUNC_INFO;
    m_currentStillResolution = res;
    resolutionChanged();
}

std::vector<libcamera::Size> CameraProxy::supportedReoluions(QString format)
{
    return m_stillFormats[libcamera::PixelFormat::fromString(format.toStdString())];
}
\
void CameraProxy::setViewFinder(ViewFinder2D *vf)
{
    qDebug() << Q_FUNC_INFO << vf;
    m_viewFinder = vf;
    connect(m_viewFinder, &ViewFinder2D::renderComplete,
            this, &CameraProxy::renderComplete);
}

void CameraProxy::setCameraIndex(QString id)
{
    qDebug() << Q_FUNC_INFO;
    stop();
    if (m_cameraManager && !id.isEmpty()) {
        m_currentCameraId = id;
        const std::shared_ptr<libcamera::Camera> &cam = m_cameraManager->get(id.toStdString());


        if (cam->acquire()) {
            qInfo() << "Failed to acquire camera" << cam->id().c_str();
            return;
        }

        qInfo() << "Switching to camera" << cam->id().c_str();

        if (m_currentCamera) {
            m_currentCamera->release();
        }

        m_currentCamera = cam;
        cacheFormats();

        //Print controls
        qDebug() << "Controls:";
        auto controls = m_currentCamera->controls();

        for(auto control: controls) {
            qDebug() << "Control:" << control.first->id() << control.first->type() << QString::fromStdString(control.first->name()) <<  QString::fromStdString(control.second.toString());
            for (auto val : control.second.values()) {
                qDebug() << "Value: " << QString::fromStdString(val.toString());
            }
        }

        cameraChanged();
        startViewFinder();
    }
}

void CameraProxy::startViewFinder()
{
    qDebug() << Q_FUNC_INFO;
    int ret;

    if (m_state != Stopped) {
        qWarning() << "Camera not stopped, not starting viewfinder";
        return;
    }

    libcamera::StreamConfiguration &vfConfig = m_viewFinderConfig->at(0);

    /* Use a format supported by the viewfinder if available. Default to JPEG*/
    vfConfig.pixelFormat = libcamera::PixelFormat::fromString("MJPEG");
    for (const libcamera::PixelFormat &format : m_viewFinder->nativeFormats()) {
        auto match = m_viewFinderFormats.find(format);

        if (match != m_viewFinderFormats.end()) {
            vfConfig.pixelFormat = format;
            break;
        }
    }

    //Configure the viewfinder at a lower resulution for speed
    vfConfig.size = libcamera::Size(1280,720);

    libcamera::CameraConfiguration::Status validation = m_viewFinderConfig->validate();
    if (validation == libcamera::CameraConfiguration::Invalid) {
        qWarning() << "Failed to create valid camera configuration";
        return;
    }

    if (validation == libcamera::CameraConfiguration::Adjusted) {
        qInfo() << "Stream configuration adjusted to "
                << vfConfig.toString().c_str();
    }

    ret = m_currentCamera->configure(m_viewFinderConfig.get());
    if (ret < 0) {
        qInfo() << "Failed to configure camera";
        return;
    }

    /* Store stream allocation. */
    m_viewFinderStream = m_viewFinderConfig->at(0).stream();

    /*
         * Configure the viewfinder. If no color space is reported, default to
         * sYCC.
         */
    ret = m_viewFinder->setFormat(vfConfig.pixelFormat,
                                  QSize(vfConfig.size.width, vfConfig.size.height),
                                  vfConfig.colorSpace.value_or(libcamera::ColorSpace::Sycc),
                                  vfConfig.stride);
    if (ret < 0) {
        qInfo() << "Failed to set viewfinder format";
        return;
    }

    /* Allocate and map buffers. */
    m_viewFinderAllocator = new libcamera::FrameBufferAllocator(m_currentCamera);

    ret = m_viewFinderAllocator->allocate(m_viewFinderStream);
    if (ret < 0) {
        qWarning() << "Failed to allocate capture buffers";
        //TODO got error;
        return;
    }

    for (const std::unique_ptr<libcamera::FrameBuffer> &buffer : m_viewFinderAllocator->buffers(m_viewFinderStream)) {
        /* Map memory buffers and cache the mappings. */
        std::unique_ptr<Image> image =
                Image::fromFrameBuffer(buffer.get(), Image::MapMode::ReadOnly);
        assert(image != nullptr);
        m_mappedBuffers[buffer.get()] = std::move(image);

        /* Store buffers on the free list. */
        m_freeBuffers[m_viewFinderStream].enqueue(buffer.get());
    }

    m_requests.clear();

    /* Create requests and fill them with buffers from the viewfinder. */
    while (!m_freeBuffers[m_viewFinderStream].isEmpty()) {
        libcamera::FrameBuffer *buffer = m_freeBuffers[m_viewFinderStream].dequeue();

        std::unique_ptr<libcamera::Request> request = m_currentCamera->createRequest();
        if (!request) {
            qWarning() << "Can't create request";
            ret = -ENOMEM;
            //goto error;
            return;
        }

        ret = request->addBuffer(m_viewFinderStream, buffer);
        if (ret < 0) {
            qWarning() << "Can't set buffer for request";
            //goto error;
            return;
        }

        m_requests.push_back(std::move(request));
    }

    ret = m_currentCamera->start();
    if (ret) {
        qInfo() << "Failed to start capture";
        //goto error;
        return;
    }
    setState(CapturingViewFinder);

    m_currentCamera->requestCompleted.connect(this, &CameraProxy::requestComplete);

    /* Queue all requests. */
    for (std::unique_ptr<libcamera::Request> &request : m_requests) {
        ret = m_currentCamera->queueRequest(request.get());
        if (ret < 0) {
            qWarning() << "Can't queue request";
            //goto error_disconnect;
            return;
        }
    }
}

void CameraProxy::requestComplete(libcamera::Request *request)
{
    //qDebug() << Q_FUNC_INFO;

    if (request->status() == libcamera::Request::RequestCancelled)
        return;

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

void CameraProxy::cacheFormats()
{
    // Clear any old cache
    m_viewFinderFormats.clear();
    m_stillFormats.clear();

    //Configure the camera for view finder
    m_viewFinderConfig = m_currentCamera->generateConfiguration({libcamera::StreamRole::Viewfinder});
    if (!m_viewFinderConfig) {
        qWarning() << "Failed to generate configuration from roles";
        return;
    }

    libcamera::StreamConfiguration &vfConfig = m_viewFinderConfig->at(0);

    for (const libcamera::PixelFormat format : vfConfig.formats().pixelformats()) {
        m_viewFinderFormats[format] = vfConfig.formats().sizes(format);
    }

    //Configure the camera for stills
    m_stillConfig = m_currentCamera->generateConfiguration({libcamera::StreamRole::StillCapture});
    if (!m_stillConfig) {
        qWarning() << "Failed to generate still configuration from roles";
        return;
    }

    libcamera::StreamConfiguration &stConfig = m_stillConfig->at(0);

    for (const libcamera::PixelFormat format : stConfig.formats().pixelformats()) {
        m_stillFormats[format] = stConfig.formats().sizes(format);
    }

    qDebug() << m_viewFinderFormats;
    qDebug() << m_stillFormats;

}

void CameraProxy::stop()
{
    qDebug() << Q_FUNC_INFO;
    if (m_currentCamera) {
        qDebug() << "stopping";
        setState(Stopping);

        m_currentCamera->stop();

        m_currentCamera->requestCompleted.disconnect(this);

        m_mappedBuffers.clear();
        m_requests.clear();
        m_freeQueue.clear();

        if (m_viewFinderAllocator) {
            qDebug() << "deleting vf allocator";
            delete m_viewFinderAllocator;
            m_viewFinderAllocator = nullptr;
        }
        if (m_stillAllocator) {
            qDebug() << "deleting still allocator";
            delete m_stillAllocator;
            m_stillAllocator = nullptr;
        }
        m_freeBuffers.clear();
        m_doneQueue.clear();
        setState(Stopped);
    }
}

void CameraProxy::stillCapture(const QString &filename)
{
    qDebug() << Q_FUNC_INFO;
    stop();

    m_saveFileName = filename;

    int ret;
    m_frame = 0;
    //Do stuff
    libcamera::StreamConfiguration &stillConfig = m_stillConfig->at(0);

    /* Use a format supported by the viewfinder if available. Default to JPEG*/
    stillConfig.pixelFormat = libcamera::PixelFormat::fromString(m_currentStillFormat.toStdString());

    //Configure the viewfinder at a lower resulution for speed
    stillConfig.size = libcamera::Size(m_currentStillResolution.width(),m_currentStillResolution.height());

    libcamera::CameraConfiguration::Status validation = m_stillConfig->validate();
    if (validation == libcamera::CameraConfiguration::Invalid) {
        qWarning() << "Failed to create valid camera configuration";
        return;
    }

    if (validation == libcamera::CameraConfiguration::Adjusted) {
        qInfo() << "Stream configuration adjusted to "
                << stillConfig.toString().c_str();
    }

    ret = m_currentCamera->configure(m_stillConfig.get());
    if (ret < 0) {
        qInfo() << "Failed to configure camera";
        return;
    }

    /* Store stream allocation. */
    m_stillStream = m_stillConfig->at(0).stream();

    /* Allocate and map buffers. */
    m_stillAllocator = new libcamera::FrameBufferAllocator(m_currentCamera);

    ret = m_stillAllocator->allocate(m_stillStream);
    if (ret < 0) {
        qWarning() << "Failed to allocate still capture buffers";
        //TODO got error;
        return;
    }

    qDebug() << "Creating still buffers";
    for (const std::unique_ptr<libcamera::FrameBuffer> &buffer : m_stillAllocator->buffers(m_stillStream)) {
        qDebug() << "Creating still buffers : 1";
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
        libcamera::FrameBuffer *buffer = m_freeBuffers[m_stillStream].dequeue();

        std::unique_ptr<libcamera::Request> request = m_currentCamera->createRequest();
        if (!request) {
            qWarning() << "Can't create request";
            ret = -ENOMEM;
            //goto error;
            return;
        }

        ret = request->addBuffer(m_stillStream, buffer);
        if (ret < 0) {
            qWarning() << "Can't set buffer for request";
            //goto error;
            return;
        }

        m_requests.push_back(std::move(request));
    }

    ret = m_currentCamera->start();
    if (ret) {
        qInfo() << "Failed to start capture";
        //goto error;
        return;
    }
    setState(CapturingStill);

    m_currentCamera->requestCompleted.connect(this, &CameraProxy::requestComplete);

    /* Queue all requests. */
    for (std::unique_ptr<libcamera::Request> &request : m_requests) {
        ret = m_currentCamera->queueRequest(request.get());
        if (ret < 0) {
            qWarning() << "Can't queue request";
            //goto error_disconnect;
            return;
        }
    }
}

bool CameraProxy::controlExists(Control c)
{
    qDebug() << Q_FUNC_INFO << c;
    if (m_currentCamera && m_currentCamera->controls().size() > 0) {
        qDebug() << (m_currentCamera->controls().find(c) != m_currentCamera->controls().end());
        return m_currentCamera->controls().find(c) != m_currentCamera->controls().end();
    }
    return false;
}

float CameraProxy::controlMin(Control c)
{
    if (!controlExists(c)) {
        return 0;
    }
    auto control = m_currentCamera->controls().find(c);

    if (control != m_currentCamera->controls().end()) {
        return control->second.min().get<float>();
    }
    return 0;
}

float CameraProxy::controlMax(Control c)
{
    if (!controlExists(c)) {
        return 0;
    }
    auto control = m_currentCamera->controls().find(c);

    if (control != m_currentCamera->controls().end()) {
        return control->second.max().get<float>();
    }
    return 0;
}

float CameraProxy::controlValue(Control c)
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

void CameraProxy::setControlValue(Control c, float val)
{
    qDebug() << Q_FUNC_INFO << c << val << controlMin(c) << controlMax(c);
    if (controlExists(c) && val <= controlMax(c) && val >= controlMin(c)) {
        m_controlValues[c] = val;
    } else {
        qWarning() << "Value" << val << "is out of range for" << c;
    }
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

    /* Process buffers. */
    if (m_state == CapturingViewFinder && request->buffers().count(m_viewFinderStream)) {
        processViewfinder(request->buffers().at(m_viewFinderStream));
    }

    if (m_state == CapturingStill && request->buffers().count(m_stillStream)) {
        processStill(request->buffers().at(m_stillStream));
    }

    if (m_state <= Stopping) {
        return;
    }

    request->reuse();
    QMutexLocker locker(&m_mutex);
    m_freeQueue.enqueue(request);
}

void CameraProxy::processViewfinder(libcamera::FrameBuffer *buffer)
{
    //qDebug() << Q_FUNC_INFO;

    m_viewFinder->renderImage(buffer, m_mappedBuffers[buffer].get());
}

void CameraProxy::processStill(libcamera::FrameBuffer *buffer)
{
    qDebug() << Q_FUNC_INFO << m_saveFileName << m_frame;

    if (m_frame < 4) {
        qDebug() << "Skipping frame " << m_frame;
        m_frame++;
        if (buffer)
            renderComplete(buffer);
        return;
    }

    QFile file(m_saveFileName);
    if (!file.open(QIODevice::WriteOnly)) {
           return;
    }

    size_t size = buffer->metadata().planes()[0].bytesused;
    file.write((const char*)m_mappedBuffers[buffer].get()->data(0).data(), size);
    file.close();

    if (buffer) {
        renderComplete(buffer);
    }

    Q_EMIT stillCaptureFinished(m_saveFileName);
}

void CameraProxy::renderComplete(libcamera::FrameBuffer *buffer)
{
    libcamera::Request *request;
    {
        QMutexLocker locker(&m_mutex);
        if (m_freeQueue.isEmpty())
            return;

        request = m_freeQueue.dequeue();
    }

    if (m_state == CapturingViewFinder) {
        request->addBuffer(m_viewFinderStream, buffer);
        for(auto c : m_controlValues) {
            request->controls().set(c.first, c.second);
        }
        m_currentCamera->queueRequest(request);
    }
    if (m_state == CapturingStill && m_frame < 5) {
        request->addBuffer(m_stillStream, buffer);
        m_currentCamera->queueRequest(request);
    }
}

CameraProxy::CameraState CameraProxy::state() const
{
    return m_state;
}

void CameraProxy::setState(CameraState newState)
{
    if (m_state == newState)
        return;
    m_state = newState;
    Q_EMIT stateChanged();
}
