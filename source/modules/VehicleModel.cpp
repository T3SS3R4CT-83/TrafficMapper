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

inline void VehicleModel::vehiclePostProcess(Vehicle * vehicle_ptr)
{
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
		m_isCameraCalibrated = true;
		m_Homography = calibrationModule->m_homography_i2p.clone();
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
