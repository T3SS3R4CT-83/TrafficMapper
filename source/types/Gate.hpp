#pragma once

#include <QQuickPaintedItem>
#include <QLineF>



class Vehicle;
//enum class VehicleType : int;


class Gate : public QQuickPaintedItem
{
	Q_OBJECT

	friend class GateModel;

	Q_PROPERTY(QString name MEMBER m_name)
	//Q_PROPERTY(int counter READ counter WRITE setCounter NOTIFY valueChanged)
	
	QPointF m_startPos;
	QPointF m_endPos;
	QString m_name;
	QLineF m_gateLine;

	//std::unordered_map<VehicleType, std::vector<int>> m_statistics;

	uint m_counter;
	std::vector<int> m_timelineCounter;

public:
	Gate(QQuickItem * parent = nullptr);
	//Gate(const Gate & oldGate);

	Q_INVOKABLE void setStartPos(QPoint pos);
	//QPoint getStartPos() const;

	Q_INVOKABLE void setEndPos(QPoint pos);
	//QPoint getEndPos() const;

	//Q_INVOKABLE void setName(QString name);
	QString getName() const;

	//int counter() const;
	//void setCounter(int counter);

	//std::unordered_map<VehicleType, std::vector<int>> getStatistics() const;

	void initGate();
	uint checkVehiclePass(Vehicle * vehicle_ptr);
	void buildTimeline();
	void onFrameDisplayed(int frameIdx);

private:
	// Inherited via QQuickPaintedItem
	virtual void paint(QPainter * painter) override;

//signals:
//	void valueChanged();
};
