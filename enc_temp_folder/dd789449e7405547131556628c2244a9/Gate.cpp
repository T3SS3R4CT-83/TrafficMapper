#include "Gate.hpp"

#include <QPainter>

#include <TrafficMapper/Types/Vehicle>
#include <TrafficMapper/Media/MediaPlayer>



Gate::Gate(QQuickItem * parent)
	: QQuickPaintedItem(parent), m_counter(0)
{
}



void Gate::setStartPos(QPoint pos)
{
	m_startPos = QPointF(pos.x() / width(), pos.y() / height());
	m_gateLine.setP1(QPointF(
		m_startPos.x() * MediaPlayer::m_videoMeta.WIDTH,
		m_startPos.y() * MediaPlayer::m_videoMeta.HEIGHT
	));
}

//QPoint Gate::getStartPos() const
//{
//	return QPoint(m_startPos.x() * width(), m_startPos.y() * height());
//}

void Gate::setEndPos(QPoint pos)
{
	m_endPos = QPointF(pos.x() / width(), pos.y() / height());
	m_gateLine.setP2(QPointF(
		m_endPos.x() * MediaPlayer::m_videoMeta.WIDTH,
		m_endPos.y() * MediaPlayer::m_videoMeta.HEIGHT
	));

	update();
}

//QPoint Gate::getEndPos() const
//{
//	return QPoint(m_endPos.x() * width(), m_endPos.y() * height());
//}



//void Gate::setName(QString name)
//{
//	m_name = name;
//
//	update();
//}
//QString Gate::getName() const
//{
//	return m_name;
//}

QString Gate::getName() const
{
	return m_name;
}

//int Gate::counter() const
//{
//	return m_counter;
//}
//
//void Gate::setCounter(int counter)
//{
//	m_counter = counter;
//
//	update();
//}
//
//std::unordered_map<VehicleType, std::vector<int>> Gate::getStatistics() const
//{
//	return m_statistics;
//}
//


void Gate::initGate()
{
	m_counter = 0;
	m_sumVehicleNr = 0;

	m_timelineCounter.clear();
	m_timelineCounter.resize(MediaPlayer::m_videoMeta.FRAMECOUNT, 0);
}

uint Gate::checkVehiclePass(Vehicle * vehicle_ptr)
{
	for (const auto & pathSegment : vehicle_ptr->getTrajectory())
	{
		if (m_gateLine.intersect(pathSegment.second, nullptr) == QLineF::BoundedIntersection)
		{
			++m_sumVehicleNr;
			++m_timelineCounter[pathSegment.first];
//			++m_statistics[vehicle_ptr->vehicleClass()][pathSegment.first];
			return pathSegment.first;
		}
	}

	return 0;
}

void Gate::buildTimeline()
{
	for (size_t i(1); i < m_timelineCounter.size(); ++i)
		m_timelineCounter[i] = m_timelineCounter[i - 1] + m_timelineCounter[i];
}



void Gate::paint(QPainter * painter)
{
	painter->setRenderHint(QPainter::Antialiasing);

	const QPointF startPoint(m_startPos.x() * width(), m_startPos.y() * height());
	const QPointF endPoint(m_endPos.x() * width(), m_endPos.y() * height());

	QPen pen;
	pen.setColor(QColor("#CD5555"));
	pen.setWidth(10);
	pen.setCapStyle(Qt::RoundCap);

	painter->setPen(pen);
	painter->setOpacity(0.7);
	painter->drawLine(startPoint, endPoint);

	QFont font = painter->font();
	font.setPointSize(16);
	font.setBold(true);

	pen.setColor(Qt::white);
	painter->setPen(pen);
	painter->setFont(font);
	painter->setOpacity(1.0);

	QPointF labelCenter = startPoint - (startPoint - endPoint) * 0.5f;
	QPointF offset(30, 10);
	QRectF counterPos(labelCenter - offset, labelCenter + offset);

	painter->drawText(counterPos, Qt::AlignCenter, QString::number(m_counter));
}

void Gate::onFrameDisplayed(int frameIdx)
{
	m_counter = m_timelineCounter[frameIdx];

	update();
}
