// Matlab style plot functions for OpenCV
//author libing64 && Jack Dong
//https://github.com/libing64/CPlot
//
#include "cv.h"
#include "highgui.h"
#include "plot.h"
#include <iostream>


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
//采用范型设计，因此将实现部分和声明部分放在一个文件中
CPlot::CPlot()
{
	this->border_size = 40; //图外围边界
	this->window_height = WINDOW_HEIGHT;
	this->window_width = WINDOW_WIDTH;
	this->Figure = cvCreateImage(cvSize(this->window_height, this->window_width),IPL_DEPTH_8U, 3);
	memset(Figure->imageData, 255, sizeof(unsigned char)*Figure->widthStep*Figure->height);
	//color
	this->backgroud_color = CV_RGB(255,255,255); //背景白色
	this->axis_color = CV_RGB(0,0,0);//坐标黑色
	this->text_color = CV_RGB(255,0 ,0); //文字红色
	this->x_min = 0;
	this->x_max = 0;
	this->y_min = 0;
	this->y_max = 0;
}

CPlot::~CPlot()
{
	this->clear();
	cvReleaseImage( &(this->Figure) );
}

//范型设计
template<class T>
void CPlot::plot(T *X, T *Y, size_t Cnt, CvScalar color, char type,bool is_need_lined)
{
	//对数据进行存储
	T tempX, tempY;
	vector<CvPoint2D64f>data;
	for(int i = 0; i < Cnt;i++)
	{
		tempX = X[i];
		tempY = Y[i];
		data.push_back( cvPoint2D64f((double)tempX, (double)tempY) );
	}
	this->dataset.push_back(data);
	LineType LT;
	LT.type = type;
	LT.color = color;
	LT.is_need_lined = is_need_lined;
	this->lineTypeSet.push_back(LT);
	
	//printf("data count:%d\n", this->dataset.size());
	
	this->DrawData(this->Figure); //每次都是重新绘制
}

template<class T>
void CPlot::plot(T *Y, size_t Cnt, CvScalar color, char type,bool is_need_lined)
{
	//对数据进行存储
	T tempX, tempY;
	vector<CvPoint2D64f>data;
	for(int i = 0; i < Cnt;i++)
	{
		tempX = i;
		tempY = Y[i];
		data.push_back(cvPoint2D64f((double)tempX, (double)tempY));
	}
	this->dataset.push_back(data);
	LineType LT;
	LT.type = type;
	LT.color = color;
	LT.is_need_lined = is_need_lined;
	this->lineTypeSet.push_back(LT);
	this->DrawData(this->Figure);
}

void CPlot::clear()
{
	this->dataset.clear();
	this->lineTypeSet.clear();
}

void CPlot::title(string title_name,CvScalar title_color = Scalar(0,0,0))
{
	int chw = 6, chh = 10; 
	IplImage *image = this->Figure;
	CvFont font;
	cvInitFont(&font,CV_FONT_HERSHEY_PLAIN,2,0.7, 0,1,CV_AA);
	int x = (this->window_width - 2 * this->border_size ) / 2 + this->border_size - ( title_name.size() / 2.0 ) * chw;
	int y = this->border_size / 2;
	cvPutText( image, title_name.c_str(), cvPoint( x, y), &font, title_color);
}

void CPlot::xlabel(string xlabel_name, CvScalar label_color = Scalar(0,0,0))
{
	int chw = 6, chh = 10; 
	int bs = this->border_size;		
	int h = this->window_height;
	int w = this->window_width;
	// let x, y axies cross at zero if possible.
	double y_ref = this->y_min;
	if ( (this->y_max > 0 ) && ( this->y_min <= 0 ) )
	{
		y_ref = 0;
	}
	int x_axis_pos = h - bs - cvRound((y_ref - this->y_min) * this->y_scale);
	CvFont font;
	cvInitFont(&font,CV_FONT_HERSHEY_PLAIN,1.5,0.7, 0,1,CV_AA);
	int x = this->window_width - this->border_size - chw * xlabel_name.size();
	int y = x_axis_pos + bs / 1.5;
	cvPutText(this->Figure, xlabel_name.c_str(), cvPoint( x, y), &font, label_color);
}
void CPlot::ylabel(string ylabel_name, CvScalar label_color = Scalar(0,0,0))
{
	CvFont font;
	cvInitFont(&font,CV_FONT_HERSHEY_PLAIN,1.5,0.7, 0,1,CV_AA);
	int x = this->border_size;
	int y = this->border_size;
	cvPutText(this->Figure, ylabel_name.c_str(), cvPoint( x, y), &font, label_color);
}

