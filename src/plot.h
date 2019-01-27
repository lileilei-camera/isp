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
	void DrawAxis (IplImage *image); //画坐标轴
	void DrawData (IplImage *image); //画点
	int window_height; //窗口大小
	int window_width;


	vector< vector<CvPoint2D64f> >dataset;	//点集合
	vector<LineType> lineTypeSet; //线的类型
	
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
	
	//边界大小
	int border_size;
		
	template<class T>
	void plot(T *y, size_t Cnt, CvScalar color, char type = '*',bool is_need_lined = true);	
	template<class T>
	void plot(T *x, T *y, size_t Cnt, CvScalar color, char type = '*',bool is_need_lined = true);
		
	void xlabel(string xlabel_name, CvScalar label_color);
	void ylabel(string ylabel_name, CvScalar label_color);
	//清空图片上的数据
	void clear();
	void title(string title_name,CvScalar title_color); 
	
	CPlot();
	~CPlot();
		
};
#endif
