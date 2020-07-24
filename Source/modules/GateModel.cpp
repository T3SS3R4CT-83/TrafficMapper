#include "GateModel.hpp"

#include <QtConcurrent/QtConcurrent>

#include <TrafficMapper/Types/Gate>



GateModel::GateModel(QObject * parent)
	: QAbstractListModel(parent)
{
}



Q_INVOKABLE int GateModel::rowCount(const QModelIndex & parent) const
{
	return m_gateList.size();
}

Q_INVOKABLE QVariant GateModel::data(const QModelIndex & index, int role) const
{
	if (!index.isValid())
		return QVariant();

	const Gate * item = m_gateList.at(index.row());

	switch (role)
	{
	case StartPosRole:
		return QVariant(item->m_startPos);
	case EndPosRole:
		return QVariant(item->m_endPos);
	case NameRole:
		return QVariant(item->m_name);
	case CounterRole:
		return QVariant(item->m_counter);
	}

	return QVariant();
}

Q_INVOKABLE Qt::ItemFlags GateModel::flags(const QModelIndex & index) const
{
	if (!index.isValid())
		return Qt::NoItemFlags;

	return Qt::ItemIsEditable;
}

QHash<int, QByteArray> GateModel::roleNames() const
{
	QHash<int, QByteArray> names;
	names[StartPosRole] = "startPos";
	names[EndPosRole] = "endPos";
	names[NameRole] = "name";
	names[CounterRole] = "counter";

	return names;
}



void GateModel::insertData(Gate * newGate)
{
	const size_t index = m_gateList.size();

	emit beginInsertRows(QModelIndex(), index, index);
	m_gateList.push_back(newGate);
	emit endInsertRows();
}

void GateModel::removeData(const uint & index)
{
	const int itemCount = m_gateList.size();

	if (index < 0 || index >= itemCount) return;

	emit beginRemoveRows(QModelIndex(), index, index);
	delete m_gateList[index];
	m_gateList.erase(m_gateList.begin() + index);
	emit endRemoveRows();
}

Gate * GateModel::getData(const uint & index)
{
	try
	{
		return m_gateList.at(index);
	}
	catch (std::out_of_range & ex)
	{
		return nullptr;
	}
}



void GateModel::onAnalysisStarted()
{
	for (auto & gate_ptr : m_gateList)
		gate_ptr->initGate();

	m_threadRunning = true;

	QtConcurrent::run([this]()
	{
		qDebug() << "GateModel worker started!";

		for (; m_threadRunning || !m_buffer.isEmpty();)
		{
			m_bufferMutex.lock();
			if (m_buffer.isEmpty())
				m_bufferNotEmpty.wait(&m_bufferMutex);
			m_bufferMutex.unlock();

			if (!m_buffer.isEmpty())
				vehiclePostProcess(m_buffer.dequeue());
		}

		for (auto & gate_ptr : m_gateList)
		{
			gate_ptr->buildTimeline();
		}

		emit analysisEnded();

		qDebug() << "GateModel worker finished!";
	});
}

void GateModel::onAnalysisEnded()
{
	qDebug() << "GateModel worker received stop signal!";

	m_threadRunning = false;
	m_bufferNotEmpty.wakeAll();
}



void GateModel::pipelineInput(Vehicle * vehicle_ptr)
{
	m_bufferMutex.lock();
	m_buffer.enqueue(vehicle_ptr);
	m_bufferNotEmpty.wakeAll();
	m_bufferMutex.unlock();
}

void GateModel::onFrameDisplayed(int frameIdx)
{
	for (auto & gate_ptr : m_gateList)
		gate_ptr->onFrameDisplayed(frameIdx);
}



inline void GateModel::vehiclePostProcess(Vehicle * vehicle_ptr)
{
	for (auto & gate : m_gateList)
	{
		uint frameIdx = gate->checkVehiclePass(vehicle_ptr);

		if (frameIdx > 0)
		{
			emit pipelineOutput(vehicle_ptr, gate, frameIdx);
			return;
		}
	}
	
	emit pipelineOutput(vehicle_ptr, nullptr, 0);
}
