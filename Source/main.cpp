#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QQMLContext>
#include <QFileDialog>
#include <QSplashScreen>

#include <TrafficMapper/Modules/TrafficTracker>
#include <TrafficMapper/Modules/VideoFilter>
#include <TrafficMapper/Modules/GateModel>
#include <TrafficMapper/Modules/StatModel>
#include <TrafficMapper/Classes/Gate>
#include <TrafficMapper/Globals>

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QQuickStyle::setStyle("Fusion");

    QApplication app(argc, argv);
    app.setApplicationName("TrafficMapper");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("ELTE - IK");
    app.setOrganizationDomain("www.inf.elte.hu");

    QPixmap splashImg(":/img/placeholder.png");
    QSplashScreen splashScreen(splashImg);
    splashScreen.show();

    qmlRegisterType<Gate>("com.elte.t3ss3r4ct", 1, 0, "Gate");
    qmlRegisterType<GateModel>("com.elte.t3ss3r4ct", 1, 0, "GateModel");
    qmlRegisterType<TrafficTracker>("com.elte.t3ss3r4ct", 1, 0, "TrafficTracker");
    qmlRegisterType<VideoFilter>("com.elte.t3ss3r4ct", 1, 0, "VideoFilter");

    qmlRegisterSingletonType<GlobalMeta>("com.elte.t3ss3r4ct", 1, 0, "GlobalMeta",
        [](QQmlEngine* engine, QJSEngine* scriptEngine) -> QObject* {
            Q_UNUSED(engine)
            Q_UNUSED(scriptEngine)

            return GlobalMeta::getInstance();
        });

    GateModel gateModel;
    TrafficTracker tracker;
    VideoFilter videoFilter;
    StatModel statModel;

    tracker.setGateModel(&gateModel);
    tracker.setStatModel(&statModel);
    statModel.setGateModel(&gateModel);
    videoFilter.setTracker(&tracker);

    QObject::connect(&videoFilter, &VideoFilter::frameDisplayed,
        &gateModel, &GateModel::onFrameDisplayed);
    QObject::connect(&gateModel, &GateModel::vehiclePassed,
        &statModel, &StatModel::onGatePass);
    QObject::connect(&tracker, &TrafficTracker::analysisStarted,
        &statModel, &StatModel::initModel);

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("gateModel"), &gateModel);
    engine.rootContext()->setContextProperty(QStringLiteral("statModel"), &statModel);
    engine.rootContext()->setContextProperty(QStringLiteral("tracker"), &tracker);
    engine.rootContext()->setContextProperty(QStringLiteral("videoFilter"), &videoFilter);

    const QUrl url(QStringLiteral("qrc:/qml/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
        &app, [url](QObject * obj, const QUrl & objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        }, Qt::QueuedConnection);
    engine.load(url);

    QObject * qmlOpenVideoDialog = engine.rootObjects()[0]->findChild<QObject *>("dlgOpenVideo");
    if (qmlOpenVideoDialog != nullptr)
    {
        QObject::connect(qmlOpenVideoDialog, SIGNAL(videoFileOpened(QUrl)),
            &tracker, SLOT(onVideoFileOpened(QUrl)));
    }

    splashScreen.finish(nullptr);

    return app.exec();
}
