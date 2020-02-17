#include"Armor_Dection.h"
#include <vector>
Point2f Armor_Dection(Mat image, int color)
{

	Mat result;
	image.copyTo(result);
	//*************   通道相减进行颜色提取   ************

	vector<Mat> channels;
	split(image, channels);//图像通道分离
	imshow("ahfklad", channels[2]);
	/*
	Mat channels_m[3];
	cv::Mat dst;

	channels_m[0] = channels[0];  // b
	channels[1] = 0;
	channels[2] = 0;
	merge(channels, dst);
	cv::imshow("b", dst);
	split(image, channels);//图像通道分离
	*/
	int c1, c2;
	if (color == BLUE)
	{
		c1 = 0;
		c2 = 2;
	}
	if (color == RED)
	{
		c1 = 2;
		c2 = 0;
	}

	subtract(channels[c1], channels[c2], image);//减法
	imshow("Color_Filter", image);
	threshold(image, image, 80, 255, THRESH_BINARY);  //阈值可调节，thresh参考范围60~160;type为阈值类型，该为二进制阈值

	imshow("Binary Image", image);

	//for (int i = 0; i < 3; i++) {
	//	dilate(image, image, Mat());  //基础膨胀，增强特征
	//}
	//imshow("Dilate Image", image);

	/********************        找轮廓        ********************/
	vector< vector<Point> > ContourPoints;//二维数组
	findContours(image, ContourPoints, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE, Point(0, 0)); //轮廓发现
	/*ContourPoints为轮廓集合，RETR_EXTERNAL只检测最外围轮廓，包含在外围轮廓内的内围轮廓被忽略，
	CV_CHAIN_APPROX_SIMPLE 仅保存轮廓的拐点信息，把所有轮廓拐点处的点保存入contours
	Point偏移量，所有的轮廓信息相对于原始图像对应点的偏移量，相当于在每一个检测出的轮廓点上加上该偏移量，并且Point还可以是负值！*/
																						 /********************        拟合最小外接椭圆        ********************/
	vector<RotatedRect> Archor_edge_Box(ContourPoints.size());
	RotatedRect box;
	int m = 0; //装甲侧面发光带数目
	//ContourPoints.size
	for (int i = 0; i < ContourPoints.size(); i++) {
		if (ContourPoints[i].size() < 6) continue;
		box = fitEllipse(ContourPoints[i]);                    //拟合最小外接椭圆
		if (box.angle > 45 && box.angle < 135) continue;
		if (box.angle < 45) box.angle = box.angle + 180;            /***********   筛选：旋转矩形角度   **********/
		Archor_edge_Box[m] = box;
		m++;
	}
	/********************        筛选与匹配        ********************/

	vector<Vec3f> couple((m*m - m) / 2);            //前两个存储对应编号，第三个存储对应检测装甲板的x轴长度
	vector<Point2f> Archor_Center((m*m - m) / 2);   //对应装甲板中心坐标

	float height_max = 0;
	float Length_max = 0;
	float Length_min = 0xffff;
	float Length_Of_Fake = 0;
	int n = 0;
	int flag1 = 0;
	int flag2 = -1;
	int flag3 = -1;


	for (int i = 0; i < m; i++) {
		if (height_max < Archor_edge_Box[i].size.height)
			height_max = Archor_edge_Box[i].size.height;
		cout << "*****  " << i << "  *****" << endl;
		cout << Archor_edge_Box[i].size.height << endl;
		cout << Archor_edge_Box[i].center << endl;
	}

	for (int j = 0; j < m; j++)
	{
		if (Archor_edge_Box[j].size.height * 1.6 < height_max) continue; //最长height比，取1.3~1.6，过滤噪点
		for (int i = j + 1; i < m; i++)
		{
			if (abs(Archor_edge_Box[i].angle - (Archor_edge_Box[j].angle)) > 3) continue;  /*********** 第一步匹配： 角度 **********/
			if (Archor_edge_Box[i].size.height * 1.6 < height_max) continue;       /***********   第二步匹配：height   ***********/
																				   //if (abs(Archor_edge_Box[i].center.y - (Archor_edge_Box[j].center.y)) > (Archor_edge_Box[i].size.height + Archor_edge_Box[j].size.height) / 20) continue;  /***********   第三步匹配：中心坐标y轴高度   ***********/


																				   /***********   储存匹配相关信息：装甲侧边发光带编号、x轴上长度、对应装甲板中心坐标   *****************/
			couple[n](0) = i;
			couple[n](1) = j;
			couple[n](2) = abs(Archor_edge_Box[i].center.x - Archor_edge_Box[j].center.x);
			Archor_Center[n].x = (Archor_edge_Box[couple[n](0)].center.x + Archor_edge_Box[couple[n](1)].center.x) / 2;
			Archor_Center[n].y = (Archor_edge_Box[couple[n](0)].center.y + Archor_edge_Box[couple[n](1)].center.y) / 2;

			n++;
		}
	}

	cout << n << endl;
	/*********   如果出现了角度较为相近的“伪装甲侧边发光带”，进行x轴方向上的装甲长度筛选，取中位数   **********/
	if (n > 2)
	{
		/*********   找出x最短长度对应装甲编号   *********/
		for (int i = 0; i < n; i++)
		{
			if (Length_min > couple[i](2))
			{
				Length_min = couple[i](2);
				flag2 = i;
				cout << "Length_min=" << Length_min << endl;
			}
		}
		/********   利用伪装甲板最短编号中的装甲侧面发光带编号反向求出现错误的couples,把最长的伪装甲板编号找出并标记   ************/
		for (int i = 0; i < n; i++)
		{
			if (i == flag1) continue;
			if (couple[i](0) == couple[flag2](0) || couple[i](0) == couple[flag2](1) || couple[i](1) == couple[flag2](0) || couple[i](1) == couple[flag2](1))
			{
				if (Length_Of_Fake < couple[i](2))
				{
					Length_Of_Fake = couple[i](2);
					flag3 = i;
					cout << "Length_Of_Fake=" << Length_Of_Fake << endl;
				}
			}
		}
	}


	/***********      找出最好打击的装甲板编号(x轴上投影长度最长的)   ****************/
	for (int i = 0; i < n; i++)
	{
		if (i == flag2) continue;
		if (i == flag3) continue;

		circle(result, Archor_Center[i], 5, Scalar(0, 0, 255), 3);

		if (Length_max < couple[i](2))
		{
			Length_max = couple[i](2);
			flag1 = i;
		}
	}
	if (n)
	{
		circle(result, Archor_Center[flag1], 5, Scalar(0, 255, 255), 3);
		imshow("Armor_Dection", result);
		return Archor_Center[flag1];
	}
	else
	{
		Point2f zero;
		return zero;
	}
}