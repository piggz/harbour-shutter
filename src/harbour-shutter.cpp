#include <QApplication>
#include <QQmlApplicationEngine>
#include <QtQml>
#include <QUrl>
#include <QQuickStyle>
#include <QQmlContext>
#include <QQuickItem>
#include <QSortFilterProxyModel>

#include <libcamera/camera_manager.h>

#include "cameramodel.h"
#include "resolutionmodel.h"
#include "focusmodel.h"
#include "flashmodel.h"
#include "fsoperations.h"
#include "resourcehandler.h"
#include "storagemodel.h"
#include "exifmodel.h"
#include "formatmodel.h"
#include "metadatamodel.h"
#include "viewfinder2d.h"
#include "cameraproxy.h"
#include "settings.h"
#include "controlmodel.h"
#include "viewfinder3d.h"
#include "viewfindergl.h"

int main(int argc, char *argv[])
{
    QQuickStyle::setStyle(QStringLiteral("Material"));
    qputenv("QT_QUICK_CONTROLS_MATERIAL_THEME", QByteArray("Dark"));

    QApplication app(argc, argv);
    QQmlApplicationEngine engine;

    QApplication::setOrganizationDomain(QStringLiteral("piggz.co.uk"));
    QApplication::setOrganizationName(QStringLiteral("uk.co.piggz")); // needed for Sailjail
    QApplication::setApplicationName(QStringLiteral("shutter"));

    std::shared_ptr<libcamera::CameraManager> cm = std::make_shared<libcamera::CameraManager>();

    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);

    int ret = cm->start();
    if (ret) {
        qInfo() << "Failed to start camera manager:" << strerror(-ret);
        return EXIT_FAILURE;
    }

    CameraModel cameraModel(nullptr, cm);

    qmlRegisterType<FocusModel>("uk.co.piggz.shutter", 1, 0, "FocusModel");
    qmlRegisterType<FlashModel>("uk.co.piggz.shutter", 1, 0, "FlashModel");
    qmlRegisterType<ExifModel>("uk.co.piggz.shutter", 1, 0, "ExifModel");
    qmlRegisterType<MetadataModel>("uk.co.piggz.shutter", 1, 0, "MetadataModel");
    qmlRegisterUncreatableType<FormatModel>("uk.co.piggz.shutter", 1, 0, "FormatModel", QStringLiteral("Not to be created within QML"));
    qmlRegisterUncreatableType<ResolutionModel>("uk.co.piggz.shutter", 1, 0, "ResolutionModel", QStringLiteral("Not to be created within QML"));
    qmlRegisterUncreatableType<ControlModel>("uk.co.piggz.shutter", 1, 0, "ControlModel", QStringLiteral("Not to be created within QML"));
    qmlRegisterType<ViewFinder2D>("uk.co.piggz.shutter", 1, 0, "ViewFinder2D");
    qmlRegisterType<Settings>("uk.co.piggz.shutter", 1, 0, "Settings");
    qmlRegisterType<CameraProxy>("uk.co.piggz.shutter", 1, 0, "CameraProxy");
    qmlRegisterType<ViewFinder3D>("uk.co.piggz.shutter", 1, 0, "ViewFinder3D");
    qmlRegisterType<ViewFinderGL>("uk.co.piggz.shutter", 1, 0, "ViewFinderGL");

    ResourceHandler handler(&app);
    handler.acquire();

    // We do not need to pass settings to QML by using setContextProperty, because Settings is a
    // wrapper around QSettings via its m_settings member. Because the object instantiated in QML
    // and the one in C++ will use QSettings, they will use the same app-global settings store anyway.
    Settings settings(&app);

    StorageModel storageModel(&app);
    engine.rootContext()->setContextProperty(QStringLiteral("modelStorage"), (QObject*)&storageModel);

    FSOperations fsOperations(&app);
    engine.rootContext()->setContextProperty(QStringLiteral("fsOperations"), &fsOperations);

    std::shared_ptr<CameraProxy> cameraProxy = std::make_shared<CameraProxy>();
    cameraProxy->setCameraManager(cm);
    cameraProxy->setSettings(&settings);
    engine.rootContext()->setContextProperty(QStringLiteral("cameraProxy"), (QObject*)cameraProxy.get());

    FormatModel formatModel(&app);
    formatModel.setCameraProxy(cameraProxy);
    engine.rootContext()->setContextProperty(QStringLiteral("modelFormats"), (QObject*)&formatModel);

    ResolutionModel resolutionModel(&app);
    resolutionModel.setCameraProxy(cameraProxy);

    ControlModel controlModel(&app);
    controlModel.setCameraProxy(cameraProxy);
    engine.rootContext()->setContextProperty(QStringLiteral("modelControls"), (QObject*)&controlModel);


    QSortFilterProxyModel sortedResolutionModel;
    sortedResolutionModel.setSourceModel(&resolutionModel);
    sortedResolutionModel.setSortRole(ResolutionModel::ResolutionMpx);
    sortedResolutionModel.sort(0, Qt::DescendingOrder);
    engine.rootContext()->setContextProperty(QStringLiteral("modelResolutions"), (QObject*)&resolutionModel);
    engine.rootContext()->setContextProperty(QStringLiteral("modelCamera"), (QObject*)&cameraModel);
    engine.rootContext()->setContextProperty(QStringLiteral("modelResolution"), (QObject*)&resolutionModel);
    engine.rootContext()->setContextProperty(QStringLiteral("sortedModelResolution"), &sortedResolutionModel);


    engine.loadFromModule("uk.co.piggz.shutter", "Main");

    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    return app.exec();
}
