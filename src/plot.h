#ifndef _PLOT_H
#define _PLOT_H

#include <cmath>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <math.h>
#define WINDOW_WIDTH 600
#define WINDOW_HEIGHT 600
using namespace cv;
using namespace std;

struct LineType
{
	char type;
	bool is_need_lined;	
	Scalar color;
};

class CPlot
{
public:	
	void DrawAxis (IplImage *image); //��������
	void DrawData (IplImage *image); //����
	int window_height; //���ڴ�С
	int window_width;


	vector< vector<CvPoint2D64f> >dataset;	//�㼯��
	vector<LineType> lineTypeSet; //�ߵ�����
	
	//color
	CvScalar backgroud_color;
	CvScalar axis_color;
	CvScalar text_color;

	IplImage* Figure;

	// manual or automatic range
	bool custom_range_y;
	double y_max;
	double y_min;
	double y_scale;

	bool custom_range_x;
	double x_max;
	double x_min;
	double x_scale;
	
	//�߽��С
	int border_size;
		
	template<class T>
	void plot(T *y, size_t Cnt, CvScalar color, char type = '*',bool is_need_lined = true);	
	template<class T>
	void plot(T *x, T *y, size_t Cnt, CvScalar color, char type = '*',bool is_need_lined = true);
		
	void xlabel(string xlabel_name, CvScalar label_color);
	void ylabel(string ylabel_name, CvScalar label_color);
	//���ͼƬ�ϵ�����
	void clear();
	void title(string title_name,CvScalar title_color); 
	
	CPlot();
	~CPlot();
		
};
#endif
