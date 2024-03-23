#ifndef VIEWFINDERRENDERER_H
#define VIEWFINDERRENDERER_H

#include <QObject>
#include <QSize>
#include <QQuickWindow>
#include <QMutex>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLShader>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QQuickFramebufferObject>

#include <libcamera/formats.h>
#include <libcamera/framebuffer.h>

#include "viewfinder.h"

class ViewFinderRenderer : public QQuickFramebufferObject::Renderer,
        public ViewFinder,
        protected QOpenGLFunctions
{
public:
    ViewFinderRenderer();
    ~ViewFinderRenderer();

    virtual void render() override;
    void init();

    const QList<libcamera::PixelFormat> &nativeFormats() const override;

    int setFormat(const libcamera::PixelFormat &format, const QSize &size,
                  const libcamera::ColorSpace &colorSpace,
                  unsigned int stride) override;
    void renderImage(libcamera::FrameBuffer *buffer, Image *image, QList<QRectF>) override;
    void stop() override;

private:
    QSize m_viewportSize;
    //QOpenGLShaderProgram *m_program;
    QQuickWindow *m_window;

    bool selectFormat(const libcamera::PixelFormat &format);
    void selectColorSpace(const libcamera::ColorSpace &colorSpace);

    void configureTexture(QOpenGLTexture &texture);
    bool createFragmentShader();
    bool createVertexShader();
    void removeShader();
    void doRender();

    /* Captured image size, format and buffer */
    libcamera::FrameBuffer *buffer_;
    libcamera::PixelFormat format_;
    libcamera::ColorSpace colorSpace_;
    QSize size_;
    unsigned int stride_;
    Image *image_;

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

    QMutex mutex_; /* Prevent concurrent access to image_ */

    QOpenGLShaderProgram *m_program;

    bool m_hadConfig = false;

};

#endif // VIEWFINDERRENDERER_H
