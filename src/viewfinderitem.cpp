#include "viewfinderitem.h"
#include <QQuickWindow>

ViewFinderItem::ViewFinderItem() : m_renderer(nullptr)
{
    qDebug() << Q_FUNC_INFO;
    connect(this, &QQuickItem::windowChanged, this, &ViewFinderItem::handleWindowChanged);
}

QQuickFramebufferObject::Renderer *ViewFinderItem::createRenderer() const
{
    if (!m_renderer) {
        return new ViewFinderRenderer();
    }
    return m_renderer;
}

void ViewFinderItem::handleWindowChanged(QQuickWindow *win)
{
    qDebug() << Q_FUNC_INFO;
    if (win) {
        connect(win, &QQuickWindow::beforeSynchronizing, this, &ViewFinderItem::sync, Qt::DirectConnection);
        connect(win, &QQuickWindow::sceneGraphInvalidated, this, &ViewFinderItem::cleanup, Qt::DirectConnection);
        // Ensure we start with cleared to black. The squircle's blend mode relies on this.
        win->setColor(Qt::black);
    }
}

void ViewFinderItem::releaseResources()
{
    m_renderer = nullptr;
}

void ViewFinderItem::sync()
{
    qDebug() << Q_FUNC_INFO;
    if (!m_renderer) {
        m_renderer = dynamic_cast<ViewFinderRenderer*>(createRenderer());
        //connect(window(), &QQuickWindow::beforeRendering, m_renderer, &ViewFinderRenderer::init, Qt::DirectConnection);
        //connect(window(), &QQuickWindow::beforeRenderPassRecording, m_renderer, &ViewFinderRenderer::render, Qt::DirectConnection);
    }
    qDebug() << window()->size() << window()->devicePixelRatio();
}

void ViewFinderItem::cleanup()
{
    delete m_renderer;
    m_renderer = nullptr;
}

ViewFinderRenderer *ViewFinderItem::renderer()
{
    return m_renderer;
}
