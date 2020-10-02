#pragma once

#include <vector>

#include <opencv2/core/mat.hpp>

#include <QObject>
#include <QQueue>
#include <QMutex>
#include <QWaitCondition>

class Vehicle;
class VehicleModelWorker;
class CameraCalibration;
class MediaPlayer;
class Tracker;

class VehicleModel : public QObject
{
	Q_OBJECT

	Q_PROPERTY(int isCameraCalibrated READ isCameraCalibrated NOTIFY cameraCalibrationStateChanged)

	//VehicleModelWorker * m_worker_ptr;

	std::vector<Vehicle *> m_vehicles;
	std::unordered_map<int, std::vector<Vehicle *>> m_vehicleMap;

	bool m_threadRunning;
	QQueue<Vehicle *> m_buffer;
	QMutex m_bufferMutex;
	QWaitCondition m_bufferNotEmpty;

	QMutex m_finishedMutex;
	QWaitCondition m_finished;

	bool m_isCameraCalibrated;
	cv::Mat m_Homography;

public:

	VehicleModel();
	~VehicleModel();

	Q_INVOKABLE void loadHomographyMatrix(CameraCalibration * calibrationModule);
	Q_INVOKABLE void recalculateSpeeds();

	friend class VideoFilter;
	friend class MediaPlayer;

public slots:

	void onAnalysisStarted();
	void onAnalysisEnded();
	
	void pipelineInput(Vehicle * vehicle_ptr);

private:

	bool isCameraCalibrated() const;

	inline void vehiclePostProcess(Vehicle * vehicle_ptr);
	inline void deleteVehicles();

signals:

	void pipelineOutput(Vehicle *);
	void analysisEnded();

	void cameraCalibrationStateChanged();
};
