/*
*	@brief		创建多线程文件及其同步通信
*	@platform	Windows
*	@author		赵豪杰
*	@versiion	v1.0
*/

/*
*	创建线程方法1<process.h> C++库 _beginthreadex(...);
*				2<thread> C++11库 std::thread obj(CallbackFun);
*					包括函数库<atomic> ,<thread>,<mutex>,<condition_variable>和<future>
*				3<Windows.h> CreateThread(...);
*				4<pthread> Linux库
*				
*/
#include<Windows.h>
#include<highgui.hpp>
#include<time.h>
#include<iostream>
#include<CreatThread.h>
#include<fstream>

using namespace Robomaster;

namespace RM
{

/*
* @Brief:	SafeQueue
*/

/*
* @Brief:	CircularQueue
*/
	CircularQueue::CircularQueue(int size) :
		frame(size),
		head_t(0),
		tail_t(-1),
		emptyflag(1)
	{
		if (size <= 0)
			std::cout << "CircularQueue size error" << std::endl;
	}

	void CircularQueue::push(const Frame& img)
	{
		std::unique_lock<std::mutex> lock(mutexs);
		int newtail_t = (tail_t + 1) % frame.size();

		frame[newtail_t] = img;
		tail_t = newtail_t;
		if (emptyflag == 1){
			head_t = newtail_t;
			cond.notify_one();
		}
		else{
			if (head_t == newtail_t)
				head_t = (head_t + 1) % frame.size();
		}
		emptyflag = 0;
	}

	bool CircularQueue::wait_and_pop(Frame& img)
	{
		std::unique_lock<std::mutex> lock(mutexs);
	
		while (emptyflag)
		{
			cond.wait(lock);
		}
		img = frame[head_t];
		if (head_t == tail_t)//判读队列是否仅一个元素
		{
			emptyflag = 1;
			return true;
		}
		head_t = (head_t + 1) % frame.size();
		return true;

	}

/*
* @Brief:	ProcessClass core
*/

	ProcessClass::ProcessClass():
		queue(6)

	{
	}


	/*	Initial	*/
	void ProcessClass::Init()
	{
		detector.setEnemyColor(Robomaster::RED);

	}
	/*图像获取线程*/
	void ProcessClass::ImageAcquire()
	{
		clock_t start, end;
		Frame buffers;

		/*帧参数*/
		float VIDEO_FPS;
		int VIDEO_COUNT;
		/*帧参数*/

		/*读取帧*/
		//cv::VideoCapture cap(0);
		//cv::VideoCapture cap("D:/VS2017_projects/opencv/demo1/opencv_demo2/TestData/Picture/1.tif");
		cv::VideoCapture cap("D:/VS2017_projects/opencv/demo1/opencv_demo2/TestData/Video/led_30fps.avi");
		/*读取帧*/


		/*std与stdio解耦*/
		std::ios::sync_with_stdio(false);
		/*std与stdio解耦*/

		while (1)
		{
			
			/*运行并获取运行时间*/
			start = clock();

			/*调节帧率*/
			Sleep(30);
			/*调节帧率*/

			cap >> buffers.img;
			if (buffers.img.empty()) {
				cout << "can not load image" << endl;
				break;
			}

			/*获取帧参数*/
			//VIDEO_FPS = cap.get(cv::CAP_PROP_FPS);
			//VIDEO_FPS = int(round(VIDEO_FPS));
			//std::cout << "Video Frame fps:" << VIDEO_FPS << std::endl;

			//VIDEO_COUNT = cap.get(cv::CAP_PROP_FRAME_COUNT);
			//std::cout << "Video Frame count:" << VIDEO_COUNT << std::endl;

			//std::cout << "The cols of Frame :" << buffers.img.cols << std::endl;
			//std::cout << "The rows of Frame :" << buffers.img.rows << std::endl;
			/*获取帧参数*/

			/*将图片帧压入队列*/
			queue.push(buffers);
			/*将图片帧压入队列*/

			end = clock();
			//std::cout << "插入图片时间为:" << (double)(end - start) / CLOCKS_PER_SEC << std::endl;

			/*运行并获取运行时间*/

			break;//单个图片读取
		}
	}
	/*图像处理线程*/
	void ProcessClass::ImageProcess()
	{
		Frame img;
		Frame buffers;
		cv::Mat detectImg;
		clock_t start, end;
		int recive=0;

		/*std与stdio解耦*/
		std::ios::sync_with_stdio(false);
		/*std与stdio解耦*/

		while (1)
		{
			/*下一帧图像获取*/
			start = clock();
			queue.wait_and_pop(img);
			detector.loadImg(img.img);//图片载入工作区
			//imshow("Raw picture",img.img);
			/*下一帧图像获取*/

			/*单帧-灯条处理与显示*/
			{
				detector.DetectLight();

				if (detector._flag) {
					cout << "灯条中心：" << detector._targetLight.center << endl;
					//cv::ellipse(img.img, detector._targetLight, cv::Scalar(0, 255, 255), 2, 8);
					//circle(detector._srcImg, detector._targetLight.center, 2, Scalar(0, 255, 0), 2);
					imshow("寻找轮廓", img.img);
					waitKey(1);
				}
				else {
					cout << "Light Not Found!" << endl;
				}
				
			}
			/*单帧-灯条处理与显示*/

			/*视频帧-装甲板识别与显示*/
			//{
			//	detector.detect();

			//	if (detector._flag) {
			//		cout << "目标装甲板中心：" << detector._targetArmor.center << endl;
			//		//circle(img.img, detector._targetArmor.center, 2, Scalar(0, 0, 255), 2);
			//	}
			//	else {
			//		cout << "Armor Not Found!" << endl;
			//	}
			//	/*装甲板显示*/
			//	detector.draw_All_Armor(img.img);//画出所有装甲（绿色）
			//	detector.drawArmor_Points(img.img);//画出目标装甲（红色）
			//	imshow("检测装甲板", img.img);
			//	waitKey(1);
			//	/*装甲板显示*/
			//}
			/*视频帧-装甲板识别与显示*/

			/*耗时计算*/
			end = clock();
			std::cout << "读取一帧图片时间:" << (double)(end - start) / CLOCKS_PER_SEC << std::endl;
			/*耗时计算*/

		}
	}
	/*图像跟踪线程*/
	void ProcessClass::ImageTrack()
	{
		while (1)
		{

		}
	}
	/*通信线程*/
	void ProcessClass::Communicate()
	{
		ofstream fout;

		int x_axis;
		int y_axis;

		while (1)
		{

		}
	}


}

