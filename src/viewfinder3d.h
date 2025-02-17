#ifndef VIEWFINDER3D_H
#define VIEWFINDER3D_H

#include <QQuickItem>

#include <QImage>
#include <QMutex>
#include <QSize>

#include <libcamera/formats.h>
#include <libcamera/framebuffer.h>

#include "viewfinder.h"
#include "openglviewfinderrendernode.h"

class ViewFinder3D : public QQuickItem, public ViewFinder
{
    Q_OBJECT
    QML_ELEMENT
public:
    ViewFinder3D(QQuickItem *parent = nullptr);
    QSGNode *updatePaintNode(QSGNode *node, UpdatePaintNodeData *) override;

    const QList<libcamera::PixelFormat> &nativeFormats() const override;

    int setFormat(const libcamera::PixelFormat &format, const QSize &size,
                  const libcamera::ColorSpace &colorSpace,
                  unsigned int stride) override;
    void render(libcamera::FrameBuffer *buffer, Image *image, QList<QRectF>) override;
    void stop() override;

    QImage getCurrentImage() override;


Q_SIGNALS:
    void renderComplete(libcamera::FrameBuffer *buffer);

private:
    OpenGLViewFinderRenderNode *m_node = nullptr;

    /* Captured image size, format and buffer */
    libcamera::FrameBuffer *m_buffer;
    libcamera::PixelFormat m_format;
    libcamera::ColorSpace m_colorSpace;
    QSize m_size;
    unsigned int m_stride;
    Image *image_;
    QMutex mutex_; /* Prevent concurrent access to image_ */
};

#endif // VIEWFINDER3D_H
