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


