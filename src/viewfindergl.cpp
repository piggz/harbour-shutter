#include "viewfindergl.h"

#include <QtQuick/qquickwindow.h>
#include <QOpenGLShaderProgram>
#include <QOpenGLContext>
#include <QtCore/QRunnable>
#include <QFile>

#include "image.h"

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

ViewFinderGL::ViewFinderGL()
    : m_renderer(nullptr), m_buffer(nullptr),
      m_colorSpace(libcamera::ColorSpace::Raw)
{
    qDebug() << Q_FUNC_INFO;
    connect(this, &QQuickItem::windowChanged, this, &ViewFinderGL::handleWindowChanged);
}

void ViewFinderGL::handleWindowChanged(QQuickWindow *win)
{
    qDebug() << Q_FUNC_INFO << win;

    if (win) {
        connect(win, &QQuickWindow::beforeSynchronizing, this, &ViewFinderGL::sync, Qt::DirectConnection);
        connect(win, &QQuickWindow::sceneGraphInvalidated, this, &ViewFinderGL::cleanup, Qt::DirectConnection);

        // Ensure we start with cleared to black. The squircle's blend mode relies on this.
        win->setColor(Qt::black);
    }
}

void ViewFinderGL::cleanup()
{
    delete m_renderer;
    m_renderer = nullptr;
}

class CleanupJob : public QRunnable
{
public:
    CleanupJob(ViewFinderGLRenderer *renderer) : m_renderer(renderer) { }
    void run() override { delete m_renderer; }
private:
    ViewFinderGLRenderer *m_renderer;
};

void ViewFinderGL::releaseResources()
{
    window()->scheduleRenderJob(new CleanupJob(m_renderer), QQuickWindow::BeforeSynchronizingStage);
    m_renderer = nullptr;
}

ViewFinderGLRenderer::ViewFinderGLRenderer() : buffer_(nullptr),
    colorSpace_(libcamera::ColorSpace::Raw), image_(nullptr)
{

}

ViewFinderGLRenderer::~ViewFinderGLRenderer()
{
    delete m_program;
}

void ViewFinderGL::sync()
{
    qDebug() << Q_FUNC_INFO << window()->size() << window()->devicePixelRatio();

    if (!m_renderer) {
        m_renderer = new ViewFinderGLRenderer();
        connect(window(), &QQuickWindow::beforeRendering, m_renderer, &ViewFinderGLRenderer::init, Qt::DirectConnection);
        connect(window(), &QQuickWindow::beforeRenderPassRecording, m_renderer, &ViewFinderGLRenderer::paint, Qt::DirectConnection);
    }
    m_renderer->setViewportSize(window()->size() * window()->devicePixelRatio());
    m_renderer->setWindow(window());
}

void ViewFinderGLRenderer::init()
{
    qDebug() << Q_FUNC_INFO;

    if (!m_program) {
        QSGRendererInterface *rif = m_window->rendererInterface();
        Q_ASSERT(rif->graphicsApi() == QSGRendererInterface::OpenGL);

        if (vertexShaderFile_.isEmpty() || fragmentShaderFile_.isEmpty()) {
            return;
        }

        m_program = new QOpenGLShaderProgram();
        initializeOpenGLFunctions();

        glEnable(GL_TEXTURE_2D);
        glDisable(GL_DEPTH_TEST);
        glViewport(0, 0, m_viewportSize.width(), m_viewportSize.height());

        m_program->link();
        qDebug() << Q_FUNC_INFO << "LINKED";

        float vertices[] = {
            // positions   // texture coords
            -0.5f, -0.5f,  0.0f, 1.0f,
            -0.5f, 0.5f,   0.0f, 0.0f,
            0.5f, -0.5f,   1.0f, 1.0f,
            0.5f,  0.5f,   1.0f, 0.0f
        };

        vertexBuffer_.create();
        vertexBuffer_.bind();
        vertexBuffer_.allocate(vertices, sizeof(vertices));

        /* Create Vertex Shader */
        if (!createVertexShader())
            qWarning() << "[ViewFinderGL]: create vertex shader failed.";

        if (!createFragmentShader()) {
            qWarning() << "[ViewFinderGL]:"
                       << "create fragment shader failed.";
        }

        vertexBuffer_.release();


        m_testPattern = (new QImage(QStringLiteral(":/assets/test_pattern.png")))->convertToFormat(QImage::Format_RGB888);
    }
}

