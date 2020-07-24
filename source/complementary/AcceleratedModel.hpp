#pragma once

#include <opencv2/tracking/kalman_filters.hpp>

class AcceleratedModel : public cv::tracking::UkfSystemModel
{
	float m_deltaTime;

public:

	AcceleratedModel();

	void stateConversionFunction(const cv::Mat & x_k, const cv::Mat & u_k, const cv::Mat & v_k, cv::Mat & x_kplus1);

	void measurementFunction(const cv::Mat & x_k, const cv::Mat & n_k, cv::Mat & z_k);
};
