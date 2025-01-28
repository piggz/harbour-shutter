#include "viewfinder3d.h"
#include <QSGRenderNode>
#include <QSGRendererInterface>
#include <QQuickWindow>
#include <qevent.h>
#include <QFile>

#include "image.h"
#include "openglviewfinderrendernode.h"

static const QList<libcamera::PixelFormat> supportedFormats{
    /* YUV - packed (single plane) */
    libcamera::formats::UYVY,
    libcamera::formats::VYUY,
    libcamera::formats::YUYV,
    libcamera::formats::YVYU,
    /* YUV - semi planar (two planes) */
    libcamera::formats::NV12,
    libcamera::formats::NV21,
    libcamera::formats::NV16,
    libcamera::formats::NV61,
    libcamera::formats::NV24,
    libcamera::formats::NV42,
    /* YUV - fully planar (three planes) */
    libcamera::formats::YUV420,
    libcamera::formats::YVU420,
    /* RGB */
    libcamera::formats::ABGR8888,
    libcamera::formats::ARGB8888,
    libcamera::formats::BGRA8888,
    libcamera::formats::RGBA8888,
    libcamera::formats::BGR888,
    libcamera::formats::RGB888,
    /* Raw Bayer 8-bit */
    libcamera::formats::SBGGR8,
    libcamera::formats::SGBRG8,
    libcamera::formats::SGRBG8,
    libcamera::formats::SRGGB8,
    /* Raw Bayer 10-bit packed */
    libcamera::formats::SBGGR10_CSI2P,
    libcamera::formats::SGBRG10_CSI2P,
    libcamera::formats::SGRBG10_CSI2P,
    libcamera::formats::SRGGB10_CSI2P,
    /* Raw Bayer 12-bit packed */
    libcamera::formats::SBGGR12_CSI2P,
    libcamera::formats::SGBRG12_CSI2P,
    libcamera::formats::SGRBG12_CSI2P,
    libcamera::formats::SRGGB12_CSI2P,
};

ViewFinder3D::ViewFinder3D(QQuickItem *parent)
    : QQuickItem(parent), m_buffer(nullptr),
      m_colorSpace(libcamera::ColorSpace::Raw), image_(nullptr)
{
    qDebug() << Q_FUNC_INFO;
    setFlag(ItemHasContents);
}

const QList<libcamera::PixelFormat> &ViewFinder3D::nativeFormats() const
{
    return supportedFormats;
}

int ViewFinder3D::setFormat(const libcamera::PixelFormat &format, const QSize &size, const libcamera::ColorSpace &colorSpace, unsigned int stride)
{
    qDebug() << Q_FUNC_INFO;

    m_format = format;
    m_colorSpace = colorSpace;
    m_size = size;
    m_stride = stride;

    if (m_node) {
        m_node->setFormat(format, size, colorSpace, stride);
    }
    return 0;
}

void ViewFinder3D::stop()
{
    qDebug() << Q_FUNC_INFO;
    if (m_buffer) {
        renderComplete(m_buffer);
        m_buffer = nullptr;
        image_ = nullptr;
    }
}

QImage ViewFinder3D::getCurrentImage()
{
    qDebug() << Q_FUNC_INFO;

    QMutexLocker locker(&mutex_);

    return QImage(); //TODO
}

void ViewFinder3D::render(libcamera::FrameBuffer *buffer, Image *image, QList<QRectF> rects)
{
    qDebug() << Q_FUNC_INFO;

    m_node->preRender(buffer, image, rects);
    if (m_buffer)
        renderComplete(m_buffer);

    image_ = image;
    update();
    m_buffer = buffer;
}



QSGNode *ViewFinder3D::updatePaintNode(QSGNode *node, UpdatePaintNodeData *)
{
    qDebug() << Q_FUNC_INFO;

    //QSGRenderNode *node = static_cast<QSGRenderNode *>(node);

    QSGRendererInterface *ri = window()->rendererInterface();
    if (!ri)
        return nullptr;

    switch (ri->graphicsApi()) {
    case QSGRendererInterface::OpenGL:
#if QT_CONFIG(opengl)
        if (!m_node) {
            m_node = new OpenGLViewFinderRenderNode;
            m_node->setFormat(m_format, m_size, m_colorSpace, m_stride);

        }

        static_cast<OpenGLViewFinderRenderNode *>(m_node)->sync(this);
#endif
        break;

    default:
        break;
    }

    if (!m_node)
        qWarning("QSGRendererInterface reports unknown graphics API %d", ri->graphicsApi());

    return m_node;
}