void CPlot::DrawAxis (IplImage *image)
{

	CvScalar axis_color = this->axis_color;
	
	int bs = this->border_size;		
	int h = this->window_height;
	int w = this->window_width;

	// size of graph
	int gh = h - bs * 2;
	int gw = w - bs * 2;

	// draw the horizontal and vertical axis
	// let x, y axies cross at zero if possible.
	double y_ref = this->y_min;
	if ( (this->y_max > 0 ) && ( this->y_min <= 0 ) )
	{
		y_ref = 0;
	}
	int x_axis_pos = h - bs - cvRound((y_ref - this->y_min) * this->y_scale);
	//X 轴
	cvLine(image, cvPoint(bs,     x_axis_pos), 
		           cvPoint(w - bs, x_axis_pos),
				   axis_color);
	//Y 轴
	cvLine(image, cvPoint(bs, h - bs), 
		           cvPoint(bs, h - bs - gh),
				   axis_color);

	// Write the scale of the y axis
	CvFont font;
	cvInitFont(&font,CV_FONT_HERSHEY_PLAIN,0.55,0.7, 0,1,CV_AA);

	int chw = 6, chh = 10; 
	char text[16];

	// y max
	if ( (this->y_max - y_ref) > 0.05 * (this->y_max - this->y_min) )
	{
		snprintf(text, sizeof(text)-1, "%.1f", this->y_max);
		cvPutText(image, text, cvPoint(bs, bs / 2), &font, this->text_color );
	}
	// y min
	if ( (y_ref - this->y_min) > 0.05 * (this->y_max - this->y_min) )
	{
		snprintf(text, sizeof(text)-1, "%.1f", this->y_min);
		cvPutText(image, text, cvPoint(bs, h - bs / 2), &font, this->text_color);
	}

	//画Y轴的刻度 每隔 scale_pixes 个像素
	//Y正半轴
	double y_scale_pixes = chh * 2;
	for (int i = 0; i < ceil( (x_axis_pos - bs) / y_scale_pixes ) + 1; i++)
	{
		snprintf(text, sizeof(text)-1, "%.1f", i * y_scale_pixes / this->y_scale );
		cvPutText(image, text, cvPoint(bs / 5, x_axis_pos - i * y_scale_pixes), &font, this->axis_color);
	}
	//Y负半轴
	for (int i = 1; i < ceil (( h - x_axis_pos - bs ) / y_scale_pixes ) + 1; i++)
	{
		snprintf(text, sizeof(text)-1, "%.1f", -i * y_scale_pixes / this->y_scale );
		cvPutText(image, text, cvPoint(bs / 5, x_axis_pos + i * y_scale_pixes), &font, this->axis_color);
	}

	// x_max
	snprintf(text, sizeof(text)-1, "%.1f", this->x_max );
	cvPutText(image, text, cvPoint(w - bs/2 - strlen(text) * chw, x_axis_pos), &font, this->text_color);

	// x min
	snprintf(text, sizeof(text)-1, "%.1f", this->x_min );
	cvPutText(image, text, cvPoint(bs, x_axis_pos ), &font, this->text_color);

	//画X轴的刻度 每隔 scale_pixes 个像素
	double x_scale_pixes = chw * 4;
	for (int i = 1; i < ceil( gw / x_scale_pixes ) + 1; i++)
	{
		snprintf(text, sizeof(text)-1, "%.0f", this->x_min + i * x_scale_pixes / this->x_scale );
		cvPutText(image, text, cvPoint(bs + i * x_scale_pixes - bs / 4, x_axis_pos + chh), &font, this->axis_color);
	}
}

