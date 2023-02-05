#ifdef QT_QML_DEBUG
#include <QtQuick>
#endif

#include <QQuickView>
#include <QGuiApplication>
#include <QQmlContext>
#include <QQuickItem>
#include <QSortFilterProxyModel>

#ifdef MER_EDITION_SAILFISH
#include <sailfishapp.h>
#include "deviceinfo.h"
#else
#include <QQmlApplicationEngine>
#endif

#include <libcamera/camera_manager.h>

#include "cameramodel.h"
#include "effectsmodel.h"
#include "exposuremodel.h"
#include "isomodel.h"
#include "resolutionmodel.h"
#include "wbmodel.h"
#include "focusmodel.h"
#include "flashmodel.h"
#include "fsoperations.h"
#include "resourcehandler.h"
#include "storagemodel.h"
#include "exifmodel.h"
#include "formatmodel.h"
#include "metadatamodel.h"
#include "viewfinderitem.h"
#include "viewfinder2d.h"
#include "cameraproxy.h"
#include "settings.h"

int main(int argc, char *argv[])
{
    // SailfishApp::main() will display "qml/harbour-advanced-camera.qml", if you need more
    // control over initialization, you can use:
    //
    //   - SailfishApp::application(int, char *[]) to get the QGuiApplication *
    //   - SailfishApp::createView() to get a new QQuickView * instance
    //   - SailfishApp::pathTo(QString) to get a QUrl to a resource file
    //   - SailfishApp::pathToMainQml() to get a QUrl to the main QML file
    //
    // To display the view, call "show()" (will show fullscreen on device).

    QGuiApplication *app;
#ifdef MER_EDITION_SAILFISH
    app = SailfishApp::application(argc, argv);
#else
    app = new QGuiApplication(argc, argv);
#endif

    app->setOrganizationDomain("piggz.co.uk");
    app->setOrganizationName("uk.co.piggz"); // needed for Sailjail
    app->setApplicationName("pinhole");

    std::shared_ptr<libcamera::CameraManager> cm = std::make_shared<libcamera::CameraManager>();

    int ret = cm->start();
    if (ret) {
        qInfo() << "Failed to start camera manager:" << strerror(-ret);
        return EXIT_FAILURE;
    }

    CameraModel cameraModel(0, cm);

    qmlRegisterType<EffectsModel>("uk.co.piggz.pinhole", 1, 0, "EffectsModel");
    qmlRegisterType<ExposureModel>("uk.co.piggz.pinhole", 1, 0, "ExposureModel");
    qmlRegisterType<IsoModel>("uk.co.piggz.pinhole", 1, 0, "IsoModel");
    qmlRegisterType<WbModel>("uk.co.piggz.pinhole", 1, 0, "WhiteBalanceModel");
    qmlRegisterType<FocusModel>("uk.co.piggz.pinhole", 1, 0, "FocusModel");
    qmlRegisterType<FlashModel>("uk.co.piggz.pinhole", 1, 0, "FlashModel");
    qmlRegisterType<ExifModel>("uk.co.piggz.pinhole", 1, 0, "ExifModel");
    qmlRegisterType<MetadataModel>("uk.co.piggz.pinhole", 1, 0, "MetadataModel");
    qmlRegisterUncreatableType<FormatModel>("uk.co.piggz.pinhole", 1, 0, "FormatModel", "Not to be created within QML");
    qmlRegisterUncreatableType<ResolutionModel>("uk.co.piggz.pinhole", 1, 0, "ResolutionModel", "Not to be created within QML");
    qmlRegisterType<ViewFinderItem>("uk.co.piggz.pinhole", 1, 0, "ViewFinderItem");
    qmlRegisterType<ViewFinder2D>("uk.co.piggz.pinhole", 1, 0, "ViewFinder2D");
    qmlRegisterType<Settings>("uk.co.piggz.pinhole", 1, 0, "Settings");
    qmlRegisterType<CameraProxy>("uk.co.piggz.pinhole", 1, 0, "CameraProxy");


#ifdef IS_SAILFISH_OS

#endif
#ifdef IS_QTCONTROLS_QT

#endif

#ifdef MER_EDITION_SAILFISH
    QQuickView *view = SailfishApp::createView();
    QQmlContext *rootContext = view->rootContext();
#else
    QQmlApplicationEngine engine;
    QQmlContext *rootContext = engine.rootContext();
#endif

    if (!rootContext)
    {
        qDebug() << "Failed to initialize QML context\n";
        return -2;
    }

    ResourceHandler handler(app);
    handler.acquire();

    StorageModel storageModel(app);
    rootContext->setContextProperty("modelStorage", &storageModel);

    FSOperations fsOperations(app);
    rootContext->setContextProperty("fsOperations", &fsOperations);

    std::shared_ptr<CameraProxy> cameraProxy = std::make_shared<CameraProxy>();
    cameraProxy->setCameraManager(cm);
    rootContext->setContextProperty("cameraProxy", cameraProxy.get());

    FormatModel formatModel(app);
    formatModel.setCameraProxy(cameraProxy);
    rootContext->setContextProperty("modelFormats", &formatModel);

    ResolutionModel resolutionModel(app);
    resolutionModel.setCameraProxy(cameraProxy);

    QSortFilterProxyModel sortedResolutionModel;
    sortedResolutionModel.setSourceModel(&resolutionModel);
    sortedResolutionModel.setSortRole(ResolutionModel::ResolutionMpx);
    sortedResolutionModel.sort(0, Qt::DescendingOrder);
    rootContext->setContextProperty("modelResolutions", &resolutionModel);
    rootContext->setContextProperty("modelCamera", &cameraModel);
    rootContext->setContextProperty("modelResolution", &resolutionModel);
    rootContext->setContextProperty("sortedModelResolution", &sortedResolutionModel);

#ifdef MER_EDITION_SAILFISH
    DeviceInfo deviceInfo;
    rootContext->setContextProperty("CameraManufacturer", deviceInfo.manufacturer());
    rootContext->setContextProperty("CameraPrettyModelName", deviceInfo.prettyModelName());
#endif

 //   QObject::connect(&fsOperations, &FSOperations::rescan, &storageModel,
 //                    &StorageModel::scan);

#ifdef MER_EDITION_SAILFISH
    QObject::connect(view, &QQuickView::focusObjectChanged, &handler,
                     &ResourceHandler::handleFocusChange);

    view->setSource(SailfishApp::pathTo("qml/harbour-pinhole.qml"));
    view->show();
#else
    engine.load(QUrl("qrc:/qml/harbour-pinhole.qml"));
#endif

    return app->exec();
}