void ViewFinderGLRenderer::paint()
{
    qDebug() << Q_FUNC_INFO << image_;

    if (!m_program) {
        return;
    }

    // Play nice with the RHI. Not strictly needed when the scenegraph uses
    // OpenGL directly.
    m_window->beginExternalCommands();

    vertexBuffer_.bind();

    if (!m_program->bind()) {
        qWarning() << "[ViewFinderGL]:" << m_program->log();
        return;
    }

    if (!image_) {
        return;
    }

    //Position
    m_program->setAttributeBuffer(0,
                                  GL_FLOAT,
                                  0,
                                  2,
                                  4 * sizeof(GLfloat));

    //Texture
    m_program->setAttributeBuffer(1,
                                  GL_FLOAT,
                                  2 * sizeof(GLfloat),
                                  2,
                                  4 * sizeof(GLfloat));


    glActiveTexture(GL_TEXTURE0);
    configureTexture(*textures_[0]);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGBA,
                 stride_ / 4,
                 size_.height(),
                 0,
                 GL_RGBA,
                 GL_UNSIGNED_BYTE,
                 image_->data(0).data());
    m_program->setUniformValue(textureUniformY_, 0);

    /*
     * The shader needs the step between two texture pixels in the
     * horizontal direction, expressed in texture coordinate units
     * ([0, 1]). There are exactly width - 1 steps between the
     * leftmost and rightmost texels.
     */
    m_program->setUniformValue(textureUniformStep_,
                               1.0f / (size_.width() / 2 - 1),
                               1.0f /* not used */);



    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDisable(GL_DEPTH_TEST);

    glActiveTexture(GL_TEXTURE0);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    m_program->release();

    m_window->endExternalCommands();
}

