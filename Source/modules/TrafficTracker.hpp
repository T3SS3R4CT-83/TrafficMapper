#pragma once

//#include <memory>

#include <opencv2/dnn/dnn.hpp>
//#include <opencv2/tracking/tracker.hpp>

#include <QObject>
#include <QUrl>
#include <QMutex>
//#include <QWaitCondition>

#include <TrafficMapper/Classes/Detection>
#include <TrafficMapper/Modules/GateModel>

class Vehicle;
//class QUrl;

class TrafficTracker : public QObject
{
	Q_OBJECT

    GateModel *m_gateModel_ptr;

	std::unordered_map<int, std::vector<Detection>> m_detections;
	std::vector<Vehicle *> m_vehicles;
	std::unordered_map<int, std::vector<Vehicle *>> m_trajectories;

	std::unordered_map<const Gate*, std::unordered_map<VehicleType, std::vector<int>>> m_statistics;
	QStringList m_stat_axisX_values;
	int m_stat_axisY_maxval;
	std::unordered_map<VehicleType, std::vector<int>> m_stat_vtype_values;

	QMutex m_runningMutex;
	bool m_isRunning;

public:
	TrafficTracker();
	~TrafficTracker();

    void setGateModel(GateModel *_gateModel);

    Q_INVOKABLE void extractDetectionData(QUrl _cacheFileUrl);
	Q_INVOKABLE void analizeVideo();

	Q_INVOKABLE void terminate();

	Q_INVOKABLE void openCacheFile(QUrl _fileUrl);
	Q_INVOKABLE void exportFrames();

	std::vector<Vehicle*> getVehiclesOnFrame(const int frameIdx);
	std::vector<Detection> getDetections(const int frameIdx) const;

	Q_INVOKABLE void generateStatistics(Gate * _gate_ptr, const int _interval);
	Q_INVOKABLE QStringList getAxisX();
	Q_INVOKABLE int getAxisY();
	Q_INVOKABLE QList<QVariant> getCarValues();
	Q_INVOKABLE QList<QVariant> getTruckValues();
	Q_INVOKABLE QList<QVariant> getBusValues();

	void onFrameDisplayed(int _frameIdx);

private:
//	bool isRunning();
	inline cv::dnn::Net initNeuralNet();
	inline std::vector<Detection> getRawFrameDetections(const cv::Mat& _frame, cv::dnn::Net &_net);
	//inline void filterFrameDetections(std::vector<Detection>& _frameDetections);

	inline void prepIOUmatrix(std::vector<std::vector<double>>& iouMatrix, const int & _tNum, const int & _dNum, std::vector<Detection>& prevDetections, std::vector<Detection>& frameDetections);

signals:
	void processTerminated();
	void progressUpdated(int _currentFrameIdx, int _allFrameNr);
//	void analysisStarted();
};
