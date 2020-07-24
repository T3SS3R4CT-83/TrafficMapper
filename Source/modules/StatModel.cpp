#include "StatModel.hpp"

#include <QtConcurrent/QtConcurrent>

#include <TrafficMapper/Modules/VehicleModel>
#include <TrafficMapper/Modules/GateModel>
#include <TrafficMapper/Complementary/FrameProvider>
#include <TrafficMapper/Types/Vehicle>
#include <TrafficMapper/Types/Gate>

#include <cppitertools/starmap.hpp>



StatModel::StatModel(QObject * parent)
	: QAbstractItemModel(parent), m_intervalNr(0)
{
}



Q_INVOKABLE QModelIndex StatModel::index(int row, int column, const QModelIndex & parent) const
{
	return QAbstractItemModel::createIndex(row, column);
}

Q_INVOKABLE QModelIndex StatModel::parent(const QModelIndex & child) const
{
	return Q_INVOKABLE QModelIndex();
}

Q_INVOKABLE int StatModel::rowCount(const QModelIndex & parent) const
{
	return m_displayedData.size();
}

Q_INVOKABLE int StatModel::columnCount(const QModelIndex & parent) const
{
	return m_displayedData[0].size();
}

Q_INVOKABLE QVariant StatModel::data(const QModelIndex & index, int role) const
{
	return Q_INVOKABLE QVariant(m_displayedData[index.row()][index.column()]);
}

QVariant StatModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	QStringList legend = {"Cars", "Buses", "Trucks", "Motorcycles", "Bicycles"};

	if (section < 5)
		return QVariant(legend[section]);
	else
		return QVariant(QStringLiteral("unknown"));
}



void StatModel::updateStat(Gate * gate, const uint & intervalSize)
{
	m_intervalNr = std::ceil(FrameProvider::m_videoMeta.LENGTH / intervalSize * 0.001f);

	const uint intSize = FrameProvider::m_videoMeta.FPS * intervalSize;

	m_displayedData.clear();
	m_displayedData.resize(m_intervalNr);
	for (auto & dataRow : m_displayedData)
		dataRow.resize(5, 0);

	const auto & gateData = m_data.at(gate);
	for (size_t frameIdx(0); frameIdx < FrameProvider::m_videoMeta.FRAMECOUNT; ++frameIdx)
		for (uint vClass(0); vClass < 5; ++vClass)
			m_displayedData[frameIdx / intSize][vClass] += gateData[vClass][frameIdx];

	emit intervalChanged();

	m_graphTitle = gate->getName();
	emit titleChanged();

	m_axisY_maxval = 0;
	for (int i(0); i < m_intervalNr; ++i)
	{
		int val = 0;

		for (int j(0); j < 5; ++j)
			val += m_displayedData[i][j];

		if (val > m_axisY_maxval)
			m_axisY_maxval = val;
	}
	emit axisYchanged();

	m_axisX_labels.clear();
	for (int i(1); i <= m_intervalNr; ++i)
	{
		const int minutes = i * intervalSize / 60;
		const int seconds = i * intervalSize % 60;

		m_axisX_labels << QString("%1:%2").arg(minutes).arg(seconds);
	}
	emit axisXchanged();

	dataChanged(createIndex(0, 0), createIndex(m_intervalNr - 1, 4));
}



void StatModel::onAnalysisStarted()
{
	m_data.clear();
	m_threadRunning = true;

	QtConcurrent::run([this]()
	{
		qDebug() << "StatModel worker started!";

		for (; m_threadRunning || !m_buffer.isEmpty();)
		{
			m_bufferMutex.lock();
			if (m_buffer.isEmpty())
				m_bufferNotEmpty.wait(&m_bufferMutex);
			m_bufferMutex.unlock();

			if (!m_buffer.isEmpty())
				statPostProcess(m_buffer.dequeue());
		}

		qDebug() << "StatModel worker finished!";
	});
}

void StatModel::onAnalysisEnded()
{
	qDebug() << "StatModel worker received stop signal!";

	m_threadRunning = false;
	m_bufferNotEmpty.wakeAll();
}

void StatModel::pipelineInput(Vehicle * vehicle_ptr, Gate * gate_ptr, uint frameIdx)
{
	m_bufferMutex.lock();
	m_buffer.enqueue(std::make_tuple(vehicle_ptr, gate_ptr, frameIdx));
	m_bufferNotEmpty.wakeAll();
	m_bufferMutex.unlock();
}



inline void StatModel::statPostProcess(std::tuple<Vehicle *, Gate *, uint> data)
{
	Vehicle * vehicle_ptr = std::get<0>(data);
	Gate * gate_ptr = std::get<1>(data);
	uint frameIdx = std::get<2>(data);

	if (gate_ptr == nullptr)
		return;

	const int vehicleTypeId = static_cast<int>(vehicle_ptr->m_vehicleClass);
	if (vehicleTypeId == -1)
		return;

	if (m_data.find(gate_ptr) == std::end(m_data))
	{
		m_data[gate_ptr].resize(5);
		for (int i(0); i < 5; ++i)
		{
			m_data[gate_ptr][i].resize(FrameProvider::m_videoMeta.FRAMECOUNT, 0);
		}
	}

	++m_data[gate_ptr][vehicleTypeId][frameIdx];
}
