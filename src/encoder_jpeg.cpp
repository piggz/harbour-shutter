#include "encoder_jpeg.h"

#include <fcntl.h>
#include <iomanip>
#include <iostream>
#include <string.h>
#include <QImage>

#include <libcamera/camera.h>
#include <libcamera/libcamera/formats.h>

#include "src/image.h"
#include "src/format_converter.h"
#include "qdebug.h"

static const QMap<libcamera::PixelFormat, QImage::Format> nativeFormats
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 2, 0)
    { libcamera::formats::ABGR8888, QImage::Format_RGBX8888 },
    { libcamera::formats::XBGR8888, QImage::Format_RGBX8888 },
#endif
    { libcamera::formats::ARGB8888, QImage::Format_RGB32 },
    { libcamera::formats::XRGB8888, QImage::Format_RGB32 },
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    { libcamera::formats::RGB888, QImage::Format_BGR888 },
#endif
    { libcamera::formats::BGR888, QImage::Format_RGB888 },
};

EncoderJpeg::EncoderJpeg()
{

}

bool EncoderJpeg::encode(const libcamera::StreamConfiguration &cfg, libcamera::FrameBuffer *buffer, Image *image, std::string outFileName)
{
    qDebug() << Q_FUNC_INFO;

    size_t size = 0;
    for (uint plane = 0; plane < buffer->metadata().planes().size(); ++plane) {
        size += buffer->metadata().planes()[plane].bytesused;
    }

    QSize qs = QSize(cfg.size.width, cfg.size.height);
    QImage image_;

    libcamera::PixelFormat pixelFormat_  = cfg.pixelFormat;

    /* If format conversion is needed, configure the converter and allocate
    * the destination image.
    */
    if (!::nativeFormats.contains(pixelFormat_)) {
        int ret = converter_.configure(pixelFormat_, qs, cfg.stride);
        if (ret < 0) {
            qDebug() << "Unable to configure converter" << ret << qs << cfg.stride;
            return false;
        }

        image_ = QImage(qs, QImage::Format_RGB32);

        qInfo() << "Using software format conversion from" << pixelFormat_.toString().c_str();
        converter_.convert(image, size, &image_);
    } else {
        qInfo() << "Zero-copy enabled";
        image_ = QImage(image->data(0).data(), qs.width(),
                        qs.height(), size / qs.height(),
                        ::nativeFormats[pixelFormat_]);
        std::swap(buffer, buffer_);
    }

    image_.save(QString::fromStdString(outFileName), "JPG", 92);
    return true;
}
