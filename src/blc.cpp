#include "cv.h"
#include "highgui.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "isp_pipeline.h"
#include "isp_log.h"

blc_pra_t get_blc_value(raw_ch_t ch,bayer_format_t bayer)
{  
  IplImage imge;
  blc_pra_t blc_pra;
  cv::Mat ch_img;
  CvScalar mean;
  ch_img=get_R_raw(ch,bayer);
  imge=ch_img;
  mean=cvAvg(&imge,NULL);
  blc_pra.r_blc=mean.val[0];
  ch_img=get_B_raw(ch,bayer);
  imge=ch_img;
  mean=cvAvg(&imge,NULL);
  blc_pra.b_blc=mean.val[0];
  ch_img=get_Gr_raw(ch,bayer);
  imge=ch_img;
  mean=cvAvg(&imge,NULL);
  blc_pra.gr_blc=mean.val[0];
  ch_img=get_Gb_raw(ch,bayer);
  imge=ch_img;
  mean=cvAvg(&imge,NULL);
  blc_pra.gb_blc=mean.val[0];
  log_info("r=%d gr=%d rb=%d b=%d",blc_pra.r_blc,blc_pra.gr_blc,blc_pra.gb_blc,blc_pra.b_blc);
  return blc_pra;   
}

raw_ch_t  process_blc(raw_ch_t ch,blc_pra_t blc_pra,bayer_format_t bayer)
{
  int i=0,j=0;
  int row=ch.raw_ch_00.rows;
  int col=ch.raw_ch_00.cols;
  int tmp=0;
  int r_gain,gr_gain,gb_gain,b_gain;
  cv::Mat ch_img_r,ch_img_gr,ch_img_gb,ch_img_b;
  ch_img_r=get_R_raw(ch,bayer);
  ch_img_gr=get_Gr_raw(ch,bayer);
  ch_img_gb=get_Gb_raw(ch,bayer);
  ch_img_b=get_B_raw(ch,bayer);
  r_gain=(int)((float)0xffff/(float)(0xffff-blc_pra.r_blc)*1024);  
  gr_gain=(int)((float)0xffff/(float)(0xffff-blc_pra.gr_blc)*1024);
  gb_gain=(int)((float)0xffff/(float)(0xffff-blc_pra.gb_blc)*1024);
  b_gain=(int)((float)0xffff/(float)(0xffff-blc_pra.b_blc)*1024);

  for(i=0;i<row;i++)
  {
    for(j=0;j<col;j++)
    {    
       tmp=ch_img_r.at<u_int16_t>(i,j)-blc_pra.r_blc;
       tmp=tmp*r_gain/1024;
       ch_img_r.at<u_int16_t>(i,j)=(u_int16_t)tmp;  
       
       tmp=ch_img_gr.at<u_int16_t>(i,j)-blc_pra.gr_blc;
       tmp=tmp*gr_gain/1024;
       ch_img_gr.at<u_int16_t>(i,j)=(u_int16_t)tmp;      
   
       tmp=ch_img_gb.at<u_int16_t>(i,j)-blc_pra.gb_blc;
       tmp=tmp*gb_gain/1024;
       ch_img_gb.at<u_int16_t>(i,j)=(u_int16_t)tmp;      

       tmp=ch_img_b.at<u_int16_t>(i,j)-blc_pra.b_blc;
       tmp=tmp*b_gain/1024;
       ch_img_b.at<u_int16_t>(i,j)=(u_int16_t)tmp;      
    }
  }
  return ch;
}
Mat   process_blc_sample(Mat in_image,u_int16_t blc)
{
   for(int i=0;i<in_image.rows;i++)
   {
       for(int j=0;j<in_image.cols;j++)
       {
           if(in_image.at<u_int16_t>(i,j)>=blc){
              in_image.at<u_int16_t>(i,j)=in_image.at<u_int16_t>(i,j)-blc;
           }else
           {
               in_image.at<u_int16_t>(i,j)=0;
           }
       }
   }
   return in_image;
}
