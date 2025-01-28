#include "openglviewfinderrendernode.h"
#include <QQuickItem>

#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions>

#include <QFile>
#include <libcamera/libcamera/formats.h>

OpenGLViewFinderRenderNode::OpenGLViewFinderRenderNode() : vertexBuffer_(QOpenGLBuffer::VertexBuffer), colorSpace_(libcamera::ColorSpace::Raw)
{
    qDebug() << Q_FUNC_INFO;
}

OpenGLViewFinderRenderNode::~OpenGLViewFinderRenderNode()

{
    qDebug() << Q_FUNC_INFO;

    releaseResources();
}

void OpenGLViewFinderRenderNode::releaseResources()
{
    qDebug() << Q_FUNC_INFO;
}

void OpenGLViewFinderRenderNode::init()
{
    qDebug() << Q_FUNC_INFO;

    //initializeOpenGLFunctions();
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);

    const GLfloat coordinates[2][4][2]{
        {
            /* Vertex coordinates */
            { -1.0f, -1.0f },
            { -1.0f, +1.0f },
            { +1.0f, +1.0f },
            { +1.0f, -1.0f },
        },
        {
            /* Texture coordinates */
            { 0.0f, 1.0f },
            { 0.0f, 0.0f },
            { 1.0f, 0.0f },
            { 1.0f, 1.0f },
        },
    };

    vertexBuffer_.create();
    vertexBuffer_.bind();
    vertexBuffer_.allocate(coordinates, sizeof(coordinates));

    /* Create Vertex Shader */
    if (!createVertexShader())
        qWarning() << "[ViewFinderGL]: create vertex shader failed.";
}

#if 0
void OpenGLViewFinderRenderNode::paintGL()
{
    if (!fragmentShader_) {
        if (!createFragmentShader()) {
            qWarning() << "[ViewFinderGL]:"
                   << "create fragment shader failed.";
        }
    }

    /* Bind shader pipeline for use. */
    if (!shaderProgram_.bind()) {
        qWarning() << "[ViewFinderGL]:" << shaderProgram_.log();
        close();
    }

    if (!image_)
        return;

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    doRender();
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}
#endif

