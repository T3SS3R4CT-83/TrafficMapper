#pragma once

#include <vector>

#include <QQuickPaintedItem>
#include <QLineF>

#include <TrafficMapper/Globals>

class Vehicle;

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

	//std::map<int, std::vector<Vehicle*>> m_vehicleCounter;
	//std::map<std::string, std::vector<int>> m_stat;

public:
    Gate(QQuickItem* parent = nullptr);
	Gate(const Gate &oldGate);

//	void operator=(const Gate &oldGate);

	QPoint startPos() const;
	void setStartPos(QPoint pos);
	QPoint endPos() const;
	void setEndPos(QPoint pos);
	QString name() const;
	void setName(QString name);
	int counter() const;
	void setCounter(int counter);

//	void initGate();
	//void checkVehiclePass(Vehicle* _vehicle, const int _frameIdx);
	void checkVehiclePass(Vehicle *vehicle);
	void buildGateStats();
	void onFrameDisplayed(int frameIdx);
//	QMap<QString, QList<int>> prepGateStat(int windowSize);

private:
	// Inherited via QQuickPaintedItem
	virtual void paint(QPainter* painter) override;

signals:
	void valueChanged();
};

Q_DECLARE_METATYPE(Gate)
