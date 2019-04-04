#include "isp_pipeline.h"
Mat get_long_exp_img(Mat img,int height,int long_offset)
{
    int i=0,j=0;
    Mat long_img(height,img.cols,CV_16UC1);
    for(i=0;i<height;i++)
    {
       for(j=0;j<img.cols;j++){
          long_img.at<u_int16_t>(i,j)=img.at<u_int16_t>(long_offset+2*i,j);
       }
    }
    return long_img;
}
Mat get_short_exp_img(Mat img,int height,int short_offset)
{
    int i=0,j=0;
    Mat short_img(height,img.cols,CV_16UC1);
    for(i=0;i<height;i++)
    {
       for(j=0;j<img.cols;j++){
          short_img.at<u_int16_t>(i,j)=img.at<u_int16_t>(short_offset+2*i,j);
       }
    }
    return short_img;
}
Mat merge_hdr_img(Mat long_img, Mat short_img,float exp_ration,hdr_merge_pra_t *pra)
{   
   int i=0,j=0;
   int max_val=0xffff*exp_ration;
   float w_th;
   float long_val=0;
   float short_val=0;
   Mat merge_img(long_img.rows,long_img.cols,CV_32FC1);
   for(i=0;i<long_img.rows;i++)
   {
        for(j=0;j<long_img.cols;j++)
        {
            long_val=long_img.at<u_int16_t>(i,j);
            if(long_val>pra->long_exp_th)
            {
                w_th=0;
            }else if(long_val<pra->long_exp_th_low)
            {
                w_th=1; 
            }else
            {
                w_th=(long_val-pra->long_exp_th_low)/(pra->long_exp_th-pra->long_exp_th_low);
                w_th=1-w_th;
            }
            short_val=(float)short_img.at<u_int16_t>(i,j)*exp_ration;
            if(short_val>=max_val)
            {
                short_val=max_val;
            }
            merge_img.at<float>(i,j)=long_val*w_th+short_val*(1-w_th);
        }
   }
   return merge_img;
}

Mat drc_hdr_img(Mat hdr_img,float exp_ration,drc_pra_t *pra)
{    
    float max_val=0xffff*exp_ration;    
    Mat img(hdr_img.rows,hdr_img.cols,CV_16UC1);    
    Mat gain(hdr_img.rows,hdr_img.cols,CV_32FC1);    
    int i=0,j=0;
    float y_out=0;
    float y_in=0;
    float min_gain=1000000000;
    if(pra->methed==DRC_GAMMA){
       //y_out=x^(1/gamma)  x>=0 && x<=1
       for(i=0;i<hdr_img.rows;i++)
       {
           for(j=0;j<hdr_img.cols;j++)
           {
               y_out=pow(hdr_img.at<float>(i,j)/max_val,(1/pra->gamma))*0xffff;
               img.at<u_int16_t>(i,j)=(u_int16_t)y_out;
           }
        }
    }else if(DRC_GTM1==pra->methed)
    {
       //y=k1*x*e^(-k2*(x^k3))
       for(i=0;i<hdr_img.rows;i++)
       {
           for(j=0;j<hdr_img.cols;j++)
           {
               y_in=hdr_img.at<float>(i,j)/max_val;
               y_out=pra->k1*y_in*exp(-pra->k2*pow(y_in,pra->k3))*0xffff;   
               img.at<u_int16_t>(i,j)=y_out;
           }
        }
    }
    return img;
}
