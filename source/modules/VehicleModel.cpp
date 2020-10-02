#include "VehicleModel.hpp"

#include <QThread>
#include <QtConcurrent/QtConcurrent>

#include <TrafficMapper/Complementary/CameraCalibration>
#include <TrafficMapper/Modules/Tracker>
#include <TrafficMapper/Types/Vehicle>
#include <TrafficMapper/Types/Detection>

#include <cppitertools/sliding_window.hpp>



VehicleModel::VehicleModel()
	: QObject(), m_isCameraCalibrated(false)
{
}

VehicleModel::~VehicleModel()
{
	deleteVehicles();
}



void VehicleModel::pipelineInput(Vehicle * vehicle_ptr)
{
	if (vehicle_ptr->weaksFallFirst())
	{
		delete vehicle_ptr;
		return;
	}

	m_bufferMutex.lock();
	m_buffer.enqueue(vehicle_ptr);
	m_bufferNotEmpty.wakeAll();
	m_bufferMutex.unlock();
}

bool VehicleModel::isCameraCalibrated() const
{
	return m_isCameraCalibrated;
}

inline void VehicleModel::vehiclePostProcess(Vehicle * vehicle_ptr)
{
	m_vehicles.push_back(vehicle_ptr);

	vehicle_ptr->calcVehicleType();
	vehicle_ptr->calcPositions();
	if (m_isCameraCalibrated)
		vehicle_ptr->calcVehicleSpeed(m_Homography);

	for (int frameIdx(vehicle_ptr->m_firstFrame); frameIdx <= vehicle_ptr->m_lastFrame; ++frameIdx)
		m_vehicleMap[frameIdx].push_back(vehicle_ptr);

	emit pipelineOutput(vehicle_ptr);
}

inline void VehicleModel::deleteVehicles()
{
	for (auto vehicle_ptr : m_vehicles)
		delete vehicle_ptr;

	m_vehicles.clear();
}

void VehicleModel::loadHomographyMatrix(CameraCalibration * calibrationModule)
{
	if (calibrationModule->m_pointSet == 4)
	{
		m_Homography = calibrationModule->m_homography_i2p.clone();
		m_isCameraCalibrated = true;
		emit cameraCalibrationStateChanged();

		//qDebug() << m_Homography.at<float>(0, 0) << m_Homography.at<float>(0, 1) << m_Homography.at<float>(0, 2) << m_Homography.at<float>(0, 3);
		//qDebug() << m_Homography.at<float>(1, 0) << m_Homography.at<float>(1, 1) << m_Homography.at<float>(1, 2) << m_Homography.at<float>(1, 3);
		//qDebug() << m_Homography.at<float>(2, 0) << m_Homography.at<float>(2, 1) << m_Homography.at<float>(2, 2) << m_Homography.at<float>(2, 3);
		//qDebug() << m_Homography.at<float>(3, 0) << m_Homography.at<float>(3, 1) << m_Homography.at<float>(3, 2) << m_Homography.at<float>(3, 3);
	}
}

void VehicleModel::recalculateSpeeds()
{
	if (m_isCameraCalibrated)
	{
		for (auto vehicle_ptr : m_vehicles)
		{
			vehicle_ptr->calcVehicleSpeed(m_Homography);
		}
	}
}

 

void VehicleModel::onAnalysisStarted()
{
	deleteVehicles();
	m_vehicleMap.clear();

	m_threadRunning = true;

	QtConcurrent::run([this]()
	{
		qDebug() << "VehicleModel worker started!";

		for (; m_threadRunning || !m_buffer.isEmpty();)
		{
			m_bufferMutex.lock();
			if (m_buffer.isEmpty())
				m_bufferNotEmpty.wait(&m_bufferMutex);

			if (!m_buffer.isEmpty())
				vehiclePostProcess(m_buffer.dequeue());
			m_bufferMutex.unlock();
		}

		emit analysisEnded();

		qDebug() << "VehicleModel worker finished!";
	});
}

void VehicleModel::onAnalysisEnded()
{
	qDebug() << "VehicleModel worker received stop signal!";

	m_threadRunning = false;
	m_bufferNotEmpty.wakeAll();
}
