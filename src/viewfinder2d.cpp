#include "viewfinder2d.h"

#include <assert.h>
#include <stdint.h>
#include <utility>

#include <libcamera/formats.h>

#include <QImage>
#include <QImageWriter>
#include <QMap>
#include <QMutexLocker>
#include <QPainter>
#include <QtDebug>
#include <QVideoFrame>
#include "facedetection.h"

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
#include "private/qvideoframe_p.h"
#endif

#include "image.h"
#include <string.h>

static const QMap<libcamera::PixelFormat, QVideoFrame::PixelFormat> nativeFormats
{
    { libcamera::formats::MJPEG, QVideoFrame::Format_Jpeg },
    { libcamera::formats::ABGR8888, QVideoFrame::Format_BGR32 },
    { libcamera::formats::ARGB8888, QVideoFrame::Format_ARGB32 },
    { libcamera::formats::XRGB8888, QVideoFrame::Format_RGB32 },
    { libcamera::formats::RGB888, QVideoFrame::Format_RGB24 },
    { libcamera::formats::BGR888, QVideoFrame::Format_BGR24 },
    { libcamera::formats::YUYV, QVideoFrame::Format_YUYV },
    { libcamera::formats::YUV444, QVideoFrame::Format_YUV444 },
    { libcamera::formats::UYVY, QVideoFrame::Format_UYVY },
    { libcamera::formats::NV12, QVideoFrame::Format_NV12 },
    { libcamera::formats::NV21, QVideoFrame::Format_NV21 },
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
    qDebug() << "Setting vf pixel format to " << m_qvFormat << size;

    m_image = QImage();
    m_format = format;
    m_size = size;

    auto match = ::nativeFormats.find(m_format);
    if (match != ::nativeFormats.end()) {
        m_qvFormat = match.value();
        qDebug() << "Setting vf pixel format to " << m_qvFormat;
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

    //qDebug() << "Frame size " << totalSize << "Planes " <<  buffer->metadata().planes().size();

    {
        QMutexLocker locker(&m_mutex);
        QSize sz(m_size.width(), m_size.height());

        //Configure the frame if required
        if ((m_frame.width() != sz.width() || m_frame.height() != sz.height()) || m_qvFormat == QVideoFrame::Format_Jpeg) {
            m_frame = QVideoFrame(totalSize, sz, size1 / m_size.height(), m_qvFormat);
        }

        //Copy data into the frame
        if (m_frame.map(QAbstractVideoBuffer::WriteOnly)) {
            size_t curOffset = 0;
            for (uint plane = 0; plane < buffer->metadata().planes().size(); ++plane) {
                memcpy(m_frame.bits() + curOffset, image->data(plane).data(), buffer->metadata().planes()[plane].bytesused);
                curOffset += buffer->metadata().planes()[plane].bytesused;
            }
            m_frame.unmap();
        } else {
            qDebug() << "Unable to map video frame writeonly";
        }

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
        m_image= qt_imageFromVideoFrame(m_frame);
#else
        m_image = m_frame.image();
#endif
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
