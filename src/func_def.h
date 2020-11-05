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
int matplot_test(); 
std::vector<cv::Mat> split_hdr_dol_encoder_raw(char *name,int frame_num,int height,int offset1,int offset2,int offset3,raw_type_file_dscr_t *raw_dscr);
int dump_raw_bye(char *name,cv::Mat img,bayer_format_t bayer,raw_type_file_dscr_t *raw_dscr);
std::vector<cv::Mat> split_hdr_dol_dng_raw(char *name,int frame_num,int height,int offset1,int offset2,int offset3,raw_type_file_dscr_t *raw_dscr);
cv::Mat isp_cfa_3x3_region(cv::Mat in_img,int format);
Mat drc_hdr_img_kang2014_bilateral(Mat hdr_img);
int save_mat_to_bin(char *picname,char *func_name,Mat img);
Mat drc_hdr_img_kang2014_local_bilateral(Mat hdr_img);
Mat   process_blc_sample(Mat in_image,u_int16_t blc);
cv::Mat  convert_raw_to_hdr_encoder_frame(cv::Mat raw_imge,raw_type_file_dscr_t *raw_dscr);
int dump_raw_bye_to_text_hex_format(char *name,cv::Mat img,bayer_format_t bayer,raw_type_file_dscr_t *raw_dscr);
#endif

