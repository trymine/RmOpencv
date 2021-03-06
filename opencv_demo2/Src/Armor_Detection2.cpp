#include"Armor_Detection2.h"

namespace Robomaster
{

	ArmorParam _param;

	/*
	*	@Brief: Initialize with all 0
	*/
	ArmorDescriptor::ArmorDescriptor()
	{
		sizeScore = 0;
		distScore = 0;
		disProbabilityScore = 0;
		finalScore = 0;
		vertex.resize(4);
		for (int i = 0; i < 4; i++)
		{
			vertex[i] = cv::Point2f(0, 0);
		}

	}

	/*
	*	@Brief: calculate the rest of information(except for match&final score)of ArmroDescriptor based on:
			l&r light, part of members in ArmorDetector, and the armortype(for the sake of saving time)
	*	@Calls: ArmorDescriptor::getFrontImg()
	*/
	ArmorDescriptor::ArmorDescriptor(const LightDescriptor& lLight,
									 const LightDescriptor& rLight)
	{
		//handle two lights
		lightPairs[0] = lLight.rec();
		lightPairs[1] = rLight.rec();

		cv::Size exLSize(int(lightPairs[0].size.width), int(lightPairs[0].size.height * 2));
		cv::Size exRSize(int(lightPairs[1].size.width), int(lightPairs[1].size.height * 2));
		cv::RotatedRect exLLight(lightPairs[0].center, exLSize, (lightPairs[0].angle + lightPairs[1].angle) / 2);
		cv::RotatedRect exRLight(lightPairs[1].center, exRSize, (lightPairs[0].angle + lightPairs[1].angle) / 2);

		cv::Point2f pts_l[4];
		exLLight.points(pts_l);
		cv::Point2f upper_l = pts_l[2];
		cv::Point2f lower_l = pts_l[3];

		cv::Point2f pts_r[4];
		exRLight.points(pts_r);
		cv::Point2f upper_r = pts_r[1];
		cv::Point2f lower_r = pts_r[0];

		vertex.resize(4);
		vertex[0] = upper_l;
		vertex[1] = upper_r;
		vertex[2] = lower_r;
		vertex[3] = lower_l;


		//set armor center
		center = cvex::crossPointOf(array<Point2f, 2>{vertex[0], vertex[2]}, array<Point2f, 2>{vertex[1], vertex[3]});

		//set armor length
		length = lightPairs[1].center.x - lightPairs[0].center.x;

		// calculate the size score
		float normalized_area = contourArea(vertex) / _param.area_normalized_base;
		sizeScore = exp(normalized_area);


		// calculate the distance score

		Point2f srcImgCenter(_param.srcImageSize.height / 2, _param.srcImageSize.width / 2);
		float sightOffset = cvex::distance(srcImgCenter, center);
		distScore = exp(sightOffset / _param.sight_offset_normalized_base);

		//finalScore = sizeScore + distScore;
		finalScore = sizeScore;
	}



	ArmorDetector::ArmorDetector() 
	{
		_flag = ARMOR_NO;
		_targetArmor.angle = 0;
		_targetArmor.center = Point2f(300, 400);
		_targetArmor.vertex[0] = Point2f(0, 0);
		_targetArmor.vertex[1] = Point2f(0, 800);
		_targetArmor.vertex[2] = Point2f(600, 800);
		_targetArmor.vertex[3] = Point2f(600, 0);

	}


	/*
	*	@Brief: Detect Led
	*	@Outputs: ALL the info of detection result
	*	@
	*	@Calls: The Consumer
	*/
	int ArmorDetector::DetectLight()
	{

		/*build light bars' desciptors*/
		std::vector<LightDescriptor> lightInfos;
		/*build light bars' desciptors*/

		/*
		*	pre-treatment
		*/

		// 通道相减分割颜色
		std::vector<cv::Mat> channels;
		cv::Mat image;
		split(_srcImg, channels);// 把一个3通道图像转换成3个单通道图像

		int c1, c2;

		if (_enemy_color == BLUE)
		{
			c1 = 0;
			c2 = 2;
		}
		if (_enemy_color == RED)
		{
			c1 = 2;
			c2 = 0;
		}

		/*
		*	颜色通道相减过滤颜色,删除己方装甲板颜色,得到灰度图
		*/
		subtract(channels[c1], channels[c2], image);
		//imshow("颜色分离", image);
		/*
		*	二值化，100为鲁棒性最佳
		*/
		cv::threshold(image, image, _param.color_Threshold, 255, cv::THRESH_BINARY);
		//imshow("二值化", image);
		/*
		*	膨胀，防止灯条形状异变
		*		1产生结构化（特定大小形状）元素，用于形态学处理
		*		2膨胀
		*/
		Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(3, 3));
		//imshow("BeforeBin", image);
		dilate(image, image, kernel);

		//imshow("膨胀处理", image);

		/*
		*	寻找轮廓
		*/

