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

Mat merge_hdr_img_kang2014(Mat img1, Mat img2,Mat img3,Mat img4,float exp_ration[],hdr_merge_pra_kang2014_t *pra)
{
   float w1_r,w2_r,w3_r,w4_r;
   float u1_h,u2_h,u3_h;   
   float u2_l,u3_l,u4_l;
   float deta_t1=1;
   float deta_t2=deta_t1/exp_ration[0];   
   float deta_t3=deta_t2/exp_ration[1];
   float deta_t4=deta_t3/exp_ration[2];
   float c;
   float gamma1,gamma2,gamma3;
   log_info("rou=%f beta=%f deta=%f",pra->rou,pra->beta,pra->deta);
   log_info("deta_t (%f %f %f %f)",deta_t1,deta_t2,deta_t3,deta_t4);
   gamma1=(pra->rou*(0xffff-pra->beta)-pra->beta*(0xffff-pra->rou))/(deta_t1*pra->rou+deta_t2*(0xffff-pra->rou));   
   gamma2=(pra->rou*(0xffff-pra->beta)-pra->beta*(0xffff-pra->rou))/(deta_t2*pra->rou+deta_t3*(0xffff-pra->rou));
   gamma3=(pra->rou*(0xffff-pra->beta)-pra->beta*(0xffff-pra->rou))/(deta_t3*pra->rou+deta_t4*(0xffff-pra->rou));   
   log_info("gamma1=%f gamma2=%f gamma3=%f",gamma1,gamma2,gamma3);
   u1_h=pra->rou/deta_t1;
   c=-log10(pra->deta)/((gamma1-u1_h)*(gamma1-u1_h));
   u2_h=gamma2-sqrt(-log10(pra->deta)/c);
   u3_h=gamma3-sqrt(-log10(pra->deta)/c);
   u2_l=2*gamma1-u1_h;   
   u3_l=2*gamma2-u2_h;
   u4_l=2*gamma3-u3_h;   

   log_info("u_h(%f %f %f) u_l(%f %f %f) c=%f",u1_h,u2_h,u3_h,u2_l,u3_l,u4_l,c*1000000);
   
   Mat merge_img(img1.rows,img1.cols,CV_32FC1); 
   int i,j;
   float i1_r,i2_r,i3_r,i4_r;
   for(i=0;i<img1.rows;i++)
   {
        for(j=0;j<img1.cols;j++)
        {
            i1_r=(float)(img1.at<u_int16_t>(i,j)-pra->beta)/deta_t1;
            if(i1_r<u1_h){
               w1_r=1;
            }else
            {
               w1_r=exp(-c*(i1_r-u1_h)*(i1_r-u1_h));
            }
            i2_r=(float)(img2.at<u_int16_t>(i,j)-pra->beta)/deta_t2;
            if(i2_r<u2_l)
            {
               w2_r=exp(-c*(i2_r-u2_l)*(i2_r-u2_l));
            }else if(i2_r>u2_l && i2_r < u2_h)
            {
               w2_r=1; 
            }else
            {
                w2_r=exp(-c*(i2_r-u2_h)*(i2_r-u2_h));
            }
            i3_r=(float)(img3.at<u_int16_t>(i,j)-pra->beta)/deta_t3;
            if(i3_r<u3_l)
            {
               w3_r=exp(-c*(i3_r-u3_l)*(i3_r-u3_l));
            }else if(i3_r>u3_l && i3_r < u3_h)
            {
               w3_r=1; 
            }else
            {
                w3_r=exp(-c*(i3_r-u3_h)*(i3_r-u3_h));
            }
            i4_r=(float)(img4.at<u_int16_t>(i,j)-pra->beta)/deta_t4;
            if(i4_r<u4_l){
                w4_r=exp(-c*(i4_r-u4_l)*(i4_r-u4_l));               
            }else
            {
               w4_r=1;
            }
            merge_img.at<float>(i,j)=(w1_r*i1_r+w2_r*i2_r+w3_r*i3_r+w4_r*i4_r)/(w1_r+w2_r+w3_r+w4_r);   
        }
   }
   return merge_img;
}
Mat drc_hdr_img_kang2014_linear(Mat hdr_img)
{   
   Mat img(hdr_img.rows,hdr_img.cols,CV_16UC1); 
   int i=0,j=0;   
   float y_out=0;
   for(i=0;i<hdr_img.rows;i++)
   {
       for(j=0;j<hdr_img.cols;j++)
       {
           y_out=hdr_img.at<float>(i,j)/4096;
           img.at<u_int16_t>(i,j)=(u_int16_t)y_out;
       }
    }
   return img;
}
Mat drc_hdr_img_kang2014_gamma(Mat hdr_img,float gamma)
{   
   Mat img(hdr_img.rows,hdr_img.cols,CV_16UC1); 
   int i=0,j=0;   
   float y_out=0;
   //y_out=x^(1/gamma)  x>=0 && x<=1
   for(i=0;i<hdr_img.rows;i++)
   {
       for(j=0;j<hdr_img.cols;j++)
       {
           y_out=pow(hdr_img.at<float>(i,j)/(0xffff*4096),(1/gamma))*0xffff;
           img.at<u_int16_t>(i,j)=(u_int16_t)y_out;
       }
    }
    return img;
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
