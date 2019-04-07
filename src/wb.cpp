#include "isp_pipeline.h"

Mat isp_wb(Mat in_bayer_img,float r_gain,float g_gain,float b_gain,bayer_format_t format)
{   
  raw_ch_t ch=split_raw_ch(in_bayer_img,format);
  Mat r_ch=get_R_raw(ch,format);
  Mat gr_ch=get_Gr_raw(ch,format);
  Mat gb_ch=get_Gb_raw(ch,format);
  Mat b_ch=get_B_raw(ch,format);
  Mat out_bayer_img(in_bayer_img.rows(),in_bayer_img.cols(),in_bayer_img.type());
  int i=0,j=0;
  float value;
  for(i=0;i<r_ch.rows();i++)
  {
    for(j=0;j<r_ch.cols();i++)
    {
       value=r_ch.at<u_int16_t>(i,j)*r_gain;
       if(value>0xffff)
        value=0xfffff;
       r_ch.at<u_int16_t>(i,j)=value;
       
       value=gr_ch.at<u_int16_t>(i,j)*g_gain;
       if(value>0xffff)
        value=0xfffff;
       gr_ch.at<u_int16_t>(i,j)=value;

       value=gb_ch.at<u_int16_t>(i,j)*g_gain;
       if(value>0xffff)
        value=0xfffff;
       gb_ch.at<u_int16_t>(i,j)=value;

       value=b_ch.at<u_int16_t>(i,j)*r_gain;
       if(value>0xffff)
        value=0xfffff;
       b_ch.at<u_int16_t>(i,j)=value;
    }
  }
  merge_raw_ch(ch,format,out_bayer_img);
  return out_bayer_img;
}