//添加对线型的支持
//TODO线型未补充完整
//标记		线型
//l          直线	
//*          星 
//.          点 
//o          圈 
//x          叉 
//+          十字 
//s          方块 
void CPlot::DrawData (IplImage *image)
{
	
	//this->x_min = this->x_max = this->dataset[0][0].x;
	//this->y_min = this->y_max = this->dataset[0][0].y;
	
	int bs = this->border_size;
	for(size_t i = 0; i < this->dataset.size(); i++)
	{
		for(size_t j = 0; j < this->dataset[i].size(); j++)
		{
			if(this->dataset[i][j].x < this->x_min)
			{
				this->x_min = this->dataset[i][j].x;
			}else if(this->dataset[i][j].x > this->x_max)
			{
				this->x_max = this->dataset[i][j].x;
			}
		
			if(this->dataset[i][j].y < this->y_min)
			{
				this->y_min = this->dataset[i][j].y;
			}else if(this->dataset[i][j].y > this->y_max)
			{
				this->y_max = this->dataset[i][j].y;
			}
		}
	}
	double x_range = this->x_max - this->x_min;
	double y_range = this->y_max - this->y_min;
	this->x_scale = (image->width - bs*2)/ x_range;
	this->y_scale = (image->height- bs*2)/ y_range;
	
	
	//清屏
	memset(image->imageData, 255, sizeof(unsigned char)*Figure->widthStep*Figure->height);
	this->DrawAxis(image);
	
	//printf("x_range: %f y_range: %f\n", x_range, y_range);
	//绘制点
	double tempX, tempY;
	CvPoint prev_point, current_point;
	int radius = 3;
	int slope_radius = (int)( radius * 1.414 / 2 + 0.5);
	for(size_t i = 0; i < this->dataset.size(); i++)
	{
		for(size_t j = 0; j < this->dataset[i].size(); j++)
		{
			tempX = (int)((this->dataset[i][j].x - this->x_min)*this->x_scale);
			tempY = (int)((this->dataset[i][j].y - this->y_min)*this->y_scale);
			current_point = cvPoint(bs + tempX, image->height - (tempY + bs));
			
			if(this->lineTypeSet[i].type == 'l')
			{
				// draw a line between two points
				if (j >= 1)
				{
					cvLine(image, prev_point, current_point, lineTypeSet[i].color, 1, CV_AA);
				}		
				prev_point = current_point;
			}else if(this->lineTypeSet[i].type == '.')
			{
				cvCircle(image, current_point, 1, lineTypeSet[i].color, -1, 8);
				if (lineTypeSet[i].is_need_lined == true)
				{
					if (j >= 1)
					{
						cvLine(image, prev_point, current_point, lineTypeSet[i].color, 1, CV_AA);
					}		
					prev_point = current_point;
				}
			}else if(this->lineTypeSet[i].type == '*')
			{
				//画*
				cvLine(image, cvPoint(current_point.x - slope_radius, current_point.y - slope_radius), 
			    cvPoint(current_point.x + slope_radius, current_point.y + slope_radius), lineTypeSet[i].color, 1, 8);
					   
				cvLine(image, cvPoint(current_point.x - slope_radius, current_point.y + slope_radius), 
			    cvPoint(current_point.x + slope_radius, current_point.y - slope_radius), lineTypeSet[i].color, 1, 8);

				cvLine(image, cvPoint(current_point.x - radius, current_point.y), 
				cvPoint(current_point.x + radius, current_point.y), lineTypeSet[i].color, 1, 8);
					   
				cvLine(image, cvPoint(current_point.x, current_point.y - radius), 
			    cvPoint(current_point.x, current_point.y + radius), lineTypeSet[i].color, 1, 8);	 
				if (lineTypeSet[i].is_need_lined == true)
				{
					if (j >= 1)
					{
						cvLine(image, prev_point, current_point, lineTypeSet[i].color, 1, CV_AA);
					}		
					prev_point = current_point;
				}
				
			}else if(this->lineTypeSet[i].type == 'o')
			{
				cvCircle(image, current_point, radius, this->text_color, 1, CV_AA);
				if (lineTypeSet[i].is_need_lined == true)
				{
					if (j >= 1)
					{
						cvLine(image, prev_point, current_point, lineTypeSet[i].color, 1, CV_AA);
					}		
					prev_point = current_point;
				}
			}else if(this->lineTypeSet[i].type == 'x')
			{
				cvLine(image, cvPoint(current_point.x - slope_radius, current_point.y - slope_radius), 
			    cvPoint(current_point.x + slope_radius, current_point.y + slope_radius), lineTypeSet[i].color, 1, 8);
					   
				cvLine(image, cvPoint(current_point.x - slope_radius, current_point.y + slope_radius), 
			    cvPoint(current_point.x + slope_radius, current_point.y - slope_radius), lineTypeSet[i].color, 1, 8);
				if (lineTypeSet[i].is_need_lined == true)
				{
					if (j >= 1)
					{
						cvLine(image, prev_point, current_point, lineTypeSet[i].color, 1, CV_AA);
					}		
					prev_point = current_point;
				}
			}else if(this->lineTypeSet[i].type == '+')
			{
				cvLine(image, cvPoint(current_point.x - radius, current_point.y), 
				cvPoint(current_point.x + radius, current_point.y), lineTypeSet[i].color, 1, 8);
					   
				cvLine(image, cvPoint(current_point.x, current_point.y - radius), 
			    cvPoint(current_point.x, current_point.y + radius), lineTypeSet[i].color, 1, 8);
				if (lineTypeSet[i].is_need_lined == true)
				{
					if (j >= 1)
					{
						cvLine(image, prev_point, current_point, lineTypeSet[i].color, 1, CV_AA);
					}		
					prev_point = current_point;
				}
			}else if(this->lineTypeSet[i].type == 's')
			{
				cvRectangle(image, cvPoint(current_point.x - slope_radius, current_point.y - slope_radius), 
			    cvPoint(current_point.x + slope_radius, current_point.y + slope_radius), lineTypeSet[i].color, 1, 8);
				if (lineTypeSet[i].is_need_lined == true)
				{
					if (j >= 1)
					{
						cvLine(image, prev_point, current_point, lineTypeSet[i].color, 1, CV_AA);
					}		
					prev_point = current_point;
				}
			}

		}
	}	
}


