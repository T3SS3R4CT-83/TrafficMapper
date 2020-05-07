#include "Gate.hpp"

#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QMap>


Gate::Gate(QQuickItem* parent) : QQuickPaintedItem(parent)
{
	const int frameNr = GlobalMeta::getInstance()->VIDEO_FRAMECOUNT();

	m_statistics[VehicleType::BICYCLE].resize(frameNr, 0);
	m_statistics[VehicleType::BUS].resize(frameNr, 0);
	m_statistics[VehicleType::CAR].resize(frameNr, 0);
	m_statistics[VehicleType::MOTORCYCLE].resize(frameNr, 0);
	m_statistics[VehicleType::TRUCK].resize(frameNr, 0);
	m_statistics[VehicleType::undefined].resize(frameNr, 0);

	m_counterTimeline.resize(frameNr, 0);
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
	const float scale_X = GlobalMeta::getInstance()->VIDEO_WIDTH() / 782.f;
	const float scale_Y = GlobalMeta::getInstance()->VIDEO_HEIGHT() / 480.f;

	m_startPos = pos;
	m_gateLine.setP1(QPointF(pos.x() * scale_X, pos.y() * scale_Y));
	update();
}

QPoint Gate::endPos() const
{
	return m_endPos;
}

void Gate::setEndPos(QPoint pos)
{
	const float scale_X = GlobalMeta::getInstance()->VIDEO_WIDTH() / 782.f;
	const float scale_Y = GlobalMeta::getInstance()->VIDEO_HEIGHT() / 480.f;

	m_endPos = pos;
	m_gateLine.setP2(QPointF(pos.x() * scale_X, pos.y() * scale_Y));
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

std::unordered_map<VehicleType, std::vector<int>> Gate::getStatistics() const
{
	return m_statistics;
}



//void Gate::checkVehiclePass(Vehicle* _vehicle, const int _frameIdx)
//{
//	if (m_gateLine.intersect(_vehicle->getPathSegment(_frameIdx), nullptr) == QLineF::BoundedIntersection)
//	{
//		++m_counterTimeline[_frameIdx];
//		++m_statistics[_vehicle->vehicleClass()][_frameIdx];
//	}
//}
int Gate::checkVehiclePass(Vehicle *vehicle)
{
	for (auto pathSegment : vehicle->getVehiclePath())
	{
		if (m_gateLine.intersect(pathSegment.second, nullptr) == QLineF::BoundedIntersection) {
			++m_counterTimeline[pathSegment.first];
			++m_statistics[vehicle->vehicleClass()][pathSegment.first];
			return pathSegment.first;
		}
	}

	return -1;
}

void Gate::buildGateTimeline()
{
	for (size_t i(1); i < m_counterTimeline.size(); ++i)
		m_counterTimeline[i] = m_counterTimeline[i - 1] + m_counterTimeline[i];
}

//void Gate::initGate()
//{
//	m_statistics.clear();
//	m_vehicleCounter.clear();
//	m_historyCounter.clear();
//	for (int i(0); i < GlobalMeta::VIDEO_LENGTH; ++i)
//		m_historyCounter[i] = 0;
//	for (auto pair : Settings::DETECTOR_CLASSES) {
//		m_statistics[pair.second].reserve(GlobalMeta::VIDEO_LENGTH);
//		std::fill(
//			std::begin(m_statistics[pair.second]),
//			std::end(m_statistics[pair.second]),
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

	QPointF labelCenter = m_startPos - (m_startPos - m_endPos) * 0.5f;
	QPointF offset(30, 10);
	QRectF counterPos(labelCenter - offset, labelCenter + offset);

	painter->drawText(counterPos, Qt::AlignCenter, QString::number(m_counter));
}

void Gate::onFrameDisplayed(int frameIdx)
{
	m_counter = m_counterTimeline[frameIdx];
	update();
	//try {
	//	setCounter(m_historyCounter.at(frameIdx));
	//}
	//catch (const std::out_of_range & ex) {
	//	setCounter(9999);
	//}
}

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
//				sum += m_statistics[name.toStdString()][i * windowSize + j];
//			}
//			stat[name][i] = sum;
//		}
//	}

//	return stat;
//}
