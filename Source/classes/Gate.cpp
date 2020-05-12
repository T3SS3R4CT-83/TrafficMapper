#include "Gate.hpp"

#include <QPainter>

#include <TrafficMapper/Globals>
#include <TrafficMapper/Types>


Gate::Gate(QQuickItem * parent) : QQuickPaintedItem(parent)
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
}

Gate::Gate(const Gate & oldGate)
{
	m_startPos = std::move(oldGate.m_startPos);
	m_endPos = std::move(oldGate.m_endPos);
	m_name = std::move(oldGate.m_name);
	m_counter = std::move(oldGate.m_counter);

	update();
}

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



int Gate::checkVehiclePass(Vehicle * vehicle)
{
	for (auto pathSegment : vehicle->getVehiclePath())
	{
		if (m_gateLine.intersect(pathSegment.second, nullptr) == QLineF::BoundedIntersection)
		{
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

void Gate::paint(QPainter * painter)
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
}
