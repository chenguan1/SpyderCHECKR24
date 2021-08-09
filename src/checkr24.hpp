#ifndef _CHECKR24_H_
#define _CHECKR24_H_

#include <Eigen/Core>
#include <Eigen/LU>
#include <iostream>
#include <vector>

const int COLOR_COUNT = 24;


using ChartData		= Eigen::Matrix<double, COLOR_COUNT, 3>;
using ChartHmData	= Eigen::Matrix<double, COLOR_COUNT, 4>;
using CCM			= Eigen::Matrix<double, 4, 3>;


// ref color of the card
static const double ref_rgbs[COLOR_COUNT * 3]{
	249,242,238 ,
	202,198,195 ,
	161,157,154 ,
	122,118,116 ,
	80,80,78 ,
	43,41,43 ,
	0,127,159 , // 
	192,75,145 ,
	245,205,0 ,
	186,26,51 ,
	57,146,64 ,
	25,55,135 ,
	222,118,32 ,
	58,88,159 ,
	195,79,95 ,
	83,58,106 ,
	157,188,54 ,
	238,158,25 ,
	98,187,166 ,
	126,125,174 ,
	82,106,60 ,
	87,120,155 ,
	197,145,125 ,
	112,76,60
};


// convert linear rgb to xyz space
static void conv_sRGB2XYZ(const ChartData &rgb, ChartData &xyz) {
	Eigen::Matrix<double, 3, 3> M;
	M << 0.4124564, 0.3575761, 0.1804375,
		0.2126729, 0.7151522, 0.0721750,
		0.0193339, 0.1191920, 0.9503041;
	xyz = (M * rgb.transpose()).transpose();
}

// convert xyz to rgb space
static void conv_XYZ2sRGB(const ChartData &xyz, ChartData &rgb) {
	Eigen::Matrix<double, 3, 3> M;
	M << 0.4124564, 0.3575761, 0.1804375,
		0.2126729, 0.7151522, 0.0721750,
		0.0193339, 0.1191920, 0.9503041;
	rgb = (M.inverse() * xyz.transpose()).transpose();
}


bool CCMSolve(const std::vector<int> &real_colors, double *ccm, double gamma = 2.2)
{
	if (real_colors.size() < 24 * 3) return false;


	ChartData _ref_colors;	// ref colors
	ChartData _real_colors;	// real colors
	CCM _ccm;				// Color Correction Matrix

	for (int i = 0; i < COLOR_COUNT; i++)
	{
		_ref_colors(i, 0) = ref_rgbs[i * 3 + 0] / 255.0;
		_ref_colors(i, 1) = ref_rgbs[i * 3 + 1] / 255.0;
		_ref_colors(i, 2) = ref_rgbs[i * 3 + 2] / 255.0;
	}
	for (int i = 0; i < 24; i++)
	{
		_real_colors(i, 0) = real_colors[i * 3 + 0] / 255.0;
		_real_colors(i, 1) = real_colors[i * 3 + 1] / 255.0;
		_real_colors(i, 2) = real_colors[i * 3 + 2] / 255.0;
	}

	// degamma 0.45 <-> 2.2
	ChartData reference_linear = _ref_colors.array().pow(gamma);
	ChartData source_linear = _real_colors.array().pow(gamma);

	// RGB -> XYZ
	ChartData reference_xyz, source_xyz;
	conv_sRGB2XYZ(reference_linear, reference_xyz);
	conv_sRGB2XYZ(source_linear, source_xyz);

	// source_xyz * ccm == reference_xyz
	// (24, 3 + 1) * (4, 3) = (24 * 3)
	ChartHmData source_xyz_hm;
	source_xyz_hm.col(0) = source_xyz.col(0);
	source_xyz_hm.col(1) = source_xyz.col(1);
	source_xyz_hm.col(2) = source_xyz.col(2);
	source_xyz_hm.col(3) = Eigen::VectorXd::Ones(24);
	auto source_xyz_hm_t = source_xyz_hm.transpose();
	auto pinv = (source_xyz_hm_t * source_xyz_hm).inverse() * source_xyz_hm_t;
	_ccm = pinv * reference_xyz;

	// 4 x 3
	for (int r = 0; r < 4; r++)
	{
		for (int c = 0; c < 3; c++)
		{
			ccm[r * 3 + c] = _ccm(r, c);
		}
	}

	return true;
}




#endif // _CHECKR24_H_


