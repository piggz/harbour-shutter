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

#include "format_converter.h"
#include "viewfinder.h"


class ViewFinder2D : public QQuickPaintedItem, public ViewFinder
{
    Q_OBJECT
public:
    ViewFinder2D();

    const QList<libcamera::PixelFormat> &nativeFormats() const override;

    int setFormat(const libcamera::PixelFormat &format, const QSize &size,
                  const libcamera::ColorSpace &colorSpace,
                  unsigned int stride) override;
    void renderImage(libcamera::FrameBuffer *buffer, class Image *image) override;
    void stop() override;

    //QImage getCurrentImage() override;

Q_SIGNALS:
    void renderComplete(libcamera::FrameBuffer *buffer);

protected:
    virtual void paint(QPainter *) override;

private:
    FormatConverter converter_;

    libcamera::PixelFormat format_;
    QSize size_;

    /* Camera stopped icon */
    QSizeF vfSize_;
    QPixmap pixmap_;

    /* Buffer and render image */
    libcamera::FrameBuffer *buffer_;
    QImage image_;
    QMutex mutex_; /* Prevent concurrent access to image_ */

};

#endif // VIEWFINDER2D_H
