#include "StatModel.hpp"

#include <TrafficMapper/Classes/Gate>
#include <TrafficMapper/Classes/Vehicle>
#include <TrafficMapper/Modules/GateModel>
#include <TrafficMapper/Globals>

#include <QDebug>



StatModel::StatModel(QObject* parent) : QAbstractItemModel(parent)
{
	m_asdf = 2;

	m_displayedData = {
		{75, 54, 19, 68, 48},
		{96, 65, 48, 68, 18},
		{68, 81, 96, 35, 68},
		{93, 86, 18, 33, 86},
		{96, 84, 18, 55, 45},
		{92, 44, 62, 51, 91},
		{37, 26, 56, 18, 75}
	};

	m_axisX_labels << "A" << "S" << "D" << "F" << "G" << "H" << "J";
}



void StatModel::setGateModel(GateModel* gateModel_ptr)
{
	m_gateModel_ptr = gateModel_ptr;
}



Q_INVOKABLE QModelIndex StatModel::index(int row, int column, const QModelIndex& parent) const
{
	return QAbstractItemModel::createIndex(row, column);
	//return QModelIndex();
}

Q_INVOKABLE QModelIndex StatModel::parent(const QModelIndex& child) const
{
	return Q_INVOKABLE QModelIndex();
}

Q_INVOKABLE int StatModel::rowCount(const QModelIndex& parent) const
{
	return m_displayedData.size();
}

Q_INVOKABLE int StatModel::columnCount(const QModelIndex& parent) const
{
	return m_displayedData[0].size();
}

Q_INVOKABLE QVariant StatModel::data(const QModelIndex& index, int role) const
{
	return Q_INVOKABLE QVariant(m_displayedData[index.row()][index.column()]);
}

QVariant StatModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	QStringList legend = { "Cars", "Buses", "Trucks", "Motorcycles", "Bicycles" };

	if (section < 5)
		return QVariant(legend[section]);
	else
		return QVariant(section);
}



void StatModel::updateStat(Gate* gate, int intervalSize)
{
	m_intervalNr = std::ceil(GlobalMeta::getInstance()->VIDEO_LENGTH() / intervalSize * 0.001f);
	const int intSize = GlobalMeta::getInstance()->VIDEO_FPS() * intervalSize;

	m_displayedData.clear();
	m_displayedData.resize(m_intervalNr);
	for (int i(0); i < m_intervalNr; ++i)
		m_displayedData[i].resize(5, 0);

	//auto gateData = m_data.begin()->second;
	auto gateData = m_data.at(gate);
	for (int frameIdx(0); frameIdx < GlobalMeta::getInstance()->VIDEO_FRAMECOUNT(); ++frameIdx)
		for (int vClass(0); vClass < 5; ++vClass)
			m_displayedData[frameIdx / intSize][vClass] += gateData[vClass][frameIdx];

	m_axisY_maxval = 0;
	for (int i(0); i < m_intervalNr; ++i)
	{
		int val = 0;

		for (int j(0); j < 5; ++j)
			val += m_displayedData[i][j];

		if (val > m_axisY_maxval)
			m_axisY_maxval = val;
	}

	m_axisX_labels.clear();
	for (int i(1); i <= m_intervalNr; ++i)
	{
		const int minutes = i * intervalSize / 60;
		const int seconds = i * intervalSize % 60;

		m_axisX_labels << QString("%1:%2").arg(minutes).arg(seconds);
	}

	dataChanged(createIndex(0, 0), createIndex(m_intervalNr - 1, 4));
}



void StatModel::initModel()
{
	auto gateList = m_gateModel_ptr->getGates();
	int frameNr = GlobalMeta::getInstance()->VIDEO_FRAMECOUNT();

	m_data.clear();

	for (auto gate : gateList)
	{
		m_data[gate].resize(5);
		for (int i(0); i < 5; ++i)
		{
			m_data[gate][i].resize(frameNr, 0);
		}
	}
}

void StatModel::onGatePass(Vehicle* vehicle, Gate* gate, int frameIdx)
{
	++(m_data[gate][(int)vehicle->vehicleClass()][frameIdx]);
}