/**
上面提供是符合C语言使用习惯的用法，下面提供C++类型，减少传入的参数
*/

class Plot : public CPlot
{
public:
	//重载这两个函数 传参简单
	template<class T>
	void plot( vector<T> Y,CvScalar color, char type = '*',bool is_need_lined = true);	
	template<class T>
	void plot(vector< Point_<T> > p,CvScalar color, char type = '*',bool is_need_lined = true);
	//增加一个函数把C版本的 IplImage 转换成Mat
	Mat figure()
	{
		return Mat(this->Figure);
	}
};



template<class T>
void Plot::plot(vector<T> Y, CvScalar color, char type,bool is_need_lined)
{
	//对数据进行存储
	T tempX, tempY;
	vector<CvPoint2D64f>data;
	for(int i = 0; i < Y.size();i++)
	{
		tempX = i;
		tempY = Y[i];
		data.push_back(cvPoint2D64f((double)tempX, (double)tempY));
	}
	this->dataset.push_back(data);
	LineType LT;
	LT.type = type;
	LT.color = color;
	LT.is_need_lined = is_need_lined;
	this->lineTypeSet.push_back(LT);
	this->DrawData(this->Figure);
}

template<class T>
void Plot::plot(vector< Point_<T> > p, CvScalar color, char type,bool is_need_lined)
{
	//对数据进行存储
	T tempX, tempY;
	vector<CvPoint2D64f>data;
	for(int i = 0; i < p.size();i++)
	{
		tempX = p[i].x;
		tempY = p[i].y;
		data.push_back( cvPoint2D64f((double)tempX, (double)tempY) );
	}
	this->dataset.push_back(data);
	LineType LT;
	LT.type = type;
	LT.color = color;
	LT.is_need_lined = is_need_lined;
	this->lineTypeSet.push_back(LT);
	
	//printf("data count:%d\n", this->dataset.size());
	
	this->DrawData(this->Figure); //每次都是重新绘制
}