bool ViewFinderGLRenderer::selectFormat(const libcamera::PixelFormat &format)
{
    qDebug() << Q_FUNC_INFO << format.toString();

    bool ret = true;

    /* Set min/mag filters to GL_LINEAR by default. */
    textureMinMagFilters_ = GL_LINEAR;

    /* Use identity.vert as the default vertex shader. */
    vertexShaderFile_ = QStringLiteral(":assets/shaders/identity.vert");

    fragmentShaderDefines_.clear();
    fragmentShaderDefines_.append(QStringLiteral("#version 330 core"));

    switch (format) {
    case libcamera::formats::NV12:
        horzSubSample_ = 2;
        vertSubSample_ = 2;
        fragmentShaderDefines_.append(QStringLiteral("#define YUV_PATTERN_UV"));
        fragmentShaderFile_ = QStringLiteral(":assets/shaders/YUV_2_planes.frag");
        break;
    case libcamera::formats::NV21:
        horzSubSample_ = 2;
        vertSubSample_ = 2;
        fragmentShaderDefines_.append(QStringLiteral("#define YUV_PATTERN_VU"));
        fragmentShaderFile_ = QStringLiteral(":assets/shaders/YUV_2_planes.frag");
        break;
    case libcamera::formats::NV16:
        horzSubSample_ = 2;
        vertSubSample_ = 1;
        fragmentShaderDefines_.append(QStringLiteral("#define YUV_PATTERN_UV"));
        fragmentShaderFile_ = QStringLiteral(":assets/shaders/YUV_2_planes.frag");
        break;
    case libcamera::formats::NV61:
        horzSubSample_ = 2;
        vertSubSample_ = 1;
        fragmentShaderDefines_.append(QStringLiteral("#define YUV_PATTERN_VU"));
        fragmentShaderFile_ = QStringLiteral(":assets/shaders/YUV_2_planes.frag");
        break;
    case libcamera::formats::NV24:
        horzSubSample_ = 1;
        vertSubSample_ = 1;
        fragmentShaderDefines_.append(QStringLiteral("#define YUV_PATTERN_UV"));
        fragmentShaderFile_ = QStringLiteral(":assets/shaders/YUV_2_planes.frag");
        break;
    case libcamera::formats::NV42:
        horzSubSample_ = 1;
        vertSubSample_ = 1;
        fragmentShaderDefines_.append(QStringLiteral("#define YUV_PATTERN_VU"));
        fragmentShaderFile_ = QStringLiteral(":YUV_2_planes.frag");
        break;
    case libcamera::formats::YUV420:
        horzSubSample_ = 2;
        vertSubSample_ = 2;
        fragmentShaderFile_ = QStringLiteral(":assets/shaders/YUV_3_planes.frag");
        break;
    case libcamera::formats::YVU420:
        horzSubSample_ = 2;
        vertSubSample_ = 2;
        fragmentShaderFile_ = QStringLiteral(":assets/shaders/YUV_3_planes.frag");
        break;
    case libcamera::formats::UYVY:
        fragmentShaderDefines_.append(QStringLiteral("#define YUV_PATTERN_UYVY"));
        fragmentShaderFile_ = QStringLiteral(":assets/shaders/YUV_packed.frag");
        break;
    case libcamera::formats::VYUY:
        fragmentShaderDefines_.append(QStringLiteral("#define YUV_PATTERN_VYUY"));
        fragmentShaderFile_ = QStringLiteral(":assets/shaders/YUV_packed.frag");
        break;
    case libcamera::formats::YUYV:
        fragmentShaderDefines_.append(QStringLiteral("#define YUV_PATTERN_YUYV"));
        fragmentShaderFile_ = QStringLiteral(":assets/shaders/YUV_packed.frag");
        //fragmentShaderFile_ = QStringLiteral(":assets/shaders/test.frag");
        break;
    case libcamera::formats::YVYU:
        fragmentShaderDefines_.append(QStringLiteral("#define YUV_PATTERN_YVYU"));
        fragmentShaderFile_ = QStringLiteral(":assets/shaders/YUV_packed.frag");
        break;
    case libcamera::formats::ABGR8888:
        fragmentShaderDefines_.append(QStringLiteral("#define RGB_PATTERN rgb"));
        fragmentShaderFile_ = QStringLiteral(":assets/shaders/RGB.frag");
        break;
    case libcamera::formats::ARGB8888:
        fragmentShaderDefines_.append(QStringLiteral("#define RGB_PATTERN bgr"));
        fragmentShaderFile_ = QStringLiteral(":Rassets/shaders/GB.frag");
        break;
    case libcamera::formats::BGRA8888:
        fragmentShaderDefines_.append(QStringLiteral("#define RGB_PATTERN gba"));
        fragmentShaderFile_ = QStringLiteral(":assets/shaders/RGB.frag");
        break;
    case libcamera::formats::RGBA8888:
        fragmentShaderDefines_.append(QStringLiteral("#define RGB_PATTERN abg"));
        fragmentShaderFile_ = QStringLiteral(":assets/shaders/RGB.frag");
        break;
    case libcamera::formats::BGR888:
        fragmentShaderDefines_.append(QStringLiteral("#define RGB_PATTERN rgb"));
        fragmentShaderFile_ = QStringLiteral(":assets/shaders/RGB.frag");
        break;
    case libcamera::formats::RGB888:
        fragmentShaderDefines_.append(QStringLiteral("#define RGB_PATTERN bgr"));
        fragmentShaderFile_ = QStringLiteral(":assets/shaders/RGB.frag");
        break;
    case libcamera::formats::SBGGR8:
        firstRed_.setX(1.0);
        firstRed_.setY(1.0);
        vertexShaderFile_ = QStringLiteral(":assets/shaders/bayer_8.vert");
        fragmentShaderFile_ = QStringLiteral(":assets/shaders/bayer_8.frag");
        textureMinMagFilters_ = GL_NEAREST;
        break;
    case libcamera::formats::SGBRG8:
        firstRed_.setX(0.0);
        firstRed_.setY(1.0);
        vertexShaderFile_ = QStringLiteral(":assets/shaders/bayer_8.vert");
        fragmentShaderFile_ = QStringLiteral(":bayer_8.frag");
        textureMinMagFilters_ = GL_NEAREST;
        break;
    case libcamera::formats::SGRBG8:
        firstRed_.setX(1.0);
        firstRed_.setY(0.0);
        vertexShaderFile_ = QStringLiteral(":assets/shaders/bayer_8.vert");
        fragmentShaderFile_ = QStringLiteral(":assets/shaders/bayer_8.frag");
        textureMinMagFilters_ = GL_NEAREST;
        break;
    case libcamera::formats::SRGGB8:
        firstRed_.setX(0.0);
        firstRed_.setY(0.0);
        vertexShaderFile_ = QStringLiteral(":assets/shaders/bayer_8.vert");
        fragmentShaderFile_ = QStringLiteral(":assets/shaders/bayer_8.frag");
        textureMinMagFilters_ = GL_NEAREST;
        break;
    case libcamera::formats::SBGGR10_CSI2P:
        firstRed_.setX(1.0);
        firstRed_.setY(1.0);
        fragmentShaderDefines_.append(QStringLiteral("#define RAW10P"));
        fragmentShaderFile_ = QStringLiteral(":assets/shaders/bayer_1x_packed.frag");
        textureMinMagFilters_ = GL_NEAREST;
        break;
    case libcamera::formats::SGBRG10_CSI2P:
        firstRed_.setX(0.0);
        firstRed_.setY(1.0);
        fragmentShaderDefines_.append(QStringLiteral("#define RAW10P"));
        fragmentShaderFile_ = QStringLiteral(":assets/shaders/bayer_1x_packed.frag");
        textureMinMagFilters_ = GL_NEAREST;
        break;
    case libcamera::formats::SGRBG10_CSI2P:
        firstRed_.setX(1.0);
        firstRed_.setY(0.0);
        fragmentShaderDefines_.append(QStringLiteral("#define RAW10P"));
        fragmentShaderFile_ = QStringLiteral(":assets/shaders/bayer_1x_packed.frag");
        textureMinMagFilters_ = GL_NEAREST;
        break;
    case libcamera::formats::SRGGB10_CSI2P:
        firstRed_.setX(0.0);
        firstRed_.setY(0.0);
        fragmentShaderDefines_.append(QStringLiteral("#define RAW10P"));
        fragmentShaderFile_ = QStringLiteral(":assets/shaders/bayer_1x_packed.frag");
        textureMinMagFilters_ = GL_NEAREST;
        break;
    case libcamera::formats::SBGGR12_CSI2P:
        firstRed_.setX(1.0);
        firstRed_.setY(1.0);
        fragmentShaderDefines_.append(QStringLiteral("#define RAW12P"));
        fragmentShaderFile_ = QStringLiteral(":assets/shaders/bayer_1x_packed.frag");
        textureMinMagFilters_ = GL_NEAREST;
        break;
    case libcamera::formats::SGBRG12_CSI2P:
        firstRed_.setX(0.0);
        firstRed_.setY(1.0);
        fragmentShaderDefines_.append(QStringLiteral("#define RAW12P"));
        fragmentShaderFile_ = QStringLiteral(":assets/shaders/bayer_1x_packed.frag");
        textureMinMagFilters_ = GL_NEAREST;
        break;
    case libcamera::formats::SGRBG12_CSI2P:
        firstRed_.setX(1.0);
        firstRed_.setY(0.0);
        fragmentShaderDefines_.append(QStringLiteral("#define RAW12P"));
        fragmentShaderFile_ = QStringLiteral(":assets/shaders/bayer_1x_packed.frag");
        textureMinMagFilters_ = GL_NEAREST;
        break;
    case libcamera::formats::SRGGB12_CSI2P:
        firstRed_.setX(0.0);
        firstRed_.setY(0.0);
        fragmentShaderDefines_.append(QStringLiteral("#define RAW12P"));
        fragmentShaderFile_ = QStringLiteral(":assets/shaders/bayer_1x_packed.frag");
        textureMinMagFilters_ = GL_NEAREST;
        break;
    default:
        ret = false;
        qWarning() << "[ViewFinderGL]:"
                   << "format not supported.";
        break;
    };

    return ret;
}

