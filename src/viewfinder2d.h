#ifndef VIEWFINDER2D_H
#define VIEWFINDER2D_H

#include <QQuickPaintedItem>
#include <QImage>
#include <QList>
#include <QMutex>
#include <QSize>
#include <QPixmap>

#include <libcamera/formats.h>
#include <libcamera/framebuffer.h>
#include <libcamera/pixel_format.h>

#include "viewfinder.h"
#include "format_converter.h"

class ViewFinder2D : public QQuickPaintedItem, public ViewFinder
{
    Q_OBJECT
public:
    ViewFinder2D();

    const QList<libcamera::PixelFormat> &nativeFormats() const override;

    int setFormat(const libcamera::PixelFormat &format, const QSize &size,
                  const libcamera::ColorSpace &colorSpace,
                  unsigned int stride) override;
    void render(libcamera::FrameBuffer *buffer, class Image *image, QList<QRectF>) override;
    void stop() override;

    QImage getCurrentImage() override;

Q_SIGNALS:
    void renderComplete(libcamera::FrameBuffer *buffer);

protected:
    virtual void paint(QPainter *) override;

private:
    FormatConverter m_converter;
    libcamera::PixelFormat m_format;
    QSize m_size;

    /* Camera stopped icon */
    QSizeF m_vfSize;
    QPixmap m_pixmap;

    /* Buffer and render image */
    libcamera::FrameBuffer *m_buffer;
    QImage m_image;
    QMutex m_mutex; /* Prevent concurrent access to image_ */

    QList<QRectF> m_rects;
};

#endif // VIEWFINDER2D_H
