#include "viewfinder2d.h"

#include <assert.h>

#include <libcamera/formats.h>

#include <QImage>
#include <QImageWriter>
#include <QMap>
#include <QMutexLocker>
#include <QPainter>
#include <QtDebug>
#include <QVideoFrame>
#include <QVideoFrameFormat>

#include "image.h"
#include <string.h>

static const QMap<libcamera::PixelFormat, QImage::Format> nativeFormats
{
    { libcamera::formats::ABGR8888, QImage::Format_RGBX8888 },
    { libcamera::formats::XBGR8888, QImage::Format_RGBX8888 },
    { libcamera::formats::ARGB8888, QImage::Format_RGB32 },
    { libcamera::formats::XRGB8888, QImage::Format_RGB32 },
    { libcamera::formats::RGB888, QImage::Format_BGR888 },
    { libcamera::formats::BGR888, QImage::Format_RGB888 },
    { libcamera::formats::RGB565, QImage::Format_RGB16 },
};

ViewFinder2D::ViewFinder2D()
    : m_buffer(nullptr)
{
}

const QList<libcamera::PixelFormat> &ViewFinder2D::nativeFormats() const
{
    static const QList<libcamera::PixelFormat> formats = ::nativeFormats.keys();
    return formats;
}

int ViewFinder2D::setFormat(const libcamera::PixelFormat &format, const QSize &size,
                            [[maybe_unused]] const libcamera::ColorSpace &colorSpace, unsigned int stride)
{
    qDebug() << "Setting vf pixel format to " << format << size;

    m_image = QImage();
    m_format = format;
    m_size = size;

    /*
     * If format conversion is needed, configure the converter and allocate
     * the destination image.
     */
    if (!::nativeFormats.contains(format)) {
        int ret = m_converter.configure(format, size, stride);
        if (ret < 0)
            return ret;

        m_image = QImage(size, QImage::Format_RGB32);

        qInfo() << "Using software format conversion from"
            << format.toString().c_str();
    } else {
        qInfo() << "Zero-copy enabled";
    }

    return 0;
}

void ViewFinder2D::renderImage(libcamera::FrameBuffer *buffer, class Image *image, QList<QRectF> rects)
{
    size_t size1 = buffer->metadata().planes()[0].bytesused;
    size_t totalSize = 0;

    m_rects = rects;
    for (uint plane = 0; plane < buffer->metadata().planes().size(); ++plane) {
        totalSize += buffer->metadata().planes()[plane].bytesused;
    }

    //qDebug() << "Frame size " << totalSize << size1 << "Planes " <<  buffer->metadata().planes().size() << m_format;

    {
        QMutexLocker locker(&m_mutex);

        if (::nativeFormats.contains(m_format)) {
            /*
             * If the frame format is identical to the display
             * format, create a QImage that references the frame
             * and store a reference to the frame buffer. The
             * previously stored frame buffer, if any, will be
             * released.
             *
             * \todo Get the stride from the buffer instead of
             * computing it naively
             */
            assert(buffer->planes().size() == 1);
            m_image = QImage(image->data(0).data(), m_size.width(),
                    m_size.height(), size1 / m_size.height(),
                    ::nativeFormats[m_format]);
            std::swap(buffer, m_buffer);
        } else {
            /*
             * Otherwise, convert the format and release the frame
             * buffer immediately.
             */
            m_converter.convert(image, size1, &m_image);
        }
    }
    update();

    if (buffer) {
        Q_EMIT renderComplete(buffer);
    }
}

void ViewFinder2D::stop()
{
    m_image = QImage();

    if (m_buffer) {
        Q_EMIT renderComplete(m_buffer);
        m_buffer = nullptr;
    }

    update();
}

QImage ViewFinder2D::currentImage()
{
    return m_image;
}

void ViewFinder2D::paint(QPainter *painter)
{
    /* If we have an image, draw it. */
    int w = height() * ((float)m_image.rect().width() / (float)m_image.rect().height());
    int offset = (width() - w) / 2;

    //qDebug() << Q_FUNC_INFO << m_image.rect();

    if (!m_image.isNull()) {
        painter->drawImage(QRectF(QPointF(offset,0), QSizeF(w, height())), m_image, m_image.rect());

        QPen p(Qt::white);
        p.setWidth(4);
        painter->setPen(p);
        for (QRectF r: m_rects) {
            QRectF scaled(r.x() * width(), r.y() * height(), r.width() * width(), r.height() * height());
            painter->drawRect(scaled);
        }
        return;
    }

    /*
     * Otherwise, draw the camera stopped icon. Render it to the pixmap if
     * the size has changed.
     */
    constexpr int margin = 20;
    QSizeF sz(width(), height());

    if (m_vfSize != sz || m_pixmap.isNull()) {
        QSizeF vfSize = sz - QSize{ 2 * margin, 2 * margin };
        QSizeF pixmapSize{ 1, 1 };
        pixmapSize.scale(vfSize, Qt::KeepAspectRatio);

        m_vfSize = sz;
    }

    QPoint point{ margin, margin };
    if (m_pixmap.width() < width() - 2 * margin)
        point.setX((width() - m_pixmap.width()) / 2);
    else
        point.setY((height() - m_pixmap.height()) / 2);

    painter->setBackgroundMode(Qt::OpaqueMode);
    painter->drawPixmap(point, m_pixmap);
}
