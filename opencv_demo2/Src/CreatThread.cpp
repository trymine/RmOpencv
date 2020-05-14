/*
*	@brief		�������߳��ļ�����ͬ��ͨ��
*	@platform	Windows
*	@author		�Ժ���
*	@versiion	v1.0
*/

/*
*	�����̷߳���1<process.h> C++�� _beginthreadex(...);
*				2<thread> C++11�� std::thread obj(CallbackFun);
*					����������<atomic> ,<thread>,<mutex>,<condition_variable>��<future>
*				3<Windows.h> CreateThread(...);
*				4<pthread> Linux��
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
		if (head_t == tail_t)//�ж������Ƿ��һ��Ԫ��
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
	/*ͼ���ȡ�߳�*/
	void ProcessClass::ImageAcquire()
	{
		clock_t start, end;
		Frame buffers;

		/*֡����*/
		float VIDEO_FPS;
		int VIDEO_COUNT;
		/*֡����*/

		/*��ȡ֡*/
		//cv::VideoCapture cap(0);
		//cv::VideoCapture cap("D:/VS2017_projects/opencv/demo1/opencv_demo2/TestData/Picture/1.tif");
		cv::VideoCapture cap("D:/VS2017_projects/opencv/demo1/opencv_demo2/TestData/Video/led_30fps.avi");
		/*��ȡ֡*/


		/*std��stdio����*/
		std::ios::sync_with_stdio(false);
		/*std��stdio����*/

		while (1)
		{
			
			/*���в���ȡ����ʱ��*/
			start = clock();

			/*����֡��*/
			Sleep(30);
			/*����֡��*/

			cap >> buffers.img;
			if (buffers.img.empty()) {
				cout << "can not load image" << endl;
				break;
			}

			/*��ȡ֡����*/
			//VIDEO_FPS = cap.get(cv::CAP_PROP_FPS);
			//VIDEO_FPS = int(round(VIDEO_FPS));
			//std::cout << "Video Frame fps:" << VIDEO_FPS << std::endl;

			//VIDEO_COUNT = cap.get(cv::CAP_PROP_FRAME_COUNT);
			//std::cout << "Video Frame count:" << VIDEO_COUNT << std::endl;

			//std::cout << "The cols of Frame :" << buffers.img.cols << std::endl;
			//std::cout << "The rows of Frame :" << buffers.img.rows << std::endl;
			/*��ȡ֡����*/

			/*��ͼƬ֡ѹ�����*/
			queue.push(buffers);
			/*��ͼƬ֡ѹ�����*/

			end = clock();
			//std::cout << "����ͼƬʱ��Ϊ:" << (double)(end - start) / CLOCKS_PER_SEC << std::endl;

			/*���в���ȡ����ʱ��*/

			break;//����ͼƬ��ȡ
		}
	}
	/*ͼ�����߳�*/
	void ProcessClass::ImageProcess()
	{
		Frame img;
		Frame buffers;
		cv::Mat detectImg;
		clock_t start, end;
		int recive=0;

		/*std��stdio����*/
		std::ios::sync_with_stdio(false);
		/*std��stdio����*/

		while (1)
		{
			/*��һ֡ͼ���ȡ*/
			start = clock();
			queue.wait_and_pop(img);
			detector.loadImg(img.img);//ͼƬ���빤����
			//imshow("Raw picture",img.img);
			/*��һ֡ͼ���ȡ*/

			/*��֡-������������ʾ*/
			{
				detector.DetectLight();

				if (detector._flag) {
					cout << "�������ģ�" << detector._targetLight.center << endl;
					//cv::ellipse(img.img, detector._targetLight, cv::Scalar(0, 255, 255), 2, 8);
					//circle(detector._srcImg, detector._targetLight.center, 2, Scalar(0, 255, 0), 2);
					imshow("Ѱ������", img.img);
					waitKey(1);
				}
				else {
					cout << "Light Not Found!" << endl;
				}
				
			}
			/*��֡-������������ʾ*/

			/*��Ƶ֡-װ�װ�ʶ������ʾ*/
			//{
			//	detector.detect();

			//	if (detector._flag) {
			//		cout << "Ŀ��װ�װ����ģ�" << detector._targetArmor.center << endl;
			//		//circle(img.img, detector._targetArmor.center, 2, Scalar(0, 0, 255), 2);
			//	}
			//	else {
			//		cout << "Armor Not Found!" << endl;
			//	}
			//	/*װ�װ���ʾ*/
			//	detector.draw_All_Armor(img.img);//��������װ�ף���ɫ��
			//	detector.drawArmor_Points(img.img);//����Ŀ��װ�ף���ɫ��
			//	imshow("���װ�װ�", img.img);
			//	waitKey(1);
			//	/*װ�װ���ʾ*/
			//}
			/*��Ƶ֡-װ�װ�ʶ������ʾ*/

			/*��ʱ����*/
			end = clock();
			std::cout << "��ȡһ֡ͼƬʱ��:" << (double)(end - start) / CLOCKS_PER_SEC << std::endl;
			/*��ʱ����*/

		}
	}
	/*ͼ������߳�*/
	void ProcessClass::ImageTrack()
	{
		while (1)
		{

		}
	}
	/*ͨ���߳�*/
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

