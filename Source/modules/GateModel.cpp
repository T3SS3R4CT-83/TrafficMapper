#include "GateModel.hpp"

#include <TrafficMapper/Classes/Gate>
#include <TrafficMapper/Classes/Vehicle>

GateModel::GateModel(QObject *parent) : QAbstractListModel(parent) { }

int GateModel::rowCount(const QModelIndex &parent) const
{
	if (parent.isValid())
		return 0;

	return m_gateList.size();
}

QVariant GateModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	const Gate *item = m_gateList.at(index.row());

	switch (role)
	{
	case StartPosRole:
		return QVariant(item->startPos());
	case EndPosRole:
		return QVariant(item->endPos());
	case NameRole:
		return QVariant(item->name());
	case CounterRole:
		return QVariant(item->counter());
	}

	return QVariant();
}

bool GateModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	const int rowIdx = index.row();
	
	if (rowIdx < 0 || rowIdx >= m_gateList.size()) return false;

	Gate *item = m_gateList.at(rowIdx);

	switch (role)
	{
	case StartPosRole:
		{
			const QPoint newValue = value.toPoint();
			if (item->startPos() == newValue) return false;
			item->setStartPos(newValue);
			break;
		}
	case EndPosRole:
		{
			const QPoint newValue = value.toPoint();
			if (item->endPos() == newValue) return false;
			item->setEndPos(newValue);
			break;
		}
	case NameRole:
		{
			const QString newValue = value.toString();
			if (item->name() == newValue) return false;
			item->setName(newValue);
			break;
		}
	case CounterRole:
		{
			const int newValue = value.toInt();
			if (item->counter() == newValue) return false;
			item->setCounter(newValue);
			break;
		}
	}
	   
	emit dataChanged(index, index, QVector<int>() << role);

	return true;
}

Qt::ItemFlags GateModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return Qt::NoItemFlags;

	return Qt::ItemIsEditable;
}

QHash<int, QByteArray> GateModel::roleNames() const
{
	QHash<int, QByteArray> names;
	names[StartPosRole] = "startPos";
	names[EndPosRole]	= "endPos";
	names[NameRole]		= "name";
	names[CounterRole]	= "counter";

	return names;
}

void GateModel::insertData(Gate *newGate)
{
	const int index = m_gateList.size();

	emit beginInsertRows(QModelIndex(), index, index);
	m_gateList.push_back(newGate);
	emit endInsertRows();
}

void GateModel::removeData(int index)
{
	const int itemCount = m_gateList.size();

	if (index < 0 || index >= itemCount) return;

	emit beginRemoveRows(QModelIndex(), index, index);
	delete m_gateList[index];
	m_gateList.erase(m_gateList.begin() + index);
	emit endRemoveRows();
}

void GateModel::clearData()
{
	const int itemCount = m_gateList.size();

	if (itemCount) {
		emit beginRemoveRows(QModelIndex(), 0, itemCount - 1);
		for (int i(0); i < m_gateList.size(); ++i) {
			delete m_gateList[i];
		}
		m_gateList.clear();
		emit endRemoveRows();
	}
}

//QMap<QString, QList<int>> GateModel::getGateStat(int gateIdx, int windowSize)
//{
//	return m_gateList[gateIdx]->prepGateStat(windowSize);
//}

void GateModel::onFrameDisplayed(int frameIdx)
{
	for (auto gate : m_gateList)
		gate->onFrameDisplayed(frameIdx);
}

std::vector<Gate*> GateModel::getGates() const
{
	return m_gateList;
}

//void GateModel::onVehiclePositionUpdated(Vehicle * _vehicle, int _frameIdx)
//{
//	for (auto gate : m_gateList)
//		gate->checkVehiclePass(_vehicle, _frameIdx);
//}

void GateModel::checkVehicle(Vehicle *vehicle)
{
	for (auto gate : m_gateList)
	{
		gate->checkVehiclePass(vehicle);
	}

	//std::map<int, QPoint> vehiclePath = vehicle->getVehiclePath();

	//for (auto position = std::next(vehiclePath.begin()); position != vehiclePath.end(); ++position) {
	//	QLine vehiclePath(std::prev(position)->second, position->second);
	//	for (auto gate : m_gateList)
	//		gate->checkVehiclePass(vehicle, position->first, vehiclePath);
	//}
}

void GateModel::buildGateStats()
{
	for (auto gate : m_gateList)
		gate->buildGateStats();
}

//void GateModel::onAnalisisStart()
//{
//	for (auto &gate : m_gateList) {
//		gate->initGate();
//	}
//}
