#ifndef CAMERAPROXY_H
#define CAMERAPROXY_H

#include <QObject>
#include <QQueue>
#include <QEvent>
#include <QMutex>

#include <libcamera/camera.h>
#include <libcamera/camera_manager.h>
#include <libcamera/controls.h>
#include <libcamera/framebuffer.h>
#include <libcamera/framebuffer_allocator.h>
#include <libcamera/request.h>
#include <libcamera/stream.h>
#include <libcamera/pixel_format.h>

#include "viewfinder.h"
#include "viewfinder2d.h"

#include "image.h"

class CameraProxy : public QObject
{
    Q_OBJECT
public:
    explicit CameraProxy(QObject *parent = nullptr);

    bool event(QEvent *e) override;

    void setCameraManager(std::shared_ptr<libcamera::CameraManager> cm);

    Q_INVOKABLE QStringList supportedFormats();
    Q_INVOKABLE void setStillFormat(const QString &format);
    Q_INVOKABLE QString currentStillFormat();
    Q_INVOKABLE void setResolution(const QSize &res);

    std::vector<libcamera::Size> supportedReoluions(QString format);

public Q_SLOTS:
    void renderComplete(libcamera::FrameBuffer *buffer);
    void setViewFinder(ViewFinder2D *vf);
    void setCameraIndex(QString idx);
    void startViewFinder();
    void stop();
    void stillCapture(const QString &filename);


Q_SIGNALS:
    void cameraChanged();
    void formatChanged();
    void resolutionChanged();

private:
    std::shared_ptr<libcamera::CameraManager> m_cameraManager;
    std::shared_ptr<libcamera::Camera> m_currentCamera;

    ViewFinder2D* m_viewFinder;
    QString m_currentCameraId;
    QMutex m_mutex;

    std::map<libcamera::FrameBuffer *, std::unique_ptr<Image>> mappedBuffers_;
    libcamera::FrameBufferAllocator *m_viewFinderAllocator;
    libcamera::FrameBufferAllocator *m_stillAllocator;

    // Capture state, buffers queue and statistics
    bool m_isCapturing = false;
    libcamera::Stream *m_viewFinderStream;
    libcamera::Stream *m_stillStream;
    std::map<const libcamera::Stream *, QQueue<libcamera::FrameBuffer *>> freeBuffers_;
    QQueue<libcamera::Request *> doneQueue_;
    QQueue<libcamera::Request *> freeQueue_;
    std::vector<std::unique_ptr<libcamera::Request>> requests_;


    // Cached still and viewfinder modes
    std::map<libcamera::PixelFormat, std::vector<libcamera::Size>> m_viewFinderFormats;
    std::map<libcamera::PixelFormat, std::vector<libcamera::Size>> m_stillFormats;

    std::unique_ptr<libcamera::CameraConfiguration> m_viewFinderConfig;
    std::unique_ptr<libcamera::CameraConfiguration> m_stillConfig;

    QString m_currentStillFormat;
    QSize m_currentStillResolution;
    QString m_saveFileName;

    void processCapture();
    void processViewfinder(libcamera::FrameBuffer *buffer);
    void processStill(libcamera::FrameBuffer *buffer);

    void requestComplete(libcamera::Request *request);
    void cacheFormats();
};

class CaptureEvent : public QEvent
{
public:
    CaptureEvent()
        : QEvent(type())
    {
    }

    static Type type()
    {
        static int type = QEvent::registerEventType();
        return static_cast<Type>(type);
    }
};

#endif // CAMERAPROXY_H
