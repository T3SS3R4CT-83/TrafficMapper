#pragma once

#include <QAbstractListModel>
#include <QQueue>
#include <QMutex>
#include <QWaitCondition>

class Gate;
class Vehicle;

class GateModel : public QAbstractListModel
{
	Q_OBJECT

	std::vector<Gate *> m_gateList;

	bool m_threadRunning;
	QQueue<Vehicle *> m_buffer;
	QMutex m_bufferMutex;
	QWaitCondition m_bufferNotEmpty;

	friend class StatModel;

public:

	enum {
		StartPosRole = Qt::UserRole + 1,
		EndPosRole,
		NameRole,
		CounterRole,
		SumRole
	};

	GateModel(QObject * parent = nullptr);

	// Inherited via QAbstractListModel
	virtual Q_INVOKABLE int rowCount(const QModelIndex & parent = QModelIndex()) const override;
	virtual Q_INVOKABLE QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const override;
	virtual Q_INVOKABLE Qt::ItemFlags flags(const QModelIndex & index) const override;
	virtual QHash<int, QByteArray> roleNames() const override;

	Q_INVOKABLE void insertData(Gate * newGate);
	Q_INVOKABLE void removeData(const uint & index);
	Q_INVOKABLE Gate * getData(const uint & index);

public slots:

	void onAnalysisStarted();
	void onAnalysisEnded();

	void pipelineInput(Vehicle * vehicle_ptr);

	void onFrameDisplayed(int frameIdx);

private:

	inline void vehiclePostProcess(Vehicle * vehicle_ptr);

signals:

	void pipelineOutput(Vehicle *, Gate *, uint);
	void analysisEnded();
};
