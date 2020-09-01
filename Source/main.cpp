#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QSplashScreen>
#include <QQMLContext>

#include <TrafficMapper/Complementary/CameraCalibration>
#include <TrafficMapper/Media/MediaPlayer>
#include <TrafficMapper/Media/VideoOverlay>
#include <TrafficMapper/Modules/Tracker>
#include <TrafficMapper/Modules/VehicleModel>
#include <TrafficMapper/Modules/GateModel>
#include <TrafficMapper/Modules/StatModel>
#include <TrafficMapper/Types/Gate>
#include <TrafficMapper/Types/Types>

int main(int argc, char * argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QQuickStyle::setStyle("Fusion");

    QApplication app(argc, argv);  // Maybe: QGuiApplication
    app.setApplicationName("TrafficMapper");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("ELTE - IK");
    app.setOrganizationDomain("www.inf.elte.hu");


    //QPixmap splashImg(":/img/splash.png");
    //splashImg = splashImg.scaledToHeight(540);
    //QSplashScreen splashScreen(splashImg);
    QSplashScreen splashScreen(QPixmap(QStringLiteral(":/img/splash.png")).scaledToHeight(540));
    splashScreen.show();

    qRegisterMetaType<size_t>("size_t");
    qRegisterMetaType<uint>("uint");
    qRegisterMetaType<VideoMeta>("VideoMeta");
    qRegisterMetaType<VideoOverlay>("VideoOverlay");

    qmlRegisterType<MediaPlayer>("TrafficMapper", 1, 0, "MediaPlayer");
    qmlRegisterType<VideoOverlay>("TrafficMapper", 1, 0, "VideoOverlay");
    qmlRegisterType<CameraCalibration>("TrafficMapper", 1, 0, "CameraCalibration");
    qmlRegisterType<GateModel>("TrafficMapper", 1, 0, "GateModel");
    qmlRegisterType<Tracker>("TrafficMapper", 1, 0, "Tracker");
    qmlRegisterType<Gate>("TrafficMapper", 1, 0, "Gate");

    MediaPlayer mediaPlayer;
    VideoOverlay videoOverlay;

    Tracker tracker;
    VehicleModel vehicleModel;
    GateModel gateModel;
    StatModel statModel;

    tracker.setFrameProvider(&mediaPlayer);
    videoOverlay.setVehicleModel(&vehicleModel);
    videoOverlay.setGateModel(&gateModel);
    mediaPlayer.setVehicleModel(&vehicleModel);
    mediaPlayer.setGateModel(&gateModel);

    // PIPELINE CONNECTIONS
    {
        // Tracker -> VehicleModel
        QObject::connect(&tracker, &Tracker::pipelineOutput,
            &vehicleModel, &VehicleModel::pipelineInput);

        // VehicleModel -> GateModel
        QObject::connect(&vehicleModel, &VehicleModel::pipelineOutput,
            &gateModel, &GateModel::pipelineInput);

        // GateModel -> StatModel
        QObject::connect(&gateModel, &GateModel::pipelineOutput,
            &statModel, &StatModel::pipelineInput);
    }

    // STARTING PIPELINE MODULES
    {
        // Tracker started -> Start VehicleModel
        QObject::connect(&tracker, &Tracker::analysisStarted,
            &vehicleModel, &VehicleModel::onAnalysisStarted);

        // Tracker started -> Start GateModel
        QObject::connect(&tracker, &Tracker::analysisStarted,
            &gateModel, &GateModel::onAnalysisStarted);

        // Tracker started -> Start StatModel
        QObject::connect(&tracker, &Tracker::analysisStarted,
            &statModel, &StatModel::onAnalysisStarted);
    }

    // DAISY-CHAINING STOP SIGNAL
    {
        // Tracker stopped -> Initialize VehicleModel stopping process
        QObject::connect(&tracker, &Tracker::analysisEnded,
            &vehicleModel, &VehicleModel::onAnalysisEnded);

        // VehicleModel stopped -> Initialize GateModel stopping process
        QObject::connect(&vehicleModel, &VehicleModel::analysisEnded,
            &gateModel, &GateModel::onAnalysisEnded);

        // GateModel stopped -> Initialize StatModel stopping process
        QObject::connect(&gateModel, &GateModel::analysisEnded,
            &statModel, &StatModel::onAnalysisEnded);
    }

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("mediaPlayer"), &mediaPlayer);
    engine.rootContext()->setContextProperty(QStringLiteral("videoOverlay"), &videoOverlay);
    engine.rootContext()->setContextProperty(QStringLiteral("tracker"), &tracker);
    engine.rootContext()->setContextProperty(QStringLiteral("vehicleModel"), &vehicleModel);
    engine.rootContext()->setContextProperty(QStringLiteral("gateModel"), &gateModel);
    engine.rootContext()->setContextProperty(QStringLiteral("statModel"), &statModel);
    engine.load(QUrl(QStringLiteral("qrc:/qml/MainWindow.qml")));

    splashScreen.finish(nullptr);

	return app.exec();
}