void OpenGLViewFinderRenderNode::render(const RenderState *state)
{
#if 0
    if (!m_program)
        init();

    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    m_program->bind();
    m_program->setUniformValue(m_matrixUniform, *state->projectionMatrix() * *matrix());
    m_program->setUniformValue(m_opacityUniform, float(inheritedOpacity()));
//! [2]

    m_vbo->bind();

//! [5]
    QPointF p0(m_width - 1, m_height - 1);
    QPointF p1(0, 0);
    QPointF p2(0, m_height - 1);

    GLfloat vertices[6] = { GLfloat(p0.x()), GLfloat(p0.y()),
                            GLfloat(p1.x()), GLfloat(p1.y()),
                            GLfloat(p2.x()), GLfloat(p2.y()) };
    m_vbo->write(0, vertices, sizeof(vertices));
//! [5]

    m_program->setAttributeBuffer(0, GL_FLOAT, 0, 2);
    m_program->setAttributeBuffer(1, GL_FLOAT, sizeof(vertices), 3);
    m_program->enableAttributeArray(0);
    m_program->enableAttributeArray(1);

    // We are prepared both for the legacy (direct OpenGL) and the modern
    // (abstracted by RHI) OpenGL scenegraph. So set all the states that are
    // important to us.

    //! [3]
    f->glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    f->glEnable(GL_BLEND);
    f->glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    // Regardless of flags() returning DepthAwareRendering or not,
    // we have to test against what's in the depth buffer already.
    f->glEnable(GL_DEPTH_TEST);

    // Clip support.
    if (state->scissorEnabled()) {
        f->glEnable(GL_SCISSOR_TEST);
        const QRect r = state->scissorRect(); // already bottom-up
        f->glScissor(r.x(), r.y(), r.width(), r.height());
    }
    if (state->stencilEnabled()) {
        f->glEnable(GL_STENCIL_TEST);
        f->glStencilFunc(GL_EQUAL, state->stencilValue(), 0xFF);
        f->glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    }

    f->glDrawArrays(GL_TRIANGLES, 0, 3);
    //! [3]
    //!
#endif

    qDebug() << Q_FUNC_INFO;

    if (!vertexShader_ && !vertexShaderFile_.isEmpty()) {
        init();
    }

    if (!fragmentShader_ && !fragmentShaderFile_.isEmpty()) {
        if (!createFragmentShader()) {
            qWarning() << "[ViewFinderGL]:"
                   << "create fragment shader failed.";
        }
    }

    /* Bind shader pipeline for use. */
    if (!shaderProgram_.bind()) {
        qWarning() << "[ViewFinderGL]:" << shaderProgram_.log();
        //close();
    }

    if (!image_) {
        qDebug() << "No Image";
        return;
    }

    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

    f->glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    f->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /* Stride of the first plane, in pixels. */
    unsigned int stridePixels;

    switch (format_) {
    case libcamera::formats::NV12:
    case libcamera::formats::NV21:
    case libcamera::formats::NV16:
    case libcamera::formats::NV61:
    case libcamera::formats::NV24:
    case libcamera::formats::NV42:
        /* Activate texture Y */
        f->glActiveTexture(GL_TEXTURE0);
        configureTexture(*textures_[0], f);
        f->glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_LUMINANCE,
                 stride_,
                 size_.height(),
                 0,
                 GL_LUMINANCE,
                 GL_UNSIGNED_BYTE,
                 image_->data(0).data());
        shaderProgram_.setUniformValue(textureUniformY_, 0);

        /* Activate texture UV/VU */
        f->glActiveTexture(GL_TEXTURE1);
        configureTexture(*textures_[1], f);
        f->glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_LUMINANCE_ALPHA,
                 stride_ / horzSubSample_,
                 size_.height() / vertSubSample_,
                 0,
                 GL_LUMINANCE_ALPHA,
                 GL_UNSIGNED_BYTE,
                 image_->data(1).data());
        shaderProgram_.setUniformValue(textureUniformU_, 1);

        stridePixels = stride_;
        break;

    case libcamera::formats::YUV420:
        /* Activate texture Y */
        f->glActiveTexture(GL_TEXTURE0);
        configureTexture(*textures_[0], f);
        f->glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_LUMINANCE,
                 stride_,
                 size_.height(),
                 0,
                 GL_LUMINANCE,
                 GL_UNSIGNED_BYTE,
                 image_->data(0).data());
        shaderProgram_.setUniformValue(textureUniformY_, 0);

        /* Activate texture U */
        f->glActiveTexture(GL_TEXTURE1);
        configureTexture(*textures_[1], f);
        f->glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_LUMINANCE,
                 stride_ / horzSubSample_,
                 size_.height() / vertSubSample_,
                 0,
                 GL_LUMINANCE,
                 GL_UNSIGNED_BYTE,
                 image_->data(1).data());
        shaderProgram_.setUniformValue(textureUniformU_, 1);

        /* Activate texture V */
        f->glActiveTexture(GL_TEXTURE2);
        configureTexture(*textures_[2], f);
        f->glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_LUMINANCE,
                 stride_ / horzSubSample_,
                 size_.height() / vertSubSample_,
                 0,
                 GL_LUMINANCE,
                 GL_UNSIGNED_BYTE,
                 image_->data(2).data());
        shaderProgram_.setUniformValue(textureUniformV_, 2);

        stridePixels = stride_;
        break;

    case libcamera::formats::YVU420:
        /* Activate texture Y */
        f->glActiveTexture(GL_TEXTURE0);
        configureTexture(*textures_[0], f);
        f->glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_LUMINANCE,
                 stride_,
                 size_.height(),
                 0,
                 GL_LUMINANCE,
                 GL_UNSIGNED_BYTE,
                 image_->data(0).data());
        shaderProgram_.setUniformValue(textureUniformY_, 0);

        /* Activate texture V */
        f->glActiveTexture(GL_TEXTURE2);
        configureTexture(*textures_[2], f);
        f->glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_LUMINANCE,
                 stride_ / horzSubSample_,
                 size_.height() / vertSubSample_,
                 0,
                 GL_LUMINANCE,
                 GL_UNSIGNED_BYTE,
                 image_->data(1).data());
        shaderProgram_.setUniformValue(textureUniformV_, 2);

        /* Activate texture U */
        f->glActiveTexture(GL_TEXTURE1);
        configureTexture(*textures_[1], f);
        f->glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_LUMINANCE,
                 stride_ / horzSubSample_,
                 size_.height() / vertSubSample_,
                 0,
                 GL_LUMINANCE,
                 GL_UNSIGNED_BYTE,
                 image_->data(2).data());
        shaderProgram_.setUniformValue(textureUniformU_, 1);

        stridePixels = stride_;
        break;

    case libcamera::formats::UYVY:
    case libcamera::formats::VYUY:
    case libcamera::formats::YUYV:
    case libcamera::formats::YVYU:
        /*
         * Packed YUV formats are stored in a RGBA texture to match the
         * OpenGL texel size with the 4 bytes repeating pattern in YUV.
         * The texture width is thus half of the image_ with.
         */
        f->glActiveTexture(GL_TEXTURE0);
        configureTexture(*textures_[0], f);
        f->glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGBA,
                 stride_ / 4,
                 size_.height(),
                 0,
                 GL_RGBA,
                 GL_UNSIGNED_BYTE,
                 image_->data(0).data());
        shaderProgram_.setUniformValue(textureUniformY_, 0);

        /*
         * The shader needs the step between two texture pixels in the
         * horizontal direction, expressed in texture coordinate units
         * ([0, 1]). There are exactly width - 1 steps between the
         * leftmost and rightmost texels.
         */
        shaderProgram_.setUniformValue(textureUniformStep_,
                           1.0f / (size_.width() / 2 - 1),
                           1.0f /* not used */);

        stridePixels = stride_ / 2;
        break;

    case libcamera::formats::ABGR8888:
    case libcamera::formats::ARGB8888:
    case libcamera::formats::BGRA8888:
    case libcamera::formats::RGBA8888:
        f->glActiveTexture(GL_TEXTURE0);
        configureTexture(*textures_[0], f);
        f->glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGBA,
                 stride_ / 4,
                 size_.height(),
                 0,
                 GL_RGBA,
                 GL_UNSIGNED_BYTE,
                 image_->data(0).data());
        shaderProgram_.setUniformValue(textureUniformY_, 0);

        stridePixels = stride_ / 4;
        break;

    case libcamera::formats::BGR888:
    case libcamera::formats::RGB888:
        f->glActiveTexture(GL_TEXTURE0);
        configureTexture(*textures_[0], f);
        f->glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGB,
                 stride_ / 3,
                 size_.height(),
                 0,
                 GL_RGB,
                 GL_UNSIGNED_BYTE,
                 image_->data(0).data());
        shaderProgram_.setUniformValue(textureUniformY_, 0);

        stridePixels = stride_ / 3;
        break;

    case libcamera::formats::SBGGR8:
    case libcamera::formats::SGBRG8:
    case libcamera::formats::SGRBG8:
    case libcamera::formats::SRGGB8:
    case libcamera::formats::SBGGR10_CSI2P:
    case libcamera::formats::SGBRG10_CSI2P:
    case libcamera::formats::SGRBG10_CSI2P:
    case libcamera::formats::SRGGB10_CSI2P:
    case libcamera::formats::SBGGR12_CSI2P:
    case libcamera::formats::SGBRG12_CSI2P:
    case libcamera::formats::SGRBG12_CSI2P:
    case libcamera::formats::SRGGB12_CSI2P:
        /*
         * Raw Bayer 8-bit, and packed raw Bayer 10-bit/12-bit formats
         * are stored in a GL_LUMINANCE texture. The texture width is
         * equal to the stride.
         */
        f->glActiveTexture(GL_TEXTURE0);
        configureTexture(*textures_[0], f);
        f->glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_LUMINANCE,
                 stride_,
                 size_.height(),
                 0,
                 GL_LUMINANCE,
                 GL_UNSIGNED_BYTE,
                 image_->data(0).data());
        shaderProgram_.setUniformValue(textureUniformY_, 0);
        shaderProgram_.setUniformValue(textureUniformBayerFirstRed_,
                           firstRed_);
        shaderProgram_.setUniformValue(textureUniformSize_,
                           size_.width(), /* in pixels */
                           size_.height());
        shaderProgram_.setUniformValue(textureUniformStep_,
                           1.0f / (stride_ - 1),
                           1.0f / (size_.height() - 1));

        /*
         * The stride is already taken into account in the shaders, set
         * the generic stride factor to 1.0.
         */
        stridePixels = size_.width();
        break;

    default:
        stridePixels = size_.width();
        break;
    };

    /*
     * Compute the stride factor for the vertex shader, to map the
     * horizontal texture coordinate range [0.0, 1.0] to the active portion
     * of the image.
     */
    shaderProgram_.setUniformValue(textureUniformStrideFactor_,
                       static_cast<float>(size_.width() - 1) /
                       (stridePixels - 1));

    /*
     * Place the viewfinder in the centre of the widget, preserving the
     * aspect ratio of the image.
     */
    QMatrix4x4 projMatrix;
    QSizeF scaledSize = size_.scaled(rect().size().toSize(), Qt::KeepAspectRatio);
    projMatrix.scale(scaledSize.width() / rect().size().width(),
             scaledSize.height() / rect().size().height());

    shaderProgram_.setUniformValue(projMatrixUniform_, projMatrix);

    f->glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

