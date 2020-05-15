#pragma once

#include <opencv2/dnn/dnn.hpp>
#include <opencv2/tracking/tracker.hpp>

#include <QObject>
#include <QString>

#include <TrafficMapper/Types>


// TRACKER_GOTURN
// TRACKER_KCF
// TRACKER_CSRT
// TRACKER_MOSSE
#define TRACKER_MOSSE



namespace Settings
{
    extern std::string DETECTOR_CONFIG_PATH;
    extern std::string DETECTOR_WEIGHTS_PATH;
    extern cv::Size DETECTOR_DNN_BLOB_SIZE;

    extern const float DETECTOR_CONF_THRESHOLD;
    extern const float DETECTOR_NMS_THRESHOLD;

    extern const int DETECTOR_CLIP_TRESHOLD;
	extern const std::unordered_map<VehicleType, QString> DETECTOR_CLASSES;

#ifdef TRACKER_GOTURN
    extern cv::Ptr<cv::TrackerGOTURN> (*TRACKER_ALGORITHM)();
#endif // !TRACKER_GOTURN
#ifdef TRACKER_KCF
    extern cv::Ptr<cv::TrackerKCF> (*TRACKER_ALGORITHM)();
#endif // !TRACKER_KCF
#ifdef TRACKER_CSRT
    extern cv::Ptr<cv::TrackerCSRT> (*TRACKER_ALGORITHM)();
#endif // !TRACKER_CSRT
#ifdef TRACKER_MOSSE
    extern cv::Ptr<cv::TrackerMOSSE> (*TRACKER_ALGORITHM)();
#endif // !TRACKER_MOSSE

	extern const int TRACKER_VISUAL_TRACKING_LENGTH;
	extern const float TRACKER_IOU_TRESHOLD;

    extern const int FRAME_BUFFER_SIZE;
};



class GlobalMeta : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString  VIDEO_URL               MEMBER m_VIDEO_URL              NOTIFY propertyChanged)
    Q_PROPERTY(int      VIDEO_WIDTH             MEMBER m_VIDEO_WIDTH            NOTIFY propertyChanged)
    Q_PROPERTY(int      VIDEO_HEIGHT            MEMBER m_VIDEO_HEIGHT           NOTIFY propertyChanged)
    Q_PROPERTY(float    VIDEO_FPS               MEMBER m_VIDEO_FPS              NOTIFY propertyChanged)
    Q_PROPERTY(int      VIDEO_LENGTH            MEMBER m_VIDEO_LENGTH           NOTIFY propertyChanged)
    Q_PROPERTY(int      VIDEO_FRAMECOUNT        MEMBER m_VIDEO_FRAMECOUNT       NOTIFY propertyChanged)

    Q_PROPERTY(bool     PLAYER_SHOW_DETECTIONS  MEMBER m_PLAYER_SHOW_DETECTIONS NOTIFY propertyChanged)
    Q_PROPERTY(bool     PLAYER_SHOW_PATHS       MEMBER m_PLAYER_SHOW_PATHS      NOTIFY propertyChanged)
    Q_PROPERTY(bool     PLAYER_SHOW_LABELS      MEMBER m_PLAYER_SHOW_LABELS     NOTIFY propertyChanged)
    Q_PROPERTY(bool     PLAYER_SHOW_POSITIONS   MEMBER m_PLAYER_SHOW_POSITIONS  NOTIFY propertyChanged)

    QString m_VIDEO_URL;
    int m_VIDEO_WIDTH;
    int m_VIDEO_HEIGHT;
    float m_VIDEO_FPS;
    int m_VIDEO_LENGTH;
    int m_VIDEO_FRAMECOUNT;

    bool m_PLAYER_SHOW_DETECTIONS;
    bool m_PLAYER_SHOW_PATHS;
    bool m_PLAYER_SHOW_LABELS;
    bool m_PLAYER_SHOW_POSITIONS;

public:

    ~GlobalMeta() {}

    QString& VIDEO_URL();
    int& VIDEO_WIDTH();
    int& VIDEO_HEIGHT();
    float& VIDEO_FPS();
    int& VIDEO_LENGTH();
    int& VIDEO_FRAMECOUNT();

    bool& PLAYER_SHOW_DETECTIONS();
    bool& PLAYER_SHOW_PATHS();
    bool& PLAYER_SHOW_LABELS();
    bool& PLAYER_SHOW_POSITIONS();
    
    static GlobalMeta* getInstance();

private:
    static GlobalMeta* instance;

    GlobalMeta(QObject* parent = nullptr);

signals:
    void propertyChanged();
};