		vector<vector<Point>> lightContours;
		//最耗时，只检测外部轮廓，且仅保留轮廓拐点信息进输出向量里
		cv::findContours(image, lightContours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

		/*
		*	找到轮廓后开始遍历轮廓提取灯条
		*/
		for (const auto& contour : lightContours)
		{
			//得到面积
			float lightContourArea = contourArea(contour);
			//minAreaRect();
			//面积太小的不要
			if (contour.size() <= 5 ||
				lightContourArea < _param.light_min_area)
				continue;
			//用椭圆拟合区域得到外接矩形
			RotatedRect lightRec = fitEllipse(contour);

			//旋转角度范围为[-90,0)，逆时针旋转碰到的第一个边称为宽，逆时针旋转方向为负
			//Adjust the RotatedRect lightRec angle
			if (lightRec.angle > 135)
				lightRec.angle -= 180.0;

			//filter light according to the angle
			if (abs(lightRec.angle) > 45)
				continue;


			//绘制外接椭圆（影响性能）
			//cv::ellipse(_srcImg, lightRec, cv::Scalar(0, 255, 255), 2, 8);
			//绘制外接矩形（影响性能）
			//drawTargetLight_Points(lightRec);


			lightInfos.emplace_back(lightRec);

		}

		if (lightInfos.empty())
			return _flag = ARMOR_NO;

		/*存在灯条*/
		_targetLight=lightInfos[0];
		/*存在灯条*/

		return _flag = ARMOR_FOUND;

	}

	//draw the led
	void ArmorDetector::drawTargetLight_Points(RotatedRect &rotate_rect)
	{
		
			//for (int i = 0; i < 4; i++) {
			//	//line(src, _targetLight.vertex[i], _targetLight.vertex[(i + 1) % 4], Scalar(0, 0, 255), 1, LINE_AA);
			//	circle(src, _targetLight.center, 2, Scalar(0, 0, 255), 2);
			//}
				//获取旋转矩形的四个顶点
			cv::Point2f* vertices = new cv::Point2f[4];
			rotate_rect.points(vertices);

			//逐条边绘制
			for (int j = 0; j < 4; j++)
			{
				cv::line(_srcImg, vertices[j], vertices[(j + 1) % 4], cv::Scalar(0, 255, 0));
			}

	}



	/*
	*	@Brief: core of detection algrithm, include all the main detection process
	*	@Outputs: ALL the info of detection result
	*	@Return: See enum DetectorFlag
	*	@Others: API for client
	*/
	int ArmorDetector::detect()
	{

		/*
		*	Detect lights and build light bars' desciptors
		*/
		_armors.clear();
		_True_armors.clear();

		std::vector<LightDescriptor> lightInfos;


		/*
		*	pre-treatment
		*/

		// 通道相减分割颜色


		std::vector<cv::Mat> channels;
		cv::Mat image;
		split(_srcImg, channels);// 把一个3通道图像转换成3个单通道图像

		int c1, c2;

		if (_enemy_color == BLUE)
		{
			c1 = 0;
			c2 = 2;
		}
		if (_enemy_color == RED)
		{
			c1 = 2;
			c2 = 0;
		}

		/*
		*	颜色通道相减过滤颜色,删除己方装甲板颜色,得到灰度图
		*/
		subtract(channels[c1], channels[c2], image);
		//imshow("subtract", image);
		/*
		*	二值化，100为鲁棒性最佳
		*/
		cv::threshold(image, image, _param.color_Threshold, 255, cv::THRESH_BINARY);
		//imshow("afterthreshold", image);
		/*
		*	膨胀，防止灯条形状异变
		*		1产生结构化（特定大小形状）元素，用于形态学处理
		*		2膨胀
		*/
		Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(3, 3));
		//imshow("BeforeBin", image);
		dilate(image, image, kernel);
		
		//imshow("BinImage", image);

		/*
		*	寻找轮廓
		*/

