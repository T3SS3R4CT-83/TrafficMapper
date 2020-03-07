#pragma once

#include <memory>

#include <QAbstractListModel>

class Gate;
class Vehicle;

class GateModel : public QAbstractListModel
{
	Q_OBJECT

	std::vector<Gate*> m_gateList;

public:
    GateModel(QObject *parent = nullptr);

	enum {
		StartPosRole = Qt::UserRole + 1,
		EndPosRole,
		NameRole,
		CounterRole
	};

	// Inherited via QAbstractListModel
	virtual Q_INVOKABLE int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	virtual Q_INVOKABLE QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
	Q_INVOKABLE bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

	Qt::ItemFlags flags(const QModelIndex &index) const override;
	virtual QHash<int, QByteArray> roleNames() const override;

	Q_INVOKABLE void insertData(Gate *newGate);
	Q_INVOKABLE void removeData(int index);
	Q_INVOKABLE void clearData();
//	Q_INVOKABLE QMap<QString, QList<int>> getGateStat(int gateIdx, int windowSize);


	std::vector<Gate*> getGates() const;


//	void onVehiclePositionUpdated(Vehicle * _vehicle, int _frameIdx);
	void checkVehicle(Vehicle *vehicle);
	void buildGateStats();

public slots:
//	void onAnalisisStart();
	void onFrameDisplayed(int frameIdx);
};
