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
#include <libcamera/control_ids.h>

#include "viewfinder.h"
#include "viewfinder2d.h"
#include "facedetection.h"

#include "image.h"

class CameraProxy : public QObject
{
    Q_OBJECT
public:
    explicit CameraProxy(QObject *parent = nullptr);
    ~CameraProxy();

    Q_PROPERTY(CameraState state READ state WRITE setState NOTIFY stateChanged)
    enum CameraState {
        Stopped = 0,
        Stopping,
        CapturingStill,
        CapturingViewFinder,
        ConfiguringStill,
        ConfiguringViewFinder
    };
    enum Control {
        AeEnable = libcamera::controls::AE_ENABLE,
        Brightness = libcamera::controls::BRIGHTNESS,
        Saturation = libcamera::controls::SATURATION,
        AnalogueGain = libcamera::controls::ANALOGUE_GAIN,
        Contrast = libcamera::controls::CONTRAST,
        ExposureTime = libcamera::controls::EXPOSURE_TIME
    };

    enum ControlType {
        ControlTypeNone = libcamera::ControlTypeNone,
        ControlTypeBool = libcamera::ControlTypeBool,
        ControlTypeByte = libcamera::ControlTypeByte,
        ControlTypeInteger32 = libcamera::ControlTypeInteger32,
        ControlTypeInteger64 = libcamera::ControlTypeInteger64,
        ControlTypeFloat = libcamera::ControlTypeFloat,
        ControlTypeString = libcamera::ControlTypeString,
        ControlTypeRectangle = libcamera::ControlTypeRectangle,
        ControlTypeSize = libcamera::ControlTypeSize
    };
    Q_ENUM(CameraState)
    Q_ENUM(Control);
    Q_ENUM(ControlType)

    bool event(QEvent *e) override;

    void setCameraManager(std::shared_ptr<libcamera::CameraManager> cm);

    Q_INVOKABLE QStringList supportedFormats() const;
    Q_INVOKABLE void setStillFormat(const QString &format);
    Q_INVOKABLE QString currentStillFormat() const;
    Q_INVOKABLE void setResolution(const QSize &res);

    std::vector<libcamera::Size> supportedResoluions(QString format);
    libcamera::ControlInfoMap supportedControls() const;

    CameraState state() const;
    void setState(CameraState newState);

    //Controls
    bool controlExists(CameraProxy::Control c);
    float controlMin(CameraProxy::Control c);
    float controlMax(CameraProxy::Control c);
    float controlValue(CameraProxy::Control c);
    Q_INVOKABLE void setControlValue(CameraProxy::Control c, ControlType type, QVariant val);
    Q_INVOKABLE void removeControlValue(CameraProxy::Control c);

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
    void stillSaveComplete(libcamera::FrameBuffer *buffer);
    void stillCaptureFinished(const QString &path);
    void stateChanged();

private:
    std::shared_ptr<libcamera::CameraManager> m_cameraManager;
    std::shared_ptr<libcamera::Camera> m_currentCamera;

    ViewFinder2D* m_viewFinder;
    QString m_currentCameraId;
    QMutex m_mutex;

    std::map<libcamera::FrameBuffer *, std::unique_ptr<Image>> m_mappedBuffers;
    libcamera::FrameBufferAllocator *m_allocator = nullptr;

    // Capture state, buffers queue and statistics
    CameraState m_state = Stopped;

    libcamera::Stream *m_viewFinderStream;
    libcamera::Stream *m_stillStream;
    std::map<const libcamera::Stream *, QQueue<libcamera::FrameBuffer *>> m_freeBuffers;
    QQueue<libcamera::Request *> m_doneQueue;
    QQueue<libcamera::Request *> m_freeQueue;
    std::vector<std::unique_ptr<libcamera::Request>> m_requests;


    // Cached still and viewfinder modes
    std::map<libcamera::PixelFormat, std::vector<libcamera::Size>> m_viewFinderFormats;
    std::map<libcamera::PixelFormat, std::vector<libcamera::Size>> m_stillFormats;

    std::unique_ptr<libcamera::CameraConfiguration> m_config;

    libcamera::StreamConfiguration *m_vfStreamConfig = nullptr;
    libcamera::StreamConfiguration *m_stillStreamConfig = nullptr;

    QString m_currentStillFormat;
    libcamera::Size m_currentStillResolution;
    QString m_saveFileName;
    int m_frame = 0;
    bool m_captureStill = false;
    bool m_singleStream = false;

    bool buildConfiguration( std::initializer_list<libcamera::StreamRole> roles);
    bool configureCamera();

    void processCapture();
    void processViewfinder(libcamera::FrameBuffer *buffer);
    void processStill(libcamera::FrameBuffer *buffer);

    void requestComplete(libcamera::Request *request);
    void cacheFormats(libcamera::StreamRole role);

    libcamera::Size bestViewfinderResolution(libcamera::PixelFormat format, libcamera::Size stillSize);

    std::unordered_map<Control, libcamera::ControlValue> m_controlValues;

    //Face detection
    FaceDetection m_fd;
    QList<QRectF> m_rects;
    uint m_rectDelay = 0;
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
