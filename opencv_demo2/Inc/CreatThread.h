#pragma once
#include <iostream>
#include <opencv.hpp>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <memory>
#include"Armor_Detection2.h"
#define MAX_SIZE_IMGBUFFER 5
using namespace Robomaster;
namespace RM
{

/*
* @Brief:	SafeQueue
*/
template <typename T>
class SafeQueue
{
	private:
		//容器
		std::queue<T> data_queue;
		//必须是mutable，因为empty是const方法，但是要锁mut，锁操作就是改变操作
		mutable std::mutex mut;
		std::condition_variable data_cond;
	public:
		SafeQueue() {}
		SafeQueue(SafeQueue const& other)
		{
			std::lock_guard<std::mutex> lk(other.mut);
			data_queue = other.data_queue();
		}

		void push(T new_element)
		{
			std::lock_guard<std::mutex> lk(mut);
			data_queue.push(new_element);
			data_cond.notify_one();
		}

		void wait_and_pop(T& element)
		{
			std::unique_lock<std::mutex> lk(mut);
			data_cond.wait(lk, [this] {return !data_queue.empty(); });
			element = data_queue.front();
			data_queue.pop();
		}

		std::shared_ptr<T> wait_and_pop()
		{
			std::unique_lock<std::mutex> lk(mut);
			data_cond.wait(lk, [this] {return !data_queue.empty(); });
			std::shared_ptr<T> res(std::make_shared<T>(data_queue.front()));
			data_queue.pop();
			return res;
		}

		bool empty()const
		{
			std::lock_guard<std::mutex> lk(mut);
			return data_queue.empty();
		}

	};/*class SafeQueue*/

struct Frame
{
	cv::Mat img;
	//size_t seq;         //count from 1
	//double timeStamp;	//time in ms, from initialization to now
};

class CircularQueue
{
private:
	mutable std::mutex mutexs;
	std::vector<Frame> frame;
	std::condition_variable cond;

	int head_t;
	int tail_t;
	int emptyflag;
public:
	CircularQueue(int size);

	void push(const Frame& img);

	bool wait_and_pop(Frame& img);


};
/*
* @Brief:   This class aims at separating reading(producing) images and consuming(using)
*           images into different threads. New images read from the camera are stored
*           into a circular queue. New image will replace the oldest one.
*/
class ProcessClass
{
private:
	//SafeQueue< cv::Mat >  queue;
	CircularQueue queue;
	ArmorDetector detector;
public:
	ProcessClass();
	//~ProcessClass();
	/*初始化*/
	void Init();
	/*图像获取线程*/
	 void ImageAcquire();
	/*图像处理线程*/
	 void ImageProcess();
	/*图像跟踪线程*/
	void ImageTrack();
	/*通信线程*/
	void Communicate();

};

}