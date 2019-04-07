/*
    y_out=x^(1/gamma)  x>=0 && x<=1
*/
#include "isp_pipeline.h"

Mat isp_gamma(Mat hdr_img,float gamma)
{    
    float max_val=0xffff;    
    Mat img(hdr_img.rows,hdr_img.cols,CV_16UC1);    
    Mat gain(hdr_img.rows,hdr_img.cols,CV_32FC1);    
    int i=0,j=0;
    float y_out=0;
    float min_gain=1000000000;
    //y_out=x^(1/gamma)  x>=0 && x<=1
    for(i=0;i<hdr_img.rows;i++)
    {
       for(j=0;j<hdr_img.cols;j++)
       {
           y_out=pow((float)hdr_img.at<u_int16_t>(i,j)/max_val,(1/gamma))*0xffff;
           img.at<u_int16_t>(i,j)=(u_int16_t)y_out;
       }
    }
    return img;
}