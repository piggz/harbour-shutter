#ifndef VIEWFINDERITEM_H
#define VIEWFINDERITEM_H

#include <QQuickFramebufferObject>
#include "viewfinderrenderer.h"

class ViewFinderItem : public QQuickFramebufferObject
{
    Q_OBJECT

    public:
        ViewFinderItem();
        virtual QQuickFramebufferObject::Renderer *	createRenderer() const override;

    public Q_SLOTS:
        void sync();
        void cleanup();
        ViewFinderRenderer *renderer();

    private Q_SLOTS:
        void handleWindowChanged(QQuickWindow *win);

    private:
        void releaseResources() override;

        ViewFinderRenderer *m_renderer;

};

#endif // VIEWFINDERITEM_H
