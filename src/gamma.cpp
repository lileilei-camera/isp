/*
    y_out=x^(1/gamma)  x>=0 && x<=1
*/
#include "isp_pipeline.h"

Mat isp_gamma(Mat in_img,float gamma)
{    
    float max_val=0xffff;
    float y_out;
    if(in_img.type()!=CV_16UC3)
    {
       Mat null_mat;
       log_err("err type, gamma process only rgb format");
       return null_mat;
    }
    CvScalar s_in;
    CvScalar s_out;
    Mat img(in_img.rows,in_img.cols,CV_16UC3);    
    int i=0,j=0;
    //y_out=x^(1/gamma)  x>=0 && x<=1
    for(i=0;i<in_img.rows;i++)
    {
       for(j=0;j<in_img.cols;j++)
       {
           s_in= cvGet2D(in_img, i, j);
           s_out.val[0]=pow(s_in.val[0]/max_val,(1/gamma))*0xffff;
           s_out.val[1]=pow(s_in.val[1]/max_val,(1/gamma))*0xffff;
           s_out.val[2]=pow(s_in.val[2]/max_val,(1/gamma))*0xffff;
           cvSet2D(img,i,j,s_out);
       }
    }
    return img;
}