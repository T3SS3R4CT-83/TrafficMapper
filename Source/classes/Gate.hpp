#pragma once

#include <QQuickPaintedItem>
#include <QLineF>

#include <TrafficMapper/Classes/Vehicle>


enum class VehicleType : int;


class Gate : public QQuickPaintedItem
{
	Q_OBJECT

	Q_PROPERTY(QPoint startPos READ startPos WRITE setStartPos NOTIFY valueChanged)
	Q_PROPERTY(QPoint endPos READ endPos WRITE setEndPos NOTIFY valueChanged)
	Q_PROPERTY(QString name READ name WRITE setName NOTIFY valueChanged)
	Q_PROPERTY(int counter READ counter WRITE setCounter NOTIFY valueChanged)
	
	QPoint m_startPos;
	QPoint m_endPos;
	QLineF m_gateLine;
	int m_counter;
	QString m_name;

	std::unordered_map<VehicleType, std::vector<int>> m_statistics;
	std::vector<int> m_counterTimeline;

public:
	Gate(QQuickItem * parent = nullptr);
	Gate(const Gate & oldGate);

	QPoint startPos() const;
	void setStartPos(QPoint pos);
	QPoint endPos() const;
	void setEndPos(QPoint pos);
	QString name() const;
	void setName(QString name);
	int counter() const;
	void setCounter(int counter);

	std::unordered_map<VehicleType, std::vector<int>> getStatistics() const;

	int checkVehiclePass(Vehicle * vehicle);
	void buildGateTimeline();
	void onFrameDisplayed(int frameIdx);

private:
	// Inherited via QQuickPaintedItem
	virtual void paint(QPainter * painter) override;

signals:
	void valueChanged();
};

//Q_DECLARE_METATYPE(Gate)
