
#include <TrafficMapper/Globals>

#include <opencv2/tracking/tracker.hpp>

std::istream& operator>>(std::istream& _is, VehicleType& _vType)
{
    std::string value;
    _is >> value;
    _vType = VehicleType(std::stoi(value));

    return _is;
}

std::ostream& operator<<(std::ostream& _is, const VehicleType& _vType)
{
    _is << static_cast<int>(_vType);

    return _is;
}

namespace Settings
{
#ifdef YOLOv3_SPP
    const char * DETECTOR_CONFIG_PATH       = "T:\\Models\\yolov3-spp.cfg";
    const char * DETECTOR_WEIGHTS_PATH      = "T:\\Models\\yolov3-spp.weights";
    const cv::Size DETECTOR_DNN_BLOB_SIZE   = cv::Size(416, 416);
#endif // !YOLOv3_SPP
#ifdef YOLOv3_608
    const char * DETECTOR_CONFIG_PATH       = "T:\\Models\\yolov3.cfg";
    const char * DETECTOR_WEIGHTS_PATH      = "T:\\Models\\yolov3.weights";
    const cv::Size DETECTOR_DNN_BLOB_SIZE   = cv::Size(608, 608);
#endif // !YOLOv3_608
#ifdef YOLOv3_416
    const char * DETECTOR_CONFIG_PATH       = "T:\\Models\\yolov3.cfg";
    const char * DETECTOR_WEIGHTS_PATH      = "T:\\Models\\yolov3.weights";
    const cv::Size DETECTOR_DNN_BLOB_SIZE   = cv::Size(416, 416);
#endif // !YOLO3v3_416
#ifdef YOLOv3_320
    const char * DETECTOR_CONFIG_PATH       = "T:\\Models\\yolov3.cfg";
    const char * DETECTOR_WEIGHTS_PATH      = "T:\\Models\\yolov3.weights";
    const cv::Size DETECTOR_DNN_BLOB_SIZE   = cv::Size(320, 320);
#endif // !YOLOv3_320

    const float DETECTOR_CONF_THRESHOLD = 0.5f;
    const float DETECTOR_NMS_THRESHOLD = 0.5f;

    cv::dnn::Backend DETECTOR_BACKEND = cv::dnn::Backend::DNN_BACKEND_DEFAULT;
    cv::dnn::Target DETECTOR_TARGET = cv::dnn::Target::DNN_TARGET_CPU;
    //cv::dnn::Target DETECTOR_TARGET = cv::dnn::Target::DNN_TARGET_OPENCL_FP16;

    const int DETECTOR_CLIP_TRESHOLD = 30;
    const std::unordered_map<VehicleType, QString> DETECTOR_CLASSES = {
		{ VehicleType::undefined,   QStringLiteral("undefined") },
		{ VehicleType::BICYCLE,     QStringLiteral("bicycle") },
		{ VehicleType::CAR,         QStringLiteral("car") },
		{ VehicleType::MOTORCYCLE,  QStringLiteral("motorcycle") },
		{ VehicleType::BUS,         QStringLiteral("bus") },
		{ VehicleType::TRUCK,       QStringLiteral("truck") },
	};

#ifdef TRACKER_GOTURN
    cv::Ptr<cv::TrackerGOTURN> (*TRACKER_ALGORITHM)() = &cv::TrackerGOTURN::create;
#endif // !TRACKER_GOTURN
#ifdef TRACKER_KCF
    cv::Ptr<cv::TrackerKCF> (*TRACKER_ALGORITHM)() = &cv::TrackerKCF::create;    // Higher accuracy
#endif // !TRACKER_KCF
#ifdef TRACKER_CSRT
    cv::Ptr<cv::TrackerCSRT> (*TRACKER_ALGORITHM)() = &cv::TrackerCSRT::create;     // Balanced between accuracy and speed
#endif // !TRACKER_CSRT
#ifdef TRACKER_MOSSE
    cv::Ptr<cv::TrackerMOSSE> (*TRACKER_ALGORITHM)() = &cv::TrackerMOSSE::create;  // Faster speed
#endif // !TRACKER_MOSSE

    const int TRACKER_VISUAL_TRACKING_LENGTH = 80;
    const float TRACKER_IOU_TRESHOLD = -0.3f;

    const int FRAME_BUFFER_SIZE = 2;
};



GlobalMeta *GlobalMeta::instance = nullptr;

QString& GlobalMeta::VIDEO_URL()
{
    return m_VIDEO_URL;
}

int& GlobalMeta::VIDEO_WIDTH()
{
    return m_VIDEO_WIDTH;
}

int& GlobalMeta::VIDEO_HEIGHT()
{
    return m_VIDEO_HEIGHT;
}

float& GlobalMeta::VIDEO_FPS()
{
    return m_VIDEO_FPS;
}

int& GlobalMeta::VIDEO_LENGTH()
{
    return m_VIDEO_LENGTH;
}

int& GlobalMeta::VIDEO_FRAMECOUNT()
{
    return m_VIDEO_FRAMECOUNT;
}

bool& GlobalMeta::PLAYER_SHOW_DETECTIONS()
{
    return m_PLAYER_SHOW_DETECTIONS;
}

bool& GlobalMeta::PLAYER_SHOW_PATHS()
{
    return m_PLAYER_SHOW_PATHS;
}

bool& GlobalMeta::PLAYER_SHOW_LABELS()
{
    return m_PLAYER_SHOW_LABELS;
}

bool& GlobalMeta::PLAYER_SHOW_POSITIONS()
{
    return m_PLAYER_SHOW_POSITIONS;
}

GlobalMeta* GlobalMeta::getInstance()
{
    if (instance == nullptr)
        instance = new GlobalMeta();
    return instance;
}

GlobalMeta::GlobalMeta(QObject* parent)
    : m_VIDEO_URL(""), m_VIDEO_WIDTH(0), m_VIDEO_HEIGHT(0), m_VIDEO_FPS(0), m_VIDEO_LENGTH(0), m_VIDEO_FRAMECOUNT(0),
    m_PLAYER_SHOW_DETECTIONS(true), m_PLAYER_SHOW_PATHS(true), m_PLAYER_SHOW_LABELS(true), m_PLAYER_SHOW_POSITIONS(true)

{ }
