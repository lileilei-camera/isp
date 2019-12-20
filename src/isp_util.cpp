#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include "isp_log.h"
#include "isp_util.h"
#include "isp_pipeline.h"

int save_isp_pra(void *buf,int size)
{
  int fd=open("isp_pra.bin",O_RDWR|O_CREAT);
  if(fd<0)
  {
    log_err("open file failed");
	return fd;
  }
  write(fd,buf,size);
  log_info("save success");
  close(fd);
  return 0;
}

int load_isp_pra(void *buf,int size)
{
  int fd=open("isp_pra.bin",O_RDONLY);
  if(fd<0)
  {
    log_err("open file failed,please firset gen the data");
	return fd;
  }
  read(fd,buf,size);
  close(fd);
  return 0;
}


int save_bin(char *name,void *buf,int size)
{
  int fd=open(name,O_RDWR|O_CREAT);
  if(fd<0)
  {
    log_err("open file failed");
	return fd;
  }
  write(fd,buf,size);
  log_info("save success");
  close(fd);
  return 0;
}

int load_bin(char *name,void *buf,int size)
{
  int fd=open(name,O_RDONLY);
  if(fd<0)
  {
    log_err("open file failed,please firset gen the data");
	return fd;
  }
  read(fd,buf,size);
  close(fd);
  return 0;
}


int plot_hist(char *pic_name,const char *hist_name,Mat img,float range_st, float range_end)
{
    cvNamedWindow(pic_name,0);  
	imshow(pic_name, img);
	int bins = 256;
	int hist_size[] = { bins };
	float range[] = {range_st,range_end};
	const float* ranges[] = { range };
	MatND redhist;
	int channels_r[] = { 0 };
	
	calcHist(&img, 1, channels_r, Mat(),
		redhist, 1, hist_size, ranges,
		true, false);
	double maxvalue_red;
	minMaxIdx(redhist, 0, &maxvalue_red, 0, 0);
	int scale = 1;
	int hisheight = 256;
	Mat hisimage = Mat::zeros(hisheight, bins, CV_8UC3);
	 
	//正式开始绘制
	for (int i = 1; i < bins; i++)
	{
		//参数准备
		float binvalue_red = redhist.at<float>(i);

		int intensity_red =
			cvRound(binvalue_red*hisheight / maxvalue_red);  //要绘制的高度

		//绘制红色分量的直方图
		rectangle(hisimage, Point(i*scale, hisheight - 1), Point((i + 1)*scale - 1, hisheight - intensity_red), Scalar(128, 128, 128));
	}

	//在窗口中显示出绘制好的直方图	
    char *f_name=(char *)malloc(128);    
    sprintf(f_name,"%s.%s.hist.png",pic_name,hist_name);    
    cvNamedWindow(f_name,0);  
	imshow(f_name,hisimage);
    vector<int> compression_params;
    compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION); 
    compression_params.push_back(0);
    log_info("name : %s",f_name);
    cv::imwrite(f_name,hisimage,compression_params); 
    free(f_name);
	return 0;
}

int show_and_save_img(char *picname,char *func_name,Mat img)
{
   char *f_name=(char *)malloc(128);     
   sprintf(f_name,"%s_%s.png",picname,func_name);    
   cvNamedWindow(f_name,0);  
   imshow(f_name, img);   
   vector<int> compression_params;
   compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION); 
   compression_params.push_back(0);   
   cv::imwrite(f_name,img,compression_params);    
   free(f_name);
}

Mat get_float_u16_img(Mat img)
{
   Mat result(img.rows,img.cols,CV_16UC1);
   int i=0,j=0;
   for(i=0;i<img.rows;i++)
   {
      for(j=0;j<img.cols;j++)
      {
         if(img.at<float>(i,j)>0)
         {
             result.at<u_int16_t>(i,j)=(u_int16_t)img.at<float>(i,j);
         }else
         {
             result.at<u_int16_t>(i,j)=(u_int16_t)(-img.at<float>(i,j));
         }
      }
   }
   return result;
}

Mat get_u16_float_img(Mat img)
{
   Mat result(img.rows,img.cols,CV_32FC1);
   int i=0,j=0;
   for(i=0;i<img.rows;i++)
   {
      for(j=0;j<img.cols;j++)
      {
         result.at<float>(i,j)=(float)img.at<u_int16_t>(i,j);
      }
   }
   return result;
}

Mat get_sub_wave_img(Mat img,int sub)
{
    int sub_rows=img.rows/2;
    int sub_cols=img.cols/2;    
    Mat result(sub_rows,sub_cols,CV_32FC1);
    int i=0,j=0;
    int row_offset;
    int col_offset;
    if(sub==0)
    {
       row_offset=0;
       col_offset=0;
    }else if(sub==1)
    {       
       row_offset=0;
       col_offset=sub_cols;
    }else if(sub==2)
    {       
       row_offset=sub_rows;
       col_offset=0;
    }else if(sub==3)
    {    
       row_offset=sub_rows;
       col_offset=sub_cols;
    }else
    {
       log_err("err");
       return result;
    }
    
    for(i=0;i<sub_rows;i++)
    {
      for(j=0;j<sub_cols;j++)
      {
         result.at<float>(i,j)=img.at<float>(i+row_offset,j+col_offset);
      }
    }
}


