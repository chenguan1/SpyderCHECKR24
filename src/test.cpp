#include "opencv2/opencv.hpp"
#include "opencv2/mcc.hpp"
#include <iostream>
#include <string>
#include "checkr24.hpp"

using namespace std;
using namespace cv;


// gamma transform
void GammaTransform(const Mat& srcImage, Mat& dstImage, double gamma)
{
	unsigned char lut[256];
	for (int i = 0; i < 256; i++)
	{
		lut[i] = saturate_cast<uchar>(pow((float)i / 255.0, gamma) * 255.0f);
	}
	dstImage = srcImage.clone();
	int channels = srcImage.channels();
	switch (channels)
	{
	case 1:
	{
		MatIterator_<uchar> it = dstImage.begin<uchar>();
		MatIterator_<uchar> end = dstImage.end<uchar>();
		while (it != end)
		{
			*it = lut[(*it)];
			it++;
		}
		break;
	}
	case 3:
	{
		MatIterator_<Vec3b> it = dstImage.begin<Vec3b>();
		MatIterator_<Vec3b> end = dstImage.end<Vec3b>();
		while (it != end)
		{
			(*it)[0] = lut[(*it)[0]];
			(*it)[1] = lut[(*it)[1]];
			(*it)[2] = lut[(*it)[2]];
			it++;
		}
		break;
	}
	default:
		break;
	}
}


// read color from iamge
void read_rgb_from_images(const Mat img, std::vector<int> &rgb_list, const std::vector<int> &corner_pos)
{
	rgb_list.clear();
	for (int r = 0; r < 4; r++)
	{
		int x0 = (corner_pos[4] - corner_pos[0]) / 3 * r + corner_pos[0];
		int y0 = (corner_pos[5] - corner_pos[1]) / 3 * r + corner_pos[1];

		int x1 = (corner_pos[6] - corner_pos[2]) / 3 * r + corner_pos[2];
		int y1 = (corner_pos[7] - corner_pos[3]) / 3 * r + corner_pos[3];
		
		for (int c = 0; c < 6; c++)
		{
			int x = (x1 - x0) / 5 * c + x0;
			int y = (y1 - y0) / 5 * c + y0;

			auto rgb = img.at<Vec3b>(y, x);
			rgb_list.push_back(rgb[2]);
			rgb_list.push_back(rgb[1]);
			rgb_list.push_back(rgb[0]);
		}
	}
}


int main()
{
	// read img
	auto img = imread("../img/test.jpg", 1);

	std::vector<int> corner_pos{
		912,340,
		897,1666,
		1720,328,
		1693,1692
	};

	// 从图像中读取颜色
	std::vector<int> rgb_list;
	read_rgb_from_images(img, rgb_list, corner_pos);
	
	double ccm[12];
	CCMSolve(rgb_list, ccm, 2);

	// use ccm to do correction
	// de gamma
	GammaTransform(img, img, 2);

	// bgr -> xyz
	cv::cvtColor(img, img, COLOR_BGR2XYZ);

	// use ccm
	for (int y = 0; y < img.rows; y++) 
	{
		for (int x = 0; x < img.cols; x++)
		{
			auto &v = img.at<Vec3b>(y, x);
			double v_x = v[0] * ccm[0] + v[1] * ccm[1 * 3 + 0] + v[2] * ccm[2 * 3 + 0] + ccm[3 * 3 + 0] * 255;
			double v_y = v[0] * ccm[1] + v[1] * ccm[1 * 3 + 1] + v[2] * ccm[2 * 3 + 1] + ccm[3 * 3 + 1] * 255;
			double v_z = v[0] * ccm[2] + v[1] * ccm[1 * 3 + 2] + v[2] * ccm[2 * 3 + 2] + ccm[3 * 3 + 2] * 255;
			
			v = Vec3b(MAX(MIN(v_x, 255), 0), MAX(MIN(v_y, 255), 0), MAX(MIN(v_z, 255), 0));
		}
	}

	// xyz -> bgr
	cv::cvtColor(img, img, COLOR_XYZ2BGR);

	// re gamma
	GammaTransform(img, img, 0.5);

	// save 
	cv::imwrite("3.jpg", img);   
	

	return 0;  

}