void ViewFinderGLRenderer::configureTexture(QOpenGLTexture &texture)
{
    qDebug() << Q_FUNC_INFO;

    glBindTexture(GL_TEXTURE_2D, texture.textureId());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                    textureMinMagFilters_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    textureMinMagFilters_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void ViewFinderGLRenderer::selectColorSpace(const libcamera::ColorSpace &colorSpace)
{
    qDebug() << Q_FUNC_INFO;

    std::array<double, 9> yuv2rgb;

    /* OpenGL stores arrays in column-major order. */
    switch (colorSpace.ycbcrEncoding) {
    case libcamera::ColorSpace::YcbcrEncoding::None:
    default:
        yuv2rgb = {
            1.0000,  0.0000,  0.0000,
            0.0000,  1.0000,  0.0000,
            0.0000,  0.0000,  1.0000,
        };
        break;

    case libcamera::ColorSpace::YcbcrEncoding::Rec601:
        yuv2rgb = {
            1.0000,  1.0000,  1.0000,
            0.0000, -0.3441,  1.7720,
            1.4020, -0.7141,  0.0000,
        };
        break;

    case libcamera::ColorSpace::YcbcrEncoding::Rec709:
        yuv2rgb = {
            1.0000,  1.0000,  1.0000,
            0.0000, -0.1873,  1.8856,
            1.5748, -0.4681,  0.0000,
        };
        break;

    case libcamera::ColorSpace::YcbcrEncoding::Rec2020:
        yuv2rgb = {
            1.0000,  1.0000,  1.0000,
            0.0000, -0.1646,  1.8814,
            1.4746, -0.5714,  0.0000,
        };
        break;
    }

    double offset;

    switch (colorSpace.range) {
    case libcamera::ColorSpace::Range::Full:
    default:
        offset = 0.0;
        break;

    case libcamera::ColorSpace::Range::Limited:
        offset = 16.0;

        for (unsigned int i = 0; i < 3; ++i)
            yuv2rgb[i] *= 255.0 / 219.0;
        for (unsigned int i = 4; i < 9; ++i)
            yuv2rgb[i] *= 255.0 / 224.0;
        break;
    }

    QStringList matrix;

    for (double coeff : yuv2rgb)
        matrix.append(QString::number(coeff, 'f'));

    fragmentShaderDefines_.append(QStringLiteral("#define YUV2RGB_MATRIX ") + matrix.join(QStringLiteral(", ")));
    fragmentShaderDefines_.append(QStringLiteral("#define YUV2RGB_Y_OFFSET %1")
                                  .arg(offset, 0, 'f', 1));
}


bool ViewFinderGLRenderer::createVertexShader()
{
    qDebug() << Q_FUNC_INFO << vertexShaderFile_;

    /* Create Vertex Shader */
    vertexShader_ = std::make_unique<QOpenGLShader>(QOpenGLShader::Vertex);

    /* Compile the vertex shader */
    if (!vertexShader_->compileSourceFile(vertexShaderFile_)) {
        qWarning() << "[ViewFinderGL]:" << vertexShader_->log();
        return false;
    }

    m_program->addShader(vertexShader_.get());
    return true;
}

bool ViewFinderGLRenderer::createFragmentShader()
{
    qDebug() << Q_FUNC_INFO << fragmentShaderFile_;

    int attributeVertex;
    int attributeTexture;

    /*
     * Create the fragment shader, compile it, and add it to the shader
     * program. The #define macros stored in fragmentShaderDefines_, if
     * any, are prepended to the source code.
     */
    fragmentShader_ = std::make_unique<QOpenGLShader>(QOpenGLShader::Fragment);

    QFile file(fragmentShaderFile_);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Shader" << fragmentShaderFile_ << "not found";
        return false;
    }

    QString defines = fragmentShaderDefines_.join(QStringLiteral("\n")) + QStringLiteral("\n");
    QByteArray src = file.readAll();
    src.prepend(defines.toUtf8());

    qDebug() << src;

    if (!fragmentShader_->compileSourceCode(src)) {
        qWarning() << "[ViewFinderGL]:" << fragmentShader_->log();
        return false;
    }

    m_program->addShader(fragmentShader_.get());

    /* Link shader pipeline */
    if (!m_program->link()) {
        qWarning() << "[ViewFinderGL]:" << m_program->log();
        //close();
    }

    /*
    attributeVertex = m_program->attributeLocation("vertexIn");
    attributeTexture = m_program->attributeLocation("textureIn");

    qDebug() << Q_FUNC_INFO << attributeVertex << attributeTexture;

    vertexBuffer_.bind();

    m_program->enableAttributeArray(attributeVertex);
    m_program->setAttributeBuffer(attributeVertex,
                                  GL_FLOAT,
                                  0,
                                  2,
                                  2 * sizeof(GLfloat));

    m_program->enableAttributeArray(attributeTexture);
    m_program->setAttributeBuffer(attributeTexture,
                                  GL_FLOAT,
                                  8 * sizeof(GLfloat),
                                  2,
                                  2 * sizeof(GLfloat));

    vertexBuffer_.release();
*/
    projMatrixUniform_ = m_program->uniformLocation("proj_matrix");
    textureUniformY_ = m_program->uniformLocation("tex_y");
    textureUniformU_ = m_program->uniformLocation("tex_u");
    textureUniformV_ = m_program->uniformLocation("tex_v");
    textureUniformStep_ = m_program->uniformLocation("tex_step");
    textureUniformSize_ = m_program->uniformLocation("tex_size");
    textureUniformStrideFactor_ = m_program->uniformLocation("stride_factor");
    textureUniformBayerFirstRed_ = m_program->uniformLocation("tex_bayer_first_red");

    /* Create the textures. */
    for (std::unique_ptr<QOpenGLTexture> &texture : textures_) {
        if (texture)
            continue;

        texture = std::make_unique<QOpenGLTexture>(QOpenGLTexture::Target2D);
        texture->create();
    }

    return true;
}

