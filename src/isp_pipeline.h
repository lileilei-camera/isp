#ifndef __ISP_PIPELINE_H__
#define __ISP_PIPELINE_H__
#include "cv.h"
#include "highgui.h"
#include <stdio.h>
#include <stdlib.h>
#include "highgui.h"
#include <fcntl.h>
#include "isp_log.h"
#include "string.h"
#include "isp_util.h"
#include <unistd.h>
#include<fstream>
#include<iostream>

using namespace cv;

#define ALIGN_TO(size,n) ((size+(n-1))&(~(n-1)))
#define RAW10_PIX0_LOW_MASK (0X3<<0)
#define RAW10_PIX1_LOW_MASK (0X3<<2)
#define RAW10_PIX2_LOW_MASK (0X3<<4)
#define RAW10_PIX3_LOW_MASK (0X3<<6)

#define GET_RAW10_PIX0_LOW(value) (((value)&RAW10_PIX0_LOW_MASK)>>0)
#define GET_RAW10_PIX1_LOW(value) (((value)&RAW10_PIX1_LOW_MASK)>>2)
#define GET_RAW10_PIX2_LOW(value) (((value)&RAW10_PIX2_LOW_MASK)>>4)
#define GET_RAW10_PIX3_LOW(value) (((value)&RAW10_PIX3_LOW_MASK)>>6)

#define SET_RAW10_PIX_LOW_MASK (0X3<<0)
#define SET_RAW10_PIX_HIGHT_MASK (0XFF<<2)

#define SET_RAW10_PIX_HIGH(set, value) ((set)=((value)<<2))
#define SET_RAW10_PIX_LOW(set, value) (((value)<<0)|((set)&(~SET_RAW10_PIX_LOW_MASK)))

#define LSC_MESH_GRID_WIDHT 64
#define LSC_MESH_GRID_HEIGHT 48

typedef enum
{
  CV_BayerBG,
  CV_BayerGB,
  CV_BayerRG,
  CV_BayerGR,
}bayer_format_t;

typedef struct
{
 int width;
 int hegiht;
 int bitwidth;
 int is_packed;
 int width_algin;
 int height_algin;
 int line_length_algin;
 bayer_format_t bayer_format;
}raw_type_file_dscr_t;

typedef struct
{
 cv::Mat raw_ch_00;
 cv::Mat raw_ch_01;
 cv::Mat raw_ch_10;
 cv::Mat raw_ch_11;
  
}raw_ch_t;

typedef struct
{
  u_int32_t  bins;  
  u_int32_t  sum;//cols * rows
  u_int32_t  *r_hist;
  u_int32_t  *gr_hist;
  u_int32_t  *gb_hist;
  u_int32_t  *b_hist;
}raw_hist_t;

typedef struct
{
  u_int16_t r_blc;
  u_int16_t gr_blc;
  u_int16_t gb_blc;
  u_int16_t b_blc;
}blc_pra_t;

typedef struct
{
   float rr,rg,rb;
   float gr,gg,gb;
   float br,bg,bb;
}ccm_t;


typedef struct{

}wave_denoise_pra_t;

typedef struct {
	int long_exp_th; //below this ,we will use the long exp
	int long_exp_th_low;
}hdr_merge_pra_t;

typedef struct {
   float rou;
   float beta;
   float deta;
}hdr_merge_pra_kang2014_t;


enum{
	DRC_GAMMA,
    DRC_GTM1,
};

typedef struct {
	int methed;
	//for DRC_GAMMA
	float gamma;
	//for DRC_GTM1
	float k1;
	float k2;
	float k3;
}drc_pra_t;


typedef struct
{
  float r_gain[LSC_MESH_GRID_HEIGHT][LSC_MESH_GRID_WIDHT];
  float gr_gain[LSC_MESH_GRID_HEIGHT][LSC_MESH_GRID_WIDHT];
  float gb_gain[LSC_MESH_GRID_HEIGHT][LSC_MESH_GRID_WIDHT];
  float b_gain[LSC_MESH_GRID_HEIGHT][LSC_MESH_GRID_WIDHT];
}lsc_pra_t;


typedef struct
{
  blc_pra_t blc_pra;
  lsc_pra_t lsc_pra;
}isp_pra_t;

cv::Mat fetch_raw(char *name,raw_type_file_dscr_t *raw_dscr);
int dump_raw_to_png(char *name,cv::Mat,bayer_format_t bayer);
int show_raw_image(char *name,cv::Mat,bayer_format_t bayer);
raw_ch_t split_raw_ch(cv::Mat img,bayer_format_t bayer);
int merge_raw_ch(raw_ch_t raw_ch,bayer_format_t bayer,cv::Mat img);
cv::Mat get_Gr_raw(raw_ch_t ch,bayer_format_t bayer);
cv::Mat get_Gb_raw(raw_ch_t ch,bayer_format_t bayer);
cv::Mat get_R_raw(raw_ch_t ch,bayer_format_t bayer);
cv::Mat get_B_raw(raw_ch_t ch,bayer_format_t bayer);
blc_pra_t get_blc_value(raw_ch_t ch,bayer_format_t bayer);
raw_hist_t get_raw_hist(int bins,raw_ch_t raw_ch,bayer_format_t bayer);
int release_raw_hist(raw_hist_t *raw_hist);
raw_ch_t process_blc(raw_ch_t,blc_pra_t blc_pra,bayer_format_t bayer);
int log_raw_hist(raw_hist_t *raw_hist);
int save_raw_hist(char *name,raw_hist_t *raw_hist);
int plot_hist(char *pic_name,const char *hist_name,Mat img,float range_st, float range_end);
int show_and_save_img(char *picname,char *func_name,Mat img);
Mat get_float_u16_img(Mat img);
Mat get_u16_float_img(Mat img);
Mat haar_wavelet_decompose(Mat input_img,int layers,int is_multi=0);
Mat get_sub_wave_img(Mat img,int sub);
Mat get_long_exp_img(Mat img,int height,int long_offset);
Mat get_short_exp_img(Mat img,int height,int short_offset);
Mat merge_hdr_img(Mat long_img, Mat short_img,float exp_ration,hdr_merge_pra_t *pra);
Mat drc_hdr_img(Mat hdr_img,float exp_ration,drc_pra_t *pra);
Mat merge_hdr_img_kang2014(Mat img1, Mat img2,Mat img3,Mat img4,float exp_ration[],hdr_merge_pra_kang2014_t *pra);
Mat drc_hdr_img_kang2014_linear(Mat hdr_img);
Mat drc_hdr_img_kang2014_gamma(Mat hdr_img,float gamma);
cv::Mat demosic_raw_image(cv::Mat img,bayer_format_t bayer);
int dump_to_exr(char *name,char *sub_name,cv::Mat img);
int save_yuv_img(char *picname,char *func_name,Mat img);
#endif

