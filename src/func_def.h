#ifndef __FUNC_DEF__
#define __FUNC_DEF__
#include "isp_pipeline.h"
#include "plot.h"
int plot_test();
cv::Mat bm3d_test(char *infile,float sigma,int templateWindowSize,int searchWindowSize);
char *load_yuv3p4b_10bit_img(char *name,int w,int h, int stride_w,char *format);
vector<cv::Mat> convert_yuv3p4b_10bit_420sp_bin_to_yuv1p2b_420sp(char *buffer,int w,int h,int stride_w);
vector<cv::Mat> convert_yuv3p4b_10bit_420sp_bin_to_yuv1p2b_420sp_2(char *buffer,int w,int h,int stride_w);
int save_yuv4201p2b_to_bin(char *name,vector<cv::Mat> yuv_image);
int save_yuv4201p2b_to_yuv420_8bit_bin(char *name,vector<cv::Mat> yuv_image);
vector<cv::Mat> convert_yuv3p4b_10bit_420simi_bin_to_yuv1p2b_420sp(char *buffer,int w,int h,int stride_w);
vector<cv::Mat> convert_yuv3p4b_10bit_422simi_bin_to_yuv1p2b_422sp(char *buffer,int w,int h,int stride_w);
int save_yuv4221p2b_to_yuv422_8bit_bin(char *name,vector<cv::Mat> yuv_image);
int save_img(char *picname,char *func_name,Mat img);
#endif

