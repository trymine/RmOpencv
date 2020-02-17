//*
#include<opencv.hpp>
#include<iostream>
#include"Armor_Dection.h"


cv::Mat channel_swap(cv::Mat img) {
	// get height and width
	int width = img.cols;
	int height = img.rows;

	// prepare output
	cv::Mat out = cv::Mat::zeros(height, width, CV_8UC3);

	// each y, x
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			// R -> B
			out.at<cv::Vec3b>(y, x)[0] = img.at<cv::Vec3b>(y, x)[2];
			// B -> R
			out.at<cv::Vec3b>(y, x)[2] = img.at<cv::Vec3b>(y, x)[0];
			// G -> G
			out.at<cv::Vec3b>(y, x)[1] = img.at<cv::Vec3b>(y, x)[1];
		}
	}

	return out;
}

int main(int argc, char** argv)
{
	//cv::Mat img = cv::imread("4.tif");
	//resize(img, img, Size(img.cols / 4, img.rows / 4));//尺寸缩减（后期可在相机上直接调节采样大小及图像分辨率）
	//cv::Mat out = channel_swap(img);//转化为BGR通道
	//cv::Mat dst;
	//resize(img, dst, Size(), 0.25, 0.25);//我长宽都变为原来的0.6倍
	//resize(img, img, Size(img.cols / 4, img.rows / 4));

	//cv::VideoCapture cap("D:/VS2017_projects/opencv/demo1/test_data/Video/car1_25fps.mp4");
	cv::VideoCapture cap(0);
	cv::Mat img;
	Point2f Armor_center;
	while (1)
	{
		cap >> img;

		Armor_center = Armor_Dection(img, RED);
		std::cout << "The Best Armor Center:" << Armor_center << std::endl;
		cv::imshow("视频", img);
		waitKey(1);
		//getchar();
	}

	return 0;
}



/*/

//quenction 01
//
/*
#include<opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>

using namespace cv;
using namespace std;
// Channel swap
cv::Mat channel_swap(cv::Mat img){
  // get height and width
  int width = img.cols;
  int height = img.rows;

  // prepare output
  cv::Mat out = cv::Mat::zeros(height, width, CV_8UC3);

  // each y, x
  for (int y = 0; y < height; y++){
	for (int x = 0; x < width; x++){
	  // R -> B
	  out.at<cv::Vec3b>(y, x)[0] = img.at<cv::Vec3b>(y, x)[2];
	  // B -> R
	  out.at<cv::Vec3b>(y, x)[2] = img.at<cv::Vec3b>(y, x)[0];
	  // G -> G
	  out.at<cv::Vec3b>(y, x)[1] = img.at<cv::Vec3b>(y, x)[1];
	}
  }

  return out;
}


int main(int argc, const char* argv[]){
  // read image
  cv::Mat img = cv::imread("school0.jpg", cv::IMREAD_COLOR);

  cv::Mat dst;
  resize(img, dst, Size(), 0.6, 0.6);//我长宽都变为原来的0.6倍
  
  // channel swap
  cv::Mat out = channel_swap(dst);

  //cv::imwrite("out.jpg", out);
  cv::imshow("sample", out);
  cv::waitKey(0);
  cv::destroyAllWindows();

  return 0;
}
*/



/*
//quenction 05
//RGB至HSV
#include<opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <math.h>

using namespace cv;
//using namespace std;
// BGR -> HSV
cv::Mat BGR2HSV(cv::Mat img) {
	// get height and width
	int width = img.cols;
	int height = img.rows;

	float r, g, b;
	float h, s, v;
	float _max, _min;

	// prepare output
	cv::Mat hsv = cv::Mat::zeros(height, width, CV_32FC3);

	// each y, x
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			// BGR -> HSV
			r = (float)img.at<cv::Vec3b>(y, x)[2] / 255;
			g = (float)img.at<cv::Vec3b>(y, x)[1] / 255;
			b = (float)img.at<cv::Vec3b>(y, x)[0] / 255;

			_max = fmax(r, fmax(g, b));
			_min = fmin(r, fmin(g, b));

			// get Hue
			if (_max == _min) {
				h = 0;
			}
			else if (_min == b) {
				h = 60 * (g - r) / (_max - _min) + 60;
			}
			else if (_min == r) {
				h = 60 * (b - g) / (_max - _min) + 180;
			}
			else if (_min == g) {
				h = 60 * (r - b) / (_max - _min) + 300;
			}

			// get Saturation
			s = _max - _min;

			// get Value
			v = _max;

			hsv.at<cv::Vec3f>(y, x)[0] = h;
			hsv.at<cv::Vec3f>(y, x)[1] = s;
			hsv.at<cv::Vec3f>(y, x)[2] = v;
		}
	}
	return hsv;
}


// HSV -> BGR
cv::Mat HSV2BGR(cv::Mat hsv) {
	// get height and width
	int width = hsv.cols;
	int height = hsv.rows;

	float h, s, v;
	double c, _h, _x;
	double r, g, b;

	// prepare output
	cv::Mat out = cv::Mat::zeros(height, width, CV_8UC3);

	// each y, x
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {

			h = hsv.at<cv::Vec3f>(y, x)[0];
			s = hsv.at<cv::Vec3f>(y, x)[1];
			v = hsv.at<cv::Vec3f>(y, x)[2];

			c = s;
			_h = h / 60;
			_x = c * (1 - abs(fmod(_h, 2) - 1));

			r = g = b = v - c;

			if (_h < 1) {
				r += c;
				g += _x;
			}
			else if (_h < 2) {
				r += _x;
				g += c;
			}
			else if (_h < 3) {
				g += c;
				b += _x;
			}
			else if (_h < 4) {
				g += _x;
				b += c;
			}
			else if (_h < 5) {
				r += _x;
				b += c;
			}
			else if (_h < 6) {
				r += c;
				b += _x;
			}

			out.at<cv::Vec3b>(y, x)[0] = (uchar)(b * 255);
			out.at<cv::Vec3b>(y, x)[1] = (uchar)(g * 255);
			out.at<cv::Vec3b>(y, x)[2] = (uchar)(r * 255);
		}
	}

	return out;
}

// inverse Hue
cv::Mat inverse_hue(cv::Mat hsv) {
	int height = hsv.rows;
	int width = hsv.cols;

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			hsv.at<cv::Vec3f>(y, x)[0] = fmod(hsv.at<cv::Vec3f>(y, x)[0] + 180, 360);
		}
	}

	return hsv;
}


int main(int argc, const char* argv[]) {
	// read image
	//cv::Mat img = cv::imread("school0.jpg", cv::IMREAD_COLOR);

	cv::Mat dst = cv::imread("school0.jpg", cv::IMREAD_COLOR);
	cv::Mat img;
	resize(dst, img, Size(), 0.6, 0.6);//我长宽都变为原来的0.6倍
	// BGR -> HSV
	cv::Mat hsv = BGR2HSV(img);

	cv::imshow("sample0", hsv);
	cv::waitKey(0);
	// Inverse Hue
	hsv = inverse_hue(hsv);

	cv::imshow("sample1", hsv);
	cv::waitKey(0);
	// Gray -> Binary
	cv::Mat out = HSV2BGR(hsv);

	//cv::imwrite("out.jpg", out);
	cv::imshow("sample", out);
	cv::waitKey(0);
	cv::destroyAllWindows();

	return 0;
}
/*/