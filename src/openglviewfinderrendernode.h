#ifndef OPENGLVIEWFINDERRENDERNODE_H
#define OPENGLVIEWFINDERRENDERNODE_H

#include "image.h"
#include <QSGRenderNode>
#include <QQuickItem>

#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLShader>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <libcamera/libcamera/color_space.h>
#include <libcamera/libcamera/framebuffer.h>
#include <libcamera/libcamera/pixel_format.h>

class QOpenGLShaderProgram;
class QOpenGLBuffer;

class OpenGLViewFinderRenderNode : public QSGRenderNode
{
public:
    OpenGLViewFinderRenderNode();
    ~OpenGLViewFinderRenderNode();

    void render(const RenderState *state) override;
    void releaseResources() override;
    StateFlags changedStates() const override;
    RenderingFlags flags() const override;
    QRectF rect() const override;
    //! [1]

    void sync(QQuickItem *item);

    int setFormat(const libcamera::PixelFormat &format, const QSize &size,
                  const libcamera::ColorSpace &colorSpace,
                  unsigned int stride);

    void preRender(libcamera::FrameBuffer *buffer, Image *image, QList<QRectF>);
private:
    void init(QOpenGLFunctions *f);

    int m_width = 0;
    int m_height = 0;
    int m_matrixUniform;
    int m_opacityUniform;

    /* Captured image size, format and buffer */
    libcamera::FrameBuffer *buffer_;
    libcamera::PixelFormat format_;
    libcamera::ColorSpace colorSpace_;
    QSize size_;
    unsigned int stride_;
    Image *image_;


    bool selectFormat(const libcamera::PixelFormat &format);
    void selectColorSpace(const libcamera::ColorSpace &colorSpace);

    void configureTexture(QOpenGLTexture &texture, QOpenGLFunctions *f);
    bool createFragmentShader();
    bool createVertexShader();
    void removeShader();


    /* Shaders */
    QOpenGLShaderProgram shaderProgram_;
    std::unique_ptr<QOpenGLShader> vertexShader_;
    std::unique_ptr<QOpenGLShader> fragmentShader_;
    QString vertexShaderFile_;
    QString fragmentShaderFile_;
    QStringList fragmentShaderDefines_;

    /* Vertex buffer */
    QOpenGLBuffer vertexBuffer_;

    /* Textures */
    std::array<std::unique_ptr<QOpenGLTexture>, 3> textures_;

    /* Common texture parameters */
    GLuint textureMinMagFilters_;
    GLuint projMatrixUniform_;

    /* YUV texture parameters */
    GLuint textureUniformU_;
    GLuint textureUniformV_;
    GLuint textureUniformY_;
    GLuint textureUniformStep_;
    unsigned int horzSubSample_;
    unsigned int vertSubSample_;

    /* Raw Bayer texture parameters */
    GLuint textureUniformSize_;
    GLuint textureUniformStrideFactor_;
    GLuint textureUniformBayerFirstRed_;
    QPointF firstRed_;


    QImage *m_testPattern;
};

#endif // OPENGLVIEWFINDERRENDERNODE_H