int test_plot()
{
	const int Cnt = 80;
	double X[Cnt] = {0};
	double Y[Cnt] = {0};
	for(int i = 0; i < Cnt; i++)
	{
		X[i] = (double)i;
		Y[i] = (double)i - 20;
	}
	
	CPlot plot;
	plot.x_max = 100; //可以设定横纵坐标的最大，最小值
	plot.x_min = -20;
	plot.y_max = 400;
	plot.y_min = -200;
	plot.axis_color = Scalar(0,255,0);
	plot.text_color = Scalar(255,0,255);
	plot.plot(Y, Cnt, CV_RGB(0, 0, 0)); //可以只传入Y值 X默认从0开始 
	plot.title("test plot"); //可以设定标题 只能是英文 中文会乱码 有解决方案，但是很麻烦
	plot.xlabel("X",Scalar(255,255,0));
	plot.ylabel("Y",Scalar(255,255,0));
	cvShowImage("0", plot.Figure);

	//如何在一幅图中绘制多组数据？每次绘制的同时还对数据进行存储？
	for(int i = 0; i < Cnt; i++)
	{
		X[i] = (double)i;
		Y[i] = (double)(5*i - 20);
	}
	plot.plot(X, Y, Cnt, CV_RGB(0, 255, 180), '.',true);//依次传入的参数是 X轴数据； Y轴数据； 数据长度； 线条颜色；点描绘类型（默认是'*'）；点与点之间是否需要连接（默认连接） 
    cvNamedWindow("1",0);  
    cvShowImage("1", plot.Figure);
	
	for(int i = 0; i < Cnt; i++)
	{
		X[i] = (double)i;
		Y[i] = (double)(-5*i - 20);
	}
	plot.plot(X, Y, Cnt, CV_RGB(0,255, 0), 's');
	cvShowImage("2", plot.Figure);
	
	for(int i = 0; i < Cnt; i++)
	{
		X[i] = (double)i;
		Y[i] = (double)(50*sin(i*0.1));
	}
	plot.plot(X, Y, Cnt, CV_RGB(0, 0, 255), 'x');
	cvShowImage("3", plot.Figure);

	plot.clear();//清出前面的存储的数据


	for(int i = 0; i < Cnt; i++)
	{
		X[i] = (double)i;
		Y[i] = (double)(100*sin(i*0.1));
	}

	plot.plot(X, Y, Cnt, CV_RGB(255, 255, 0), 'o',false);
	cvShowImage("4", plot.Figure);


	int X2[Cnt] = {0};
	int Y2[Cnt] = {0};
	for(int i = 0; i < Cnt; i++)
	{
		X2[i] = i;
		Y2[i] = -30*i - 20;
	}
	plot.plot(X2, Y2, Cnt/2, CV_RGB(0, 255, 255),'*',false); //默认会把点与点之间连接在一起
	cvShowImage("5", plot.Figure);

	//C++ 扩展
	Plot p;
	vector<Point2f> points;
	vector<int> Y_points;
	Point2f p_temp;
	for(int i = 0; i < Cnt; i++)
	{
		p_temp.x = i;
		p_temp.y= -10*i - 20;
		points.push_back(p_temp);
		Y_points.push_back(i * 5 + 1);
	}
	p.plot(Y_points,Scalar(255,255,0));
	imshow("6",p.figure());
	p.plot(points,Scalar(255,0,0),'+',false);
	imshow("7",p.figure());

	return 0;
}