		vector<vector<Point>> lightContours;
		//最耗时
		cv::findContours(image, lightContours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

		/*
		*	找到轮廓后开始遍历轮廓提取灯条
		*/
		for (const auto& contour : lightContours)
		{
			//得到面积
			float lightContourArea = contourArea(contour);
			//minAreaRect();
			//面积太小的不要
			if (contour.size() <= 5 ||
				lightContourArea < _param.light_min_area)
				continue;
			//用椭圆拟合区域得到外接矩形
			RotatedRect lightRec = fitEllipse(contour);


			//Adjust the RotatedRect lightRec angle
			if (lightRec.angle > 135)
				lightRec.angle -= 180.0;

			//filter light according to the angle
			if (abs(lightRec.angle) > 45)
				continue;


			lightInfos.emplace_back(lightRec);

		}

		if (lightInfos.empty())
			return _flag = ARMOR_NO;


		/*
		*	进行灯条的匹配筛选
		*/

		//升序排列灯条 将灯条从左到右排列（按灯条中心x从小到大排序）
		sort(lightInfos.begin(), lightInfos.end(), [](const LightDescriptor& ld1, const LightDescriptor& ld2)
		{
			return ld1.center.x < ld2.center.x;
		});



		//将检测到的装甲板从左到右一个一个进行匹配并筛选出初装甲板s

		for (size_t i = 0; i < lightInfos.size(); i++)
		{
			for (size_t j = i + 1; j < lightInfos.size(); j++)
			{
				const LightDescriptor& leftLight = lightInfos[i];
				const LightDescriptor& rightLight = lightInfos[j];

				/*
				*	Works for 3-7 meters situation
				*	morphologically similar: //lighhts are parallel
									// both similar height
				*/
				//角差
				float angleDiff_ = abs(leftLight.angle - rightLight.angle);
				//长度差比率
				float LenDiff_ratio = abs(leftLight.length - rightLight.length) / max(leftLight.length, rightLight.length);

				if (angleDiff_ > _param.light_max_angle_diff ||
					LenDiff_ratio > _param.light_max_height_diff_ratio)
				{
					continue;
				}



				/*
				*	proper location:  // y value of light bar close enough
				*					 // ratio of length and width is proper
				*/
				//左右灯条长度的平均值
				float meanLen = (leftLight.length + rightLight.length) / 2;
				//左右灯条中心点y的差值和差比率
				float yDiff = abs(leftLight.center.y - rightLight.center.y);
				float yDiff_ratio = yDiff / meanLen;
				//左右灯条中心点x的差值和差比率
				float xDiff = abs(leftLight.center.x - rightLight.center.x);
				float xDiff_ratio = xDiff / meanLen;

				if (yDiff_ratio > _param.light_max_y_diff_ratio ||
					xDiff_ratio > _param.light_max_x_diff_ratio)//或者xDiff_ratio < _param.light_min_x_diff_ratio_
					continue;



				ArmorDescriptor armor(leftLight, rightLight);

				armor.lightsFlags[0] = i;
				armor.lightsFlags[1] = j;
				armor.disProbabilityScore = exp2f(angleDiff_/3) + exp2f(yDiff_ratio / 0.2) + exp2f(LenDiff_ratio / 0.5);
				_armors.emplace_back(armor);
				break;
			}
		}

		if (_armors.empty())
		{
			return _flag = ARMOR_NO;
		}


		// 过滤出假装甲板（两个中非相似度disProbability得分更高）

		for (int i = 0; i < _armors.size(); i++)
		{
			bool true_or_false = 1;
			if (_armors.size() >= 2)
			{
				for (int j = i + 1; j < _armors.size(); j++)
				{
					if (_armors[i].lightsFlags[0] == _armors[j].lightsFlags[0] ||
						_armors[i].lightsFlags[0] == _armors[j].lightsFlags[1] ||
						_armors[i].lightsFlags[1] == _armors[j].lightsFlags[0] ||
						_armors[i].lightsFlags[1] == _armors[j].lightsFlags[1])
					{
						true_or_false = 0;
						if (_armors[i].length < _armors[j].length)
						{
							_armors[i].disProbabilityScore += exp(3);
						}
						_armors[i].disProbabilityScore < _armors[j].disProbabilityScore ? _True_armors.emplace_back(_armors[i]) : _True_armors.emplace_back(_armors[j]);

						////  **********************调试的可删
						//int a;
						//_armors[i].disProbabilityScore < _armors[j].disProbabilityScore ? a=j : a=i;
						//circle(_srcImg, _armors[a].center, 5, Scalar(255, 127, 255), 2);
						////


						i++;
						break;
					}
				}
			}
			if (true_or_false)
				_True_armors.emplace_back(_armors[i]);

		}


		//对所有识别的真装甲板排序并选取最高得分装甲板为target
		sort(_True_armors.begin(), _True_armors.end(), [](const ArmorDescriptor& ld1, const ArmorDescriptor& ld2)
		{
			return ld1.finalScore > ld2.finalScore;
		});

		_targetArmor = _True_armors[0];
		return _flag = ARMOR_FOUND;
	};



	//draw the target armor
	void ArmorDetector::drawArmor_Points(Mat src)
	{
		if (_flag)
		{
			for (int i = 0; i < 4; i++) {
				line(src, _targetArmor.vertex[i], _targetArmor.vertex[(i + 1) % 4], Scalar(0, 0, 255), 1, LINE_AA);
				circle(src, _targetArmor.center, 2, Scalar(0, 0, 255), 2);
			}
		}

	}


	//draw all the true armors
	void ArmorDetector::draw_All_Armor(Mat src)
	{
		if (_flag)
		{
			for (size_t i = 0; i < _True_armors.size(); i++)
			{
				for (int j = 0; j < 4; j++) {
					line(src, _True_armors[i].vertex[j], _True_armors[i].vertex[(j + 1) % 4], Scalar(0, 255, 0), 1, LINE_AA);
					circle(src, _True_armors[i].center, 2, Scalar(0, 255, 0), 2);
				}
			}

		}

	}

}