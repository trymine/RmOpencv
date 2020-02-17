#include"Armor_Dection.h"
#include <vector>
Point2f Armor_Dection(Mat image, int color)
{

	Mat result;
	image.copyTo(result);
	//*************   ͨ�����������ɫ��ȡ   ************

	vector<Mat> channels;
	split(image, channels);//ͼ��ͨ������
	imshow("ahfklad", channels[2]);
	/*
	Mat channels_m[3];
	cv::Mat dst;

	channels_m[0] = channels[0];  // b
	channels[1] = 0;
	channels[2] = 0;
	merge(channels, dst);
	cv::imshow("b", dst);
	split(image, channels);//ͼ��ͨ������
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

	subtract(channels[c1], channels[c2], image);//����
	imshow("Color_Filter", image);
	threshold(image, image, 80, 255, THRESH_BINARY);  //��ֵ�ɵ��ڣ�thresh�ο���Χ60~160;typeΪ��ֵ���ͣ���Ϊ��������ֵ

	imshow("Binary Image", image);

	//for (int i = 0; i < 3; i++) {
	//	dilate(image, image, Mat());  //�������ͣ���ǿ����
	//}
	//imshow("Dilate Image", image);

	/********************        ������        ********************/
	vector< vector<Point> > ContourPoints;//��ά����
	findContours(image, ContourPoints, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE, Point(0, 0)); //��������
	/*ContourPointsΪ�������ϣ�RETR_EXTERNALֻ�������Χ��������������Χ�����ڵ���Χ���������ԣ�
	CV_CHAIN_APPROX_SIMPLE �����������Ĺյ���Ϣ�������������յ㴦�ĵ㱣����contours
	Pointƫ���������е�������Ϣ�����ԭʼͼ���Ӧ���ƫ�������൱����ÿһ���������������ϼ��ϸ�ƫ����������Point�������Ǹ�ֵ��*/
																						 /********************        �����С�����Բ        ********************/
	vector<RotatedRect> Archor_edge_Box(ContourPoints.size());
	RotatedRect box;
	int m = 0; //װ�ײ��淢�����Ŀ
	//ContourPoints.size
	for (int i = 0; i < ContourPoints.size(); i++) {
		if (ContourPoints[i].size() < 6) continue;
		box = fitEllipse(ContourPoints[i]);                    //�����С�����Բ
		if (box.angle > 45 && box.angle < 135) continue;
		if (box.angle < 45) box.angle = box.angle + 180;            /***********   ɸѡ����ת���νǶ�   **********/
		Archor_edge_Box[m] = box;
		m++;
	}
	/********************        ɸѡ��ƥ��        ********************/

	vector<Vec3f> couple((m*m - m) / 2);            //ǰ�����洢��Ӧ��ţ��������洢��Ӧ���װ�װ��x�᳤��
	vector<Point2f> Archor_Center((m*m - m) / 2);   //��Ӧװ�װ���������

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
		if (Archor_edge_Box[j].size.height * 1.6 < height_max) continue; //�height�ȣ�ȡ1.3~1.6���������
		for (int i = j + 1; i < m; i++)
		{
			if (abs(Archor_edge_Box[i].angle - (Archor_edge_Box[j].angle)) > 3) continue;  /*********** ��һ��ƥ�䣺 �Ƕ� **********/
			if (Archor_edge_Box[i].size.height * 1.6 < height_max) continue;       /***********   �ڶ���ƥ�䣺height   ***********/
																				   //if (abs(Archor_edge_Box[i].center.y - (Archor_edge_Box[j].center.y)) > (Archor_edge_Box[i].size.height + Archor_edge_Box[j].size.height) / 20) continue;  /***********   ������ƥ�䣺��������y��߶�   ***********/


																				   /***********   ����ƥ�������Ϣ��װ�ײ�߷������š�x���ϳ��ȡ���Ӧװ�װ���������   *****************/
			couple[n](0) = i;
			couple[n](1) = j;
			couple[n](2) = abs(Archor_edge_Box[i].center.x - Archor_edge_Box[j].center.x);
			Archor_Center[n].x = (Archor_edge_Box[couple[n](0)].center.x + Archor_edge_Box[couple[n](1)].center.x) / 2;
			Archor_Center[n].y = (Archor_edge_Box[couple[n](0)].center.y + Archor_edge_Box[couple[n](1)].center.y) / 2;

			n++;
		}
	}

	cout << n << endl;
	/*********   ��������˽ǶȽ�Ϊ����ġ�αװ�ײ�߷������������x�᷽���ϵ�װ�׳���ɸѡ��ȡ��λ��   **********/
	if (n > 2)
	{
		/*********   �ҳ�x��̳��ȶ�Ӧװ�ױ��   *********/
		for (int i = 0; i < n; i++)
		{
			if (Length_min > couple[i](2))
			{
				Length_min = couple[i](2);
				flag2 = i;
				cout << "Length_min=" << Length_min << endl;
			}
		}
		/********   ����αװ�װ���̱���е�װ�ײ��淢�����ŷ�������ִ����couples,�����αװ�װ����ҳ������   ************/
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


	/***********      �ҳ���ô����װ�װ���(x����ͶӰ�������)   ****************/
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