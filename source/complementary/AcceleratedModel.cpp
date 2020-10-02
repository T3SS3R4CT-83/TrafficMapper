#include "AcceleratedModel.hpp"

AcceleratedModel::AcceleratedModel()
	: cv::tracking::UkfSystemModel(), m_deltaTime(0.1f)
{
}

/*
* x_k : previous state vector
* u_k : control vector
* v_k : noise vector
* x_kplus1 : next state vector. 
*/
void AcceleratedModel::stateConversionFunction(const cv::Mat & x_k, const cv::Mat & u_k, const cv::Mat & v_k, cv::Mat & x_kplus1)
{
	const float x0 = x_k.at<float>(0, 0);
	const float y0 = x_k.at<float>(1, 0);
	const float vx0 = x_k.at<float>(2, 0);
	const float vy0 = x_k.at<float>(3, 0);
	const float ax0 = x_k.at<float>(4, 0);
	const float ay0 = x_k.at<float>(5, 0);

	x_kplus1.at<float>(0, 0) = x0 + vx0 * m_deltaTime + ax0 * pow(m_deltaTime, 2) / 2;
	x_kplus1.at<float>(1, 0) = y0 + vy0 * m_deltaTime + ay0 * pow(m_deltaTime, 2) / 2;
	x_kplus1.at<float>(2, 0) = vx0 + ax0 * m_deltaTime;
	x_kplus1.at<float>(3, 0) = vy0 + ay0 * m_deltaTime;
	x_kplus1.at<float>(4, 0) = ax0;
	x_kplus1.at<float>(5, 0) = ay0;

	if (v_k.size() == u_k.size())
	{
		x_kplus1 += v_k + u_k;
	}
	else
	{
		x_kplus1 += v_k;
	}
}

/*
* x_k : state vector
* n_k : noise vector
* z_k : measurement vector
*/
void AcceleratedModel::measurementFunction(const cv::Mat & x_k, const cv::Mat & n_k, cv::Mat & z_k)
{
	float x0 = x_k.at<float>(0, 0);
	float y0 = x_k.at<float>(1, 0);
	float vx0 = x_k.at<float>(2, 0);
	float vy0 = x_k.at<float>(3, 0);
	float ax0 = x_k.at<float>(4, 0);
	float ay0 = x_k.at<float>(5, 0);

	float new_X = x0 + vx0 * m_deltaTime + ax0 * pow(m_deltaTime, 2) / 2 + n_k.at<float>(0, 0);
	float new_Y = y0 + vy0 * m_deltaTime + ay0 * pow(m_deltaTime, 2) / 2 + n_k.at<float>(1, 0);

	z_k.at<float>(0, 0) = new_X;
	z_k.at<float>(1, 0) = new_Y;
}
