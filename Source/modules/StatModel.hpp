#pragma once

#include <QAbstractItemModel>

//#include <TrafficMapper/Globals>

class Vehicle;
class Gate;
enum class VehicleType : int;
class GateModel;

class StatModel : public QAbstractItemModel
{
	Q_OBJECT

	Q_PROPERTY(int intervalNr MEMBER m_intervalNr)
	Q_PROPERTY(QStringList axis_X_labels MEMBER m_axisX_labels)
	Q_PROPERTY(int axis_Y_maxval MEMBER m_axisY_maxval)

	int m_asdf;
	GateModel* m_gateModel_ptr;

	int m_intervalNr;
	int m_axisY_maxval;
	QStringList m_axisX_labels;
	
	std::unordered_map<Gate*, std::vector<std::vector<int>>> m_data;
	std::vector<std::vector<int>> m_displayedData;


public:

	StatModel(QObject* parent = nullptr);

	void setGateModel(GateModel* gateModel_ptr);

	// Inherited via QAbstractItemModel
	virtual Q_INVOKABLE QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
	virtual Q_INVOKABLE QModelIndex parent(const QModelIndex& child) const override;
	virtual Q_INVOKABLE int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	virtual Q_INVOKABLE int columnCount(const QModelIndex& parent = QModelIndex()) const override;
	virtual Q_INVOKABLE QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	Q_INVOKABLE QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

	Q_INVOKABLE void updateStat(Gate *gate, int intervalSize = 10);


public slots:
	void initModel();
	void onGatePass(Vehicle* vehicle, Gate* gate, int frameIdx);
};

