#pragma once

#include <opencv2/dnn/dnn.hpp>

#include <QObject>
#include <QUrl>
#include <QMediaPlayer>

#include <TrafficMapper/Types/Detection>

class FrameProvider;
class VehicleModel;
class CameraCalibration;

class Tracker : public QObject
{
	Q_OBJECT

	Q_PROPERTY(int cacheSize READ getCacheSize NOTIFY cacheLoaded)

	FrameProvider * m_frameProvider_ptr;

	std::unordered_map<int, std::vector<Detection>> m_detections;

	bool m_isRunning;

public:

	Tracker(QObject * parent = nullptr);

	void setFrameProvider(FrameProvider * frameProvider_ptr);

	Q_INVOKABLE void openCacheFile(QUrl fileUrl);
	Q_INVOKABLE void analizeVideo(bool useGPU);
	Q_INVOKABLE void stop();

public slots:

	void onVideoLoading(QMediaPlayer::MediaStatus status);

private:

	size_t getCacheSize() const;

	inline cv::dnn::Net initYOLO(bool useGPU);
	inline std::vector<Detection> getRawFrameDetections(const cv::Mat & frame, cv::dnn::Net & net);
	inline void filterFrameDetections(std::vector<Detection> & frameDetections);
	inline void prepIOUmatrix(std::vector<std::vector<double>> & iouMatrix,
		std::vector<Detection> & prevDetections,
		std::vector<Detection> & frameDetections);

signals:
	
	void analysisStarted();
	void analysisEnded();

	// Pipeline output signal
	void pipelineOutput(Vehicle *);

	// Signals towards QML
	void cacheLoaded();
	void progressUpdated(const int &, const int &);
};
