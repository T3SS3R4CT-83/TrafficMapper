#pragma once

#include <QAbstractVideoFilter>

class VehicleModel;
class GateModel;

class VideoOverlay : public QAbstractVideoFilter
{
	Q_OBJECT

	VehicleModel * m_vehicleModel_ptr;
	GateModel * m_gateModel_ptr;

public:

	VideoOverlay(QObject * parent = nullptr);

	// Inherited via QAbstractVideoFilter
	virtual QVideoFilterRunnable * createFilterRunnable() override;

	void setVehicleModel(VehicleModel * vehicleModel_ptr);
	void setGateModel(GateModel * gateModel_ptr);
};

