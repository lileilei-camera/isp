#ifndef __ISP_PIPELINE_H__
#define __ISP_PIPELINE_H__
#include "cv.h"
#include "highgui.h"
#include <stdio.h>
#include <stdlib.h>
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
  u_int16_t r_blc;
  u_int16_t gr_blc;
  u_int16_t gb_blc;
  u_int16_t b_blc;
}blc_pra_t;

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
raw_ch_t process_blc(raw_ch_t,blc_pra_t blc_pra,bayer_format_t bayer);
#endif

