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




//#include "opencv2/highgui/highgui.hpp"
//#include "opencv2/video/tracking.hpp"
//#include <Windows.h>
//
//#define drawCross(center,color,d)                                 \
//line( img, Point( center.x - d, center.y - d ), Point( center.x + d, center.y + d ), color, 2, cv::LINE_AA, 0); \
//line( img, Point( center.x + d, center.y - d ), Point( center.x - d, center.y + d ), color, 2, cv::LINE_AA, 0 )
//
//using namespace cv;
//using namespace std;
//
//int main()
//{
//    KalmanFilter KF(4, 2, 0);
//    POINT mousePos;
//    GetCursorPos(&mousePos);
//
//    // intialization of KF...
//    KF.transitionMatrix = (Mat_<float>(4, 4) << 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1);
//    Mat_<float> measurement(2, 1); measurement.setTo(Scalar(0));
//
//    KF.statePre.at<float>(0) = mousePos.x;
//    KF.statePre.at<float>(1) = mousePos.y;
//    KF.statePre.at<float>(2) = 0;
//    KF.statePre.at<float>(3) = 0;
//    setIdentity(KF.measurementMatrix);
//    setIdentity(KF.processNoiseCov, Scalar::all(1e-4));
//    setIdentity(KF.measurementNoiseCov, Scalar::all(10));
//    setIdentity(KF.errorCovPost, Scalar::all(.1));
//    // Image to show mouse tracking
//    Mat img(600, 800, CV_8UC3);
//    vector<Point> mousev, kalmanv;
//    mousev.clear();
//    kalmanv.clear();
//
//    while (1)
//    {
//        // First predict, to update the internal statePre variable
//        Mat prediction = KF.predict();
//        Point predictPt(prediction.at<float>(0), prediction.at<float>(1));
//
//        // Get mouse point
//        GetCursorPos(&mousePos);
//        measurement(0) = mousePos.x;
//        measurement(1) = mousePos.y;
//
//        // The update phase 
//        Mat estimated = KF.correct(measurement);
//
//        Point statePt(estimated.at<float>(0), estimated.at<float>(1));
//        Point measPt(measurement(0), measurement(1));
//        // plot points
//        imshow("mouse kalman", img);
//        img = Scalar::all(0);
//
//        mousev.push_back(measPt);
//        kalmanv.push_back(statePt);
//        drawCross(statePt, Scalar(255, 255, 255), 5);
//        drawCross(measPt, Scalar(0, 0, 255), 5);
//
//        for (int i = 0; i < mousev.size() - 1; i++)
//            line(img, mousev[i], mousev[i + 1], Scalar(255, 255, 0), 1);
//
//        for (int i = 0; i < kalmanv.size() - 1; i++)
//            line(img, kalmanv[i], kalmanv[i + 1], Scalar(0, 155, 255), 1);
//
//        waitKey(10);
//    }
//
//    return 0;
//}
