#pragma once

#include <opencv2/dnn/dnn.hpp>
#include <opencv2/tracking/tracker.hpp>

#include <QObject>
#include <QString>


// YOLOv3_SPP (mAP: 60.6 - FLOPS: 141.45 Bn - FPS: 20)
// YOLOv3_608 (mAP: 57.9 - FLOPS: 140.69 Bn - FPS: 20)
// YOLOv3_416 (mAP: 55.3 - FLOPS:  65.86 Bn - FPS: 35)
// YOLOv3_320 (mAP: 51.5 - FLOPS:  38.97 Bn - FPS: 45)
#define YOLOv3_608

// TRACKER_GOTURN
// TRACKER_KCF
// TRACKER_CSRT
// TRACKER_MOSSE
#define TRACKER_MOSSE


enum class VehicleType {
    undefined = -1,
    CAR = 0,
    BUS = 1,
    TRUCK = 2,
    MOTORCYCLE = 3,
    BICYCLE = 4
};
std::istream& operator>>(std::istream& _is, VehicleType& _vType);
std::ostream& operator<<(std::ostream& _is, const VehicleType& _vType);

namespace Settings
{
    extern const char * DETECTOR_CONFIG_PATH;
    extern const char * DETECTOR_WEIGHTS_PATH;
    extern const cv::Size DETECTOR_DNN_BLOB_SIZE;

    extern const float DETECTOR_CONF_THRESHOLD;
    extern const float DETECTOR_NMS_THRESHOLD;

    extern cv::dnn::Backend DETECTOR_BACKEND;
	extern cv::dnn::Target DETECTOR_TARGET;
    
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


//    inline std::string const &const1() {
//        static std::string ret = "hello, world!";
//        return ret;
//    }
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



#include <tuple>

template <typename T,
    typename TIter = decltype(std::begin(std::declval<T>())),
    typename = decltype(std::end(std::declval<T>()))>
    constexpr auto enumerate(T&& iterable)
{
    struct iterator
    {
        size_t i;
        TIter iter;
        bool operator != (const iterator& other) const { return iter != other.iter; }
        void operator ++ () { ++i; ++iter; }
        auto operator * () const { return std::tie(i, *iter); }
    };
    struct iterable_wrapper
    {
        T iterable;
        auto begin() { return iterator{ 0, std::begin(iterable) }; }
        auto end() { return iterator{ 0, std::end(iterable) }; }
    };
    return iterable_wrapper{ std::forward<T>(iterable) };
}