//! [4]
QSGRenderNode::StateFlags OpenGLViewFinderRenderNode::changedStates() const
{
    qDebug() << Q_FUNC_INFO;

    return BlendState | ScissorState | StencilState | DepthState;
}

QSGRenderNode::RenderingFlags OpenGLViewFinderRenderNode::flags() const
{
    qDebug() << Q_FUNC_INFO;

    return BoundedRectRendering | DepthAwareRendering;
}

QRectF OpenGLViewFinderRenderNode::rect() const
{
    qDebug() << Q_FUNC_INFO;

    return QRect(0, 0, m_width, m_height);
}

void OpenGLViewFinderRenderNode::sync(QQuickItem *item)
{
    qDebug() << Q_FUNC_INFO;

    m_width = item->width();
    m_height = item->height();
}



int OpenGLViewFinderRenderNode::setFormat(const libcamera::PixelFormat &format, const QSize &size,
                const libcamera::ColorSpace &colorSpace,
                unsigned int stride)
{
    qDebug() << Q_FUNC_INFO << format.toString() << size << colorSpace.toString();

    if (format != format_ || colorSpace != colorSpace_) {
        /*
         * If the fragment already exists, remove it and create a new
         * one for the new format.
         */
        if (shaderProgram_.isLinked()) {
            shaderProgram_.release();
            shaderProgram_.removeShader(fragmentShader_.get());
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

    //updateGeometry();

    return 0;
}

void OpenGLViewFinderRenderNode::preRender(libcamera::FrameBuffer *buffer, Image *image, QList<QRectF>)
{
    qDebug() << Q_FUNC_INFO;
    buffer_ = buffer;
    image_ = image;
}

void OpenGLViewFinderRenderNode::selectColorSpace(const libcamera::ColorSpace &colorSpace)
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

bool OpenGLViewFinderRenderNode::createVertexShader()
{
    qDebug() << Q_FUNC_INFO << vertexShaderFile_;

    /* Create Vertex Shader */
    vertexShader_ = std::make_unique<QOpenGLShader>(QOpenGLShader::Vertex);

    /* Compile the vertex shader */
    if (!vertexShader_->compileSourceFile(vertexShaderFile_)) {
        qWarning() << "[ViewFinderGL]:" << vertexShader_->log();
        return false;
    }

    shaderProgram_.addShader(vertexShader_.get());
    return true;
}

bool OpenGLViewFinderRenderNode::createFragmentShader()
{
    qDebug() << Q_FUNC_INFO;

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

    if (!fragmentShader_->compileSourceCode(src)) {
        qWarning() << "[ViewFinderGL]:" << fragmentShader_->log();
        return false;
    }

    shaderProgram_.addShader(fragmentShader_.get());

    /* Link shader pipeline */
    if (!shaderProgram_.link()) {
        qWarning() << "[ViewFinderGL]:" << shaderProgram_.log();
        //close();
    }

    attributeVertex = shaderProgram_.attributeLocation("vertexIn");
    attributeTexture = shaderProgram_.attributeLocation("textureIn");

    vertexBuffer_.bind();

    shaderProgram_.enableAttributeArray(attributeVertex);
    shaderProgram_.setAttributeBuffer(attributeVertex,
                      GL_FLOAT,
                      0,
                      2,
                      2 * sizeof(GLfloat));

    shaderProgram_.enableAttributeArray(attributeTexture);
    shaderProgram_.setAttributeBuffer(attributeTexture,
                      GL_FLOAT,
                      8 * sizeof(GLfloat),
                      2,
                      2 * sizeof(GLfloat));

    vertexBuffer_.release();

    projMatrixUniform_ = shaderProgram_.uniformLocation("proj_matrix");
    textureUniformY_ = shaderProgram_.uniformLocation("tex_y");
    textureUniformU_ = shaderProgram_.uniformLocation("tex_u");
    textureUniformV_ = shaderProgram_.uniformLocation("tex_v");
    textureUniformStep_ = shaderProgram_.uniformLocation("tex_step");
    textureUniformSize_ = shaderProgram_.uniformLocation("tex_size");
    textureUniformStrideFactor_ = shaderProgram_.uniformLocation("stride_factor");
    textureUniformBayerFirstRed_ = shaderProgram_.uniformLocation("tex_bayer_first_red");

    /* Create the textures. */
    for (std::unique_ptr<QOpenGLTexture> &texture : textures_) {
        if (texture)
            continue;

        texture = std::make_unique<QOpenGLTexture>(QOpenGLTexture::Target2D);
        texture->create();
    }

    return true;
}

void OpenGLViewFinderRenderNode::configureTexture(QOpenGLTexture &texture, QOpenGLFunctions *f)
{
    qDebug() << Q_FUNC_INFO;

    f->glBindTexture(GL_TEXTURE_2D, texture.textureId());
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
            textureMinMagFilters_);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
            textureMinMagFilters_);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void OpenGLViewFinderRenderNode::removeShader()
{
    qDebug() << Q_FUNC_INFO;

    if (shaderProgram_.isLinked()) {
        shaderProgram_.release();
        shaderProgram_.removeAllShaders();
    }
}

bool OpenGLViewFinderRenderNode::selectFormat(const libcamera::PixelFormat &format)
{
    qDebug() << Q_FUNC_INFO << format.toString();

    bool ret = true;

    /* Set min/mag filters to GL_LINEAR by default. */
    textureMinMagFilters_ = GL_LINEAR;

    /* Use identity.vert as the default vertex shader. */
    vertexShaderFile_ = QStringLiteral(":assets/shaders/identity.vert");

    fragmentShaderDefines_.clear();

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