int ViewFinderGLRenderer::setFormat(const libcamera::PixelFormat &format, const QSize &size,
                                    const libcamera::ColorSpace &colorSpace,
                                    unsigned int stride)
{
    qDebug() << Q_FUNC_INFO << format.toString() << size << colorSpace.toString();

    if (format != format_ || colorSpace != colorSpace_) {
        /*
         * If the fragment already exists, remove it and create a new
         * one for the new format.
         */

        if (m_program && m_program->isLinked()) {
            m_program->release();
            m_program->removeShader(fragmentShader_.get());
            fragmentShader_.reset();
        }

        if (!selectFormat(format)) {
            qDebug() << "Unable to selectFormat";
            return -1;
        }

        selectColorSpace(colorSpace);

        format_ = format;
        colorSpace_ = colorSpace;
    }

    size_ = size;
    stride_ = stride;

    return 0;
}

void ViewFinderGLRenderer::preRender(libcamera::FrameBuffer *buffer, Image *image, QList<QRectF>)
{
    qDebug() << Q_FUNC_INFO;
    buffer_ = buffer;
    image_ = image;
}



//=================================================================================================



const QList<libcamera::PixelFormat> &ViewFinderGL::nativeFormats() const
{
    return supportedFormats;
}

int ViewFinderGL::setFormat(const libcamera::PixelFormat &format, const QSize &size, const libcamera::ColorSpace &colorSpace, unsigned int stride)
{
    qDebug() << Q_FUNC_INFO;

    m_format = format;
    m_colorSpace = colorSpace;
    m_size = size;
    m_stride = stride;

    if (m_renderer) {
        m_renderer->setFormat(format, size, colorSpace, stride);
    }
    return 0;
}

void ViewFinderGL::render(libcamera::FrameBuffer *buffer, Image *image, QList<QRectF> rects)
{
    qDebug() << Q_FUNC_INFO << buffer << image << rects;
    m_renderer->preRender(buffer, image, rects);
    m_buffer = buffer;

    if (window())
        window()->update();

    if (m_buffer)
        Q_EMIT renderComplete(m_buffer);

}

void ViewFinderGL::stop()
{
    qDebug() << Q_FUNC_INFO;
    if (m_buffer) {
        renderComplete(m_buffer);
        m_buffer = nullptr;
    }
}

QImage ViewFinderGL::getCurrentImage()
{
    qDebug() << Q_FUNC_INFO;

    QMutexLocker locker(&mutex_);

    return QImage(); //TODO
}
