#pragma once
#include"opencv_extended2.h"

using namespace cv;
using namespace std;

namespace Robomaster
{

	/*
	* 颜色 B G R
	*/
	enum Colors
	{
		BLUE = 0,
		GREEN = 1,
		RED = 2
	};

	/*
	*	该结构体储存所有装甲板识别所需的参数
	*/
	struct ArmorParam
	{

		//Pre-treatment 预处理
		int color_Threshold = 100;  //recommend 80~120

		//Filter lights 灯条过滤
		float light_min_area = 10;
		float light_max_angle = 45;

		//Filter pairs 灯条匹配过滤
		float light_max_angle_diff = 8;
		float light_max_height_diff_ratio = 0.5;
		float light_max_y_diff_ratio = 0.5;
		float light_max_x_diff_ratio = 3;

		//other params 其他参数
		float sight_offset_normalized_base = 100;
		float area_normalized_base = 500;
		int enemy_color = BLUE;
		Size srcImageSize = Size(800, 800);
	};


	/*
	*   灯条类，灯条描述子，包含了描述灯条的所有对象及其属性 长、宽、角度、面积、中心点
	*/
	class LightDescriptor
	{
	public:

		/*
		*	@Brief: Initialize with all 0
		*/
		LightDescriptor() 
		{
			width = 0;
			length = 0;
			center = Point2f(0, 0);
			angle = 0;
			area = 0;
		}

		LightDescriptor(const RotatedRect& light)
		{
			width = light.size.width;
			length = light.size.height;
			center = light.center;
			angle = light.angle;
			area = light.size.area();
		}

		const LightDescriptor& operator =(const LightDescriptor& ld)
		{
			this->width = ld.width;
			this->length = ld.length;
			this->center = ld.center;
			this->angle = ld.angle;
			this->area = ld.area;
			return *this;
		}

		/*
		*	@Brief: return the light as a cv::RotatedRect object
		*/
		RotatedRect rec() const
		{
			return RotatedRect(center, Size2f(width, length), angle);
		}


	public:
		float width;
		float length;
		Point2f center;
		float angle;
		float area;
	};


	/*
	* 	    装甲板类,装甲板描述子，包含了描述装甲板的所有对象及其属性 左右灯条、装甲板四顶点、装甲板中心点   This class describes the armor information, including maximum bounding box, vertex and so on
	*/
	class ArmorDescriptor
	{
	public:

		/*
		*	@Brief: Initialize with all 0
		*/
		ArmorDescriptor();

		/*
		*	@Brief: calculate the rest of information(except for match&final score)of ArmroDescriptor based on:
				l&r light, part of members in ArmorDetector, and the armortype(for the sake of saving time)
		*/
		ArmorDescriptor(const LightDescriptor& lLight,
						const LightDescriptor& rLight);


		/*
		*	@Brief: empty the object
		*	@Called :ArmorDetection._targetArmor
		*/
		void clear()
		{
			disProbabilityScore = 0;
			sizeScore = 0;
			distScore = 0;
			finalScore = 0;
			for (int i = 0; i < 4; i++)
			{
				vertex[i] = cv::Point2f(0, 0);
			}
		}


	public:
		std::array<cv::RotatedRect, 2> lightPairs;				//0 left, 1 right
		std::array<int, 2> lightsFlags;						  //0 left's flag, 1 right's flag
		std::vector<cv::Point2f> vertex;					 //four vertex of armor area, light bar area exclued!!	
		cv::Point2f center;								    //center point of armor
		float angle;
		float length;

		float sizeScore;		//S1 = e^(size)
		float distScore;		//S2 = e^(-offset)
		float disProbabilityScore;  //S3 = e^(angleDiff)+e^(length_ratio)+e^(yDiff_ratio)
		float finalScore;		//sum of all the scores
	};


	/*
	*	    装甲板识别所用的各类函数与对象  This class implement all the functions of detecting the armor
	*/
	class ArmorDetector
	{
	public:
		/*
		*	flag for the detection result
		*/
		enum DetectorFlag
		{
			ARMOR_NO = 0, 		// not found
			ARMOR_FOUND = 1         // found target_Armor
		};

	public:
		ArmorDetector();

		/*
		*	@Brief: set the enemy's color
		*	@Others: API for client
		*/
		void setEnemyColor(int enemy_color)
		{
			_enemy_color = enemy_color;
		}

		/*
		*	@Brief: load image
		*	@Input: frame
		*/
		void loadImg(const cv::Mat&  srcImg)
		{
			_srcImg = srcImg;
		}

		/*
		*	@Brief: core of detection algrithm, include all the main detection process
		*	@Outputs: ALL the info of detection result
		*	@Return: See enum DetectorFlag
		*	@Others: API for client
		*/
		int detect();

		//draw the target armor
		void drawArmor_Points(Mat src);

		//draw all the true armors
		void draw_All_Armor(Mat src);


	public:

		cv::Mat _srcImg; //source img

		int _enemy_color;

		std::vector<ArmorDescriptor> _armors;
		std::vector<ArmorDescriptor> _True_armors;
		ArmorDescriptor _targetArmor; //relative coordinates

		int _flag = ARMOR_NO;

	};


}

