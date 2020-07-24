#pragma once

class GateModel;
class VehicleModel;

#include <QAbstractItemModel>
#include <QQueue>
#include <QMutex>
#include <QWaitCondition>

class Gate;
class Vehicle;

class StatModel : public QAbstractItemModel
{
	Q_OBJECT

	Q_PROPERTY(QString graphTitle MEMBER m_graphTitle NOTIFY titleChanged)
	Q_PROPERTY(uint intervalNr MEMBER m_intervalNr NOTIFY intervalChanged)
	Q_PROPERTY(QStringList axis_X_labels MEMBER m_axisX_labels NOTIFY axisXchanged)
	Q_PROPERTY(uint axis_Y_maxval MEMBER m_axisY_maxval NOTIFY axisYchanged)

	bool m_threadRunning;
	QQueue<std::tuple<Vehicle *, Gate *, uint>> m_buffer;
	QMutex m_bufferMutex;
	QWaitCondition m_bufferNotEmpty;

	QString m_graphTitle;
	uint m_intervalNr;
	uint m_axisY_maxval;
	QStringList m_axisX_labels;

	std::unordered_map<Gate *, std::vector<std::vector<int>>> m_data;
	std::vector<std::vector<int>> m_displayedData;

public:

	StatModel(QObject * parent = nullptr);

	// Inherited via QAbstractItemModel
	virtual Q_INVOKABLE QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const override;
	virtual Q_INVOKABLE QModelIndex parent(const QModelIndex & child) const override;
	virtual Q_INVOKABLE int rowCount(const QModelIndex & parent = QModelIndex()) const override;
	virtual Q_INVOKABLE int columnCount(const QModelIndex & parent = QModelIndex()) const override;
	virtual Q_INVOKABLE QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const override;
	Q_INVOKABLE QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

	Q_INVOKABLE void updateStat(Gate * gate, const uint & intervalSize);

public slots:

	void onAnalysisStarted();
	void onAnalysisEnded();

	void pipelineInput(Vehicle * vehicle_ptr, Gate * gate_ptr, uint frameIdx);

private:

	inline void statPostProcess(std::tuple<Vehicle *, Gate *, uint> data);

signals:

	void titleChanged();
	void intervalChanged();
	void axisXchanged();
	void axisYchanged();
};
