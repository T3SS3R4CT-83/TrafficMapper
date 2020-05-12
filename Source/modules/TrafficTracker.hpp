#pragma once


#include <opencv2/dnn/dnn.hpp>

#include <QObject>
#include <QMutex>
#include <QVariant>


class Vehicle;
class Gate;
class Detection;
class GateModel;
class StatModel;

enum class VehicleType : int;


class TrafficTracker : public QObject
{
	Q_OBJECT

	GateModel * m_gateModel_ptr;
	StatModel * m_statModel_ptr;

	std::unordered_map<int, std::vector<Detection>> m_detections;
	std::vector<Vehicle *> m_vehicles;
	std::unordered_map<int, std::vector<Vehicle *>> m_trajectories;

	std::unordered_map<const Gate *, std::unordered_map<VehicleType, std::vector<int>>> m_statistics;
	QStringList m_stat_axisX_values;
	int m_stat_axisY_maxval;
	std::unordered_map<VehicleType, std::vector<int>> m_stat_vtype_values;

	QMutex m_runningMutex;
	bool m_isRunning;

public:
	TrafficTracker();
	~TrafficTracker();

	void setGateModel(GateModel * gateModel_ptr);
	void setStatModel(StatModel * statModel_ptr);

	Q_INVOKABLE void extractDetectionData(const QUrl & cacheFileUrl);
	Q_INVOKABLE void analizeVideo();

	Q_INVOKABLE void terminate();

	Q_INVOKABLE void openCacheFile(const QUrl & fileUrl);

	std::vector<Vehicle *> getVehiclesOnFrame(const int & frameIdx);
	std::vector<Detection> getDetections(const int & frameIdx) const;

	Q_INVOKABLE void generateStatistics(Gate * gate_ptr, const int & interval);
	Q_INVOKABLE QStringList getAxisX();
	Q_INVOKABLE int getAxisY();
	Q_INVOKABLE QList<QVariant> getCarValues();
	Q_INVOKABLE QList<QVariant> getTruckValues();
	Q_INVOKABLE QList<QVariant> getBusValues();

	void onFrameDisplayed(const int & frameIdx);

private:
	inline cv::dnn::Net initNeuralNet();
	inline std::vector<Detection> getRawFrameDetections(const cv::Mat & frame, cv::dnn::Net & net);

	inline void prepIOUmatrix(std::vector<std::vector<double>> & iouMatrix,
		const int & tNum, const int & dNum,
		std::vector<Detection> & prevDetections,
		std::vector<Detection> & frameDetections);

signals:
	void processTerminated();
	void progressUpdated(const int & currentFrameIdx, const int & allFrameNr);
	void analysisStarted();
};
