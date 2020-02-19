#include "Gate.hpp"

#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QMap>

//#include "SettingsAndMeta.hpp"

Gate::Gate(QQuickItem* parent) : QQuickPaintedItem(parent)
{
    m_counter = 0;
//	m_isDrawing = true;
}

Gate::Gate(const Gate &oldGate)
{
	m_startPos = std::move(oldGate.m_startPos);
	m_endPos = std::move(oldGate.m_endPos);
	m_name = std::move(oldGate.m_name);
	m_counter = std::move(oldGate.m_counter);
	update();
}

//void Gate::operator=(const Gate &oldGate)
//{
//	m_startPos = std::move(oldGate.m_startPos);
//	m_endPos = std::move(oldGate.m_endPos);
//	m_name = std::move(oldGate.m_name);
//	m_counter = std::move(oldGate.m_counter);
//	update();
//}

QPoint Gate::startPos() const
{
	return m_startPos;
}

void Gate::setStartPos(QPoint pos)
{
	m_startPos = pos;
	update();
}

QPoint Gate::endPos() const
{
	return m_endPos;
}

void Gate::setEndPos(QPoint pos)
{
	m_endPos = pos;
	update();
}

QString Gate::name() const
{
	return m_name;
}

void Gate::setName(QString name)
{
	m_name = name;
	update();
}

int Gate::counter() const
{
	return m_counter;
}

void Gate::setCounter(int counter)
{
	m_counter = counter;
	update();
}



//void Gate::checkVehiclePass(Vehicle *vehicle, int frameIdx, QLineF path)
//{
//	//if (m_historyCounter.count(frameIdx) == 0)
//	//	m_historyCounter[frameIdx] = 0;

//	if (m_gateLine.intersect(path, nullptr) == QLineF::BoundedIntersection) {
//		m_vehicleCounter[frameIdx].push_back(vehicle);
//		m_historyCounter[frameIdx]++;
//		m_stat[vehicle->className().toStdString()][frameIdx]++;
//	}
//}

//void Gate::buildGateHistory()
//{
//	for (int i(1); i <= m_historyCounter.rbegin()->first; ++i)
//		m_historyCounter[i] = m_historyCounter[i - 1] + m_historyCounter[i];
//}

//void Gate::initGate()
//{
//	m_stat.clear();
//	m_vehicleCounter.clear();
//	m_historyCounter.clear();
//	for (int i(0); i < GlobalMeta::VIDEO_LENGTH; ++i)
//		m_historyCounter[i] = 0;
//	for (auto pair : Settings::DETECTOR_CLASSES) {
//		m_stat[pair.second].reserve(GlobalMeta::VIDEO_LENGTH);
//		std::fill(
//			std::begin(m_stat[pair.second]),
//			std::end(m_stat[pair.second]),
//			0
//		);
//	}
//}

void Gate::paint(QPainter* painter)
{
	painter->setRenderHint(QPainter::Antialiasing);

	QPen pen;
	pen.setColor(QColor("#CD5555"));
	pen.setWidth(10);
	pen.setCapStyle(Qt::RoundCap);

	painter->setPen(pen);
	painter->setOpacity(0.6);
	painter->drawLine(m_startPos, m_endPos);

	QFont font = painter->font();
	font.setPointSize(16);
	font.setBold(true);

	pen.setColor(Qt::white);
	painter->setPen(pen);
	painter->setFont(font);
	painter->setOpacity(1.0);

	QPoint labelCenter = m_startPos - (m_startPos - m_endPos) * 0.5f;
	QPoint offset(30, 10);
	QRect counterPos(labelCenter - offset, labelCenter + offset);

	painter->drawText(counterPos, Qt::AlignCenter, QString::number(m_counter));
}

//void Gate::onFrameDisplayed(int frameIdx)
//{
//	try {
//		setCounter(m_historyCounter.at(frameIdx));
//	}
//	catch (const std::out_of_range & ex) {
//		setCounter(9999);
//	}
//}

//QMap<QString, QList<int>> Gate::prepGateStat(int windowSize)
//{
//	int windowNum = std::ceil(GlobalMeta::VIDEO_LENGTH / windowSize);

//	QMap<QString, QList<int>> stat;
//	std::vector<std::string> classNames;

//	for (auto pair : Settings::DETECTOR_CLASSES) {
//		classNames.push_back(pair.second);
//		stat[QString(pair.second.c_str())].reserve(windowNum);
//	}

//	for (auto cName : classNames) {
//		const QString name(cName.c_str());
//		for (int i(0); i < windowNum; ++i) {
//			int sum = 0;
//			for (int j(0); j < windowSize; ++j) {
//				sum += m_stat[name.toStdString()][i * windowSize + j];
//			}
//			stat[name][i] = sum;
//		}
//	}

//	return stat;
//}
