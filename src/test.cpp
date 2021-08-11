#include "opencv2/opencv.hpp"
#include "opencv2/mcc.hpp"
#include <iostream>
#include <string>
#include "checkr24.hpp"
#include "ccmlut.hpp"

using namespace std;
using namespace cv;


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
	auto img = imread("../img/test.jpg", 1);

	std::vector<int> corner_pos{
		912,340,
		897,1666,
		1720,328,
		1693,1692
	};

	// read color from image
	std::vector<int> rgb_list;
	read_rgb_from_images(img, rgb_list, corner_pos);
	
	double ccm[12];
	CCMSolve(rgb_list, ccm, 2);

	CCMLUT clut;
	clut_create(ccm, 0.45, clut);
	
	for (int y = 0; y < img.rows; y++)
	{
		for (int x = 0; x < img.cols; x++)
		{
			int r, g, b;
			auto &vs = img.at<Vec3b>(y, x);
			clut_lookup(clut, vs[2], vs[1], vs[0], r, g, b);
			vs[0] = b;
			vs[1] = g;
			vs[2] = r;
		}
	}

	// save 
	cv::imwrite("5.jpg", img);
	

	return 0;  

}