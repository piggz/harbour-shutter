#ifndef VIEWFINDERGL_H
#define VIEWFINDERGL_H

#include <QtQuick/QQuickItem>
#include <QtQuick/QQuickWindow>
#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions>
#include <QOpenGLTexture>
#include <QOpenGLBuffer>

#include <QMutex>

#include <libcamera/libcamera/color_space.h>
#include <libcamera/libcamera/framebuffer.h>
#include <libcamera/libcamera/pixel_format.h>

#include "viewfinder.h"

class ViewFinderGLRenderer : public QObject, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    ViewFinderGLRenderer();
    ~ViewFinderGLRenderer();

    void setViewportSize(const QSize &size) { m_viewportSize = size; }
    void setWindow(QQuickWindow *window) { m_window = window; }

    int setFormat(const libcamera::PixelFormat &format, const QSize &size,
                  const libcamera::ColorSpace &colorSpace,
                  unsigned int stride);
    void preRender(libcamera::FrameBuffer *buffer, Image *image, QList<QRectF>);

public Q_SLOTS:
    void init();
    void paint();

private:
    QSize m_viewportSize;
    QOpenGLShaderProgram *m_program = nullptr;
    QQuickWindow *m_window = nullptr;
    QImage m_testPattern;

    /* Captured image size, format and buffer */
    libcamera::FrameBuffer *buffer_;
    libcamera::PixelFormat format_;
    libcamera::ColorSpace colorSpace_;
    Image *image_;
    unsigned int stride_;
    QSize size_;


    /* Shaders */
    std::unique_ptr<QOpenGLShader> vertexShader_;
    std::unique_ptr<QOpenGLShader> fragmentShader_;
    QString vertexShaderFile_;
    QString fragmentShaderFile_;
    QStringList fragmentShaderDefines_;

    /* Vertex buffer */
    QOpenGLBuffer vertexBuffer_;
    QOpenGLBuffer m_indexBuffer{QOpenGLBuffer::IndexBuffer};

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

    bool selectFormat(const libcamera::PixelFormat &format);
    void selectColorSpace(const libcamera::ColorSpace &colorSpace);

    void configureTexture(QOpenGLTexture &texture);
    bool createFragmentShader();
    bool createVertexShader();
    void removeShader();
    void doRender();
};

class ViewFinderGL : public QQuickItem, public ViewFinder
{
    Q_OBJECT
    QML_ELEMENT

public:
    ViewFinderGL();

    const QList<libcamera::PixelFormat> &nativeFormats() const override;
    int setFormat(const libcamera::PixelFormat &format, const QSize &size,
                  const libcamera::ColorSpace &colorSpace,
                  unsigned int stride) override;
    void render(libcamera::FrameBuffer *buffer, Image *image, QList<QRectF>) override;
    void stop() override;
    QImage getCurrentImage() override;

public Q_SLOTS:
    void sync();
    void cleanup();

private Q_SLOTS:
    void handleWindowChanged(QQuickWindow *win);

Q_SIGNALS:
    void renderComplete();

private:
    void releaseResources() override;
    ViewFinderGLRenderer *m_renderer;

    /* Captured image size, format and buffer */
    libcamera::FrameBuffer *m_buffer;
    libcamera::PixelFormat m_format;
    libcamera::ColorSpace m_colorSpace;
    QSize m_size;
    unsigned int m_stride;
    Image *image_ = nullptr;
    QMutex mutex_; /* Prevent concurrent access to image_ */

};

#endif // VIEWFINDERGL_H
