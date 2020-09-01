#pragma once

#include <QAbstractVideoFilter>

class VideoFilter;
class VehicleModel;
class GateModel;

class VideoOverlay : public QAbstractVideoFilter
{
	Q_OBJECT

	VideoFilter * m_filter_ptr;

	VehicleModel * m_vehicleModel_ptr;
	GateModel * m_gateModel_ptr;

public:

	VideoOverlay(QObject * parent = nullptr);
	VideoOverlay(const VideoOverlay & other);

	// Inherited via QAbstractVideoFilter
	virtual QVideoFilterRunnable * createFilterRunnable() override;

	void setVehicleModel(VehicleModel * vehicleModel_ptr);
	void setGateModel(GateModel * gateModel_ptr);

signals:

	void setPlaybackOptions(bool, bool, bool, bool);
};
