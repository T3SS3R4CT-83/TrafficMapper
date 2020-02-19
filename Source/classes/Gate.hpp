#pragma once

#include <QQuickPaintedItem>
#include <QLineF>

//#include "Vehicle.h"

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

//	std::map<int, std::vector<Vehicle*>> m_vehicleCounter;
//	std::map<int, int> m_historyCounter;
//	std::map<std::string, std::vector<int>> m_stat;

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
//	void checkVehiclePass(Vehicle *vehicle, int frameIdx, QLineF path);
//	void buildGateHistory();
//	void onFrameDisplayed(int frameIdx);
//	QMap<QString, QList<int>> prepGateStat(int windowSize);

private:
	// Inherited via QQuickPaintedItem
	virtual void paint(QPainter* painter) override;

signals:
	void valueChanged();
};

Q_DECLARE_METATYPE(Gate)
