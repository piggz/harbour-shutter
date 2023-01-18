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
    if (m_cameraManager) {
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

        cameraChanged();
        startViewFinder();
    }
}

void CameraProxy::startViewFinder()
{
    qDebug() << Q_FUNC_INFO;
    int ret;

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
    for (libcamera::StreamConfiguration &config : *m_viewFinderConfig) {
        libcamera::Stream *stream = config.stream();

        ret = m_viewFinderAllocator->allocate(stream);
        if (ret < 0) {
            qWarning() << "Failed to allocate capture buffers";
            //TODO got error;
            return;
        }

        for (const std::unique_ptr<libcamera::FrameBuffer> &buffer : m_viewFinderAllocator->buffers(stream)) {
            /* Map memory buffers and cache the mappings. */
            std::unique_ptr<Image> image =
                    Image::fromFrameBuffer(buffer.get(), Image::MapMode::ReadOnly);
            assert(image != nullptr);
            mappedBuffers_[buffer.get()] = std::move(image);

            /* Store buffers on the free list. */
            freeBuffers_[stream].enqueue(buffer.get());
        }
    }

    /* Create requests and fill them with buffers from the viewfinder. */
    while (!freeBuffers_[m_viewFinderStream].isEmpty()) {
        libcamera::FrameBuffer *buffer = freeBuffers_[m_viewFinderStream].dequeue();

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

        requests_.push_back(std::move(request));
    }

    ret = m_currentCamera->start();
    if (ret) {
        qInfo() << "Failed to start capture";
        //goto error;
        return;
    }
    m_state = CapturingViewFinder;

    m_currentCamera->requestCompleted.connect(this, &CameraProxy::requestComplete);

    /* Queue all requests. */
    for (std::unique_ptr<libcamera::Request> &request : requests_) {
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
        doneQueue_.enqueue(request);
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
        m_currentCamera->stop();
        m_state = Stopped;
    }
}

void CameraProxy::stillCapture(const QString &filename)
{
    qDebug() << Q_FUNC_INFO;
    stop();

    m_saveFileName = filename;

    int ret;

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
        mappedBuffers_[buffer.get()] = std::move(image);

        /* Store buffers on the free list. */
        freeBuffers_[m_stillStream].enqueue(buffer.get());
    }

    requests_.clear();

    /* Create requests and fill them with buffers from the viewfinder. */
    while (!freeBuffers_[m_stillStream].isEmpty()) {
        libcamera::FrameBuffer *buffer = freeBuffers_[m_stillStream].dequeue();

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

        requests_.push_back(std::move(request));
    }

    ret = m_currentCamera->start();
    if (ret) {
        qInfo() << "Failed to start capture";
        //goto error;
        return;
    }
    m_state = CapturingStill;

    //m_currentCamera->requestCompleted.connect(this, &CameraProxy::requestComplete);

    /* Queue all requests. */
    for (std::unique_ptr<libcamera::Request> &request : requests_) {
        ret = m_currentCamera->queueRequest(request.get());
        if (ret < 0) {
            qWarning() << "Can't queue request";
            //goto error_disconnect;
            return;
        }
    }
}

void CameraProxy::processCapture()
{
    qDebug() << Q_FUNC_INFO;
    /*
         * Retrieve the next buffer from the done queue. The queue may be empty
         * if stopCapture() has been called while a CaptureEvent was posted but
         * not processed yet. Return immediately in that case.
         */
    libcamera::Request *request;
    {
        QMutexLocker locker(&m_mutex);
        if (doneQueue_.isEmpty())
            return;

        request = doneQueue_.dequeue();
    }

    /* Process buffers. */
    if (request->buffers().count(m_viewFinderStream)) {
        processViewfinder(request->buffers().at(m_viewFinderStream));
    }

    if (request->buffers().count(m_stillStream)) {
        processStill(request->buffers().at(m_stillStream));
    }

    request->reuse();
    QMutexLocker locker(&m_mutex);
    freeQueue_.enqueue(request);
}

void CameraProxy::processViewfinder(libcamera::FrameBuffer *buffer)
{
    //qDebug() << Q_FUNC_INFO;

    m_viewFinder->renderImage(buffer, mappedBuffers_[buffer].get());
}

void CameraProxy::processStill(libcamera::FrameBuffer *buffer)
{
    qDebug() << Q_FUNC_INFO << m_saveFileName;

    QFile file(m_saveFileName);
    if (!file.open(QIODevice::WriteOnly)) {
           return;
    }

    file.write((const char*)mappedBuffers_[buffer].get()->data(0).data());

    file.close();

    stop();
    startViewFinder();
}


void CameraProxy::renderComplete(libcamera::FrameBuffer *buffer)
{
    libcamera::Request *request;
    {
        QMutexLocker locker(&m_mutex);
        if (freeQueue_.isEmpty())
            return;

        request = freeQueue_.dequeue();
    }

    if (m_state == CapturingViewFinder) {
        request->addBuffer(m_viewFinderStream, buffer);
        m_currentCamera->queueRequest(request);
    }
}

