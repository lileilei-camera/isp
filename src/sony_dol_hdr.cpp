#include "isp_pipeline.h"

static int drc_exp_bit=16;
#define line_bits 0xffff

int save_mat_to_bin(char *picname,char *func_name,Mat img);
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
//image is long start
Mat merge_hdr_img_kang2014(Mat img1, Mat img2,Mat img3,Mat img4,float exp_ration[],hdr_merge_pra_kang2014_t *pra)
{
   float w1,w2,w3,w4;
   float u1_h,u2_h,u3_h;   
   float u2_l,u3_l,u4_l;
   float deta_t1=1;
   float deta_t2=deta_t1/exp_ration[0];   
   float deta_t3=deta_t2/exp_ration[1];
   float deta_t4=deta_t3/exp_ration[2];
   float c;
   float gamma1,gamma2,gamma3;

   if(pra->frame_num==2)
   {
      drc_exp_bit=exp_ration[0];
   }else if(pra->frame_num==3)
   {
      drc_exp_bit=exp_ration[0]*exp_ration[1];
   }	else 
   {
       drc_exp_bit=exp_ration[0]*exp_ration[1]*exp_ration[2];
   }	
   
   log_info("rou=%f beta=%f deta=%f drc_exp_bit=%d",pra->rou,pra->beta,pra->deta,drc_exp_bit);
   log_info("deta_t (%f %f %f %f)",deta_t1,deta_t2,deta_t3,deta_t4);
   gamma1=(pra->rou*(line_bits-pra->beta)-pra->beta*(line_bits-pra->rou))/(deta_t1*pra->rou+deta_t2*(line_bits-pra->rou));   
   gamma2=(pra->rou*(line_bits-pra->beta)-pra->beta*(line_bits-pra->rou))/(deta_t2*pra->rou+deta_t3*(line_bits-pra->rou));
   gamma3=(pra->rou*(line_bits-pra->beta)-pra->beta*(line_bits-pra->rou))/(deta_t3*pra->rou+deta_t4*(line_bits-pra->rou));   
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
   float i1,i2,i3,i4;
   for(i=0;i<img1.rows;i++)
   {
        for(j=0;j<img1.cols;j++)
        {
            i1=(float)(img1.at<u_int16_t>(i,j))/deta_t1;
            if(i1<=u1_h){
               w1=1;
            }else
            {
               w1=exp(-c*(i1-u1_h)*(i1-u1_h));
            }
            i2=(float)(img2.at<u_int16_t>(i,j))/deta_t2;
            if(i2<=u2_l)
            {
               w2=exp(-c*(i2-u2_l)*(i2-u2_l));
            }else if(i2>u2_l && i2 <=u2_h)
            {
               w2=1; 
            }else
            {
                w2=exp(-c*(i2-u2_h)*(i2-u2_h));
            }
            i3=(float)(img3.at<u_int16_t>(i,j))/deta_t3;
            if(i3<=u3_l)
            {
               w3=exp(-c*(i3-u3_l)*(i3-u3_l));
            }else if(i3>u3_l && i3 <=u3_h)
            {
               w3=1; 
            }else
            {
                w3=exp(-c*(i3-u3_h)*(i3-u3_h));
            }
	       if(pra->frame_num<3){
		    w3=0;
	      }
            i4=(float)(img4.at<u_int16_t>(i,j))/deta_t4;
            if(i4<=u4_l){
                w4=exp(-c*(i4-u4_l)*(i4-u4_l));               
            }else
            {
               w4=1;
            }
		 if(pra->frame_num<4){
		    w4=0;
		 }
            merge_img.at<float>(i,j)=(w1*i1+w2*i2+w3*i3+w4*i4)/(w1+w2+w3+w4);   
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
           y_out=hdr_img.at<float>(i,j)/drc_exp_bit;
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
           y_out=pow(hdr_img.at<float>(i,j)/(line_bits*drc_exp_bit),(1/gamma))*line_bits;
           img.at<u_int16_t>(i,j)=(u_int16_t)y_out;
       }
    }
    return img;
}
Mat drc_hdr_img_kang2014_gamma_rgb(Mat hdr_img,float gamma)
{   
   Mat img(hdr_img.rows,hdr_img.cols,CV_16UC3); 
   int i=0,j=0;   
   float y_out=0;
   //y_out=x^(1/gamma)  x>=0 && x<=1
   for(i=0;i<hdr_img.rows;i++)
   {
       for(j=0;j<hdr_img.cols;j++)
       {
           y_out=pow(hdr_img.at<Vec3f>(i,j)[0]/(line_bits*drc_exp_bit),(1/gamma))*line_bits;
           img.at<Vec3w>(i,j)[0]=(u_int16_t)y_out;
		y_out=pow(hdr_img.at<Vec3f>(i,j)[1]/(line_bits*drc_exp_bit),(1/gamma))*line_bits;
           img.at<Vec3w>(i,j)[1]=(u_int16_t)y_out;
		y_out=pow(hdr_img.at<Vec3f>(i,j)[2]/(line_bits*drc_exp_bit),(1/gamma))*line_bits;
           img.at<Vec3w>(i,j)[2]=(u_int16_t)y_out;
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
               y_out=pow(hdr_img.at<float>(i,j)/max_val,(1/pra->gamma))*line_bits;
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
               y_out=pra->k1*y_in*exp(-pra->k2*pow(y_in,pra->k3))*line_bits;   
               img.at<u_int16_t>(i,j)=y_out;
           }
        }
    }
    return img;
}
Mat drc_hdr_img_kang2014_bilateral(Mat hdr_img)
{
    float compression_factor=log10(255)/(log10(line_bits*drc_exp_bit)-log10(1));
    float singma_s=0.02*1920;
    float singama_r=0.4;
    Mat y_imge(hdr_img.rows,hdr_img.cols,CV_32FC1);
    Mat color_imge(hdr_img.rows,hdr_img.cols,CV_32FC3);
    Mat log_base;
    Mat log_y_image(hdr_img.rows,hdr_img.cols,CV_32FC1);
    Mat log_y_detail(hdr_img.rows,hdr_img.cols,CV_32FC1);
    Mat log_y_out(hdr_img.rows,hdr_img.cols,CV_32FC1);;
    Mat out_image(hdr_img.rows,hdr_img.cols,CV_32FC3);
    double average=0;
    double log_avergae=0;
    for(int i=0;i<hdr_img.rows;i++)
    {
        for(int j=0;j<hdr_img.cols;j++)
        {
            y_imge.at<float>(i,j)=0.299*hdr_img.at<Vec3f>(i,j)[2]+0.587*hdr_img.at<Vec3f>(i,j)[1]+0.144*hdr_img.at<Vec3f>(i,j)[0];
		 color_imge.at<Vec3f>(i,j)[2]=hdr_img.at<Vec3f>(i,j)[2]/y_imge.at<float>(i,j);		 
		 color_imge.at<Vec3f>(i,j)[1]=hdr_img.at<Vec3f>(i,j)[1]/y_imge.at<float>(i,j);
		 color_imge.at<Vec3f>(i,j)[0]=hdr_img.at<Vec3f>(i,j)[0]/y_imge.at<float>(i,j);
		 log_y_image.at<float>(i,j)=log10(y_imge.at<float>(i,j)+1);		 
		 average+=log_y_image.at<float>(i,j);
        }
    }
    log_avergae=(average/hdr_img.rows/hdr_img.cols);	
    log_info("compression_factor=%f  log_avergae=%f log_avergae=%f",compression_factor,average,log_avergae);
    log_info("start bilateralFilter");
    cv::bilateralFilter(log_y_image,log_base,10,singama_r,singma_s);
    for(int i=0;i<hdr_img.rows;i++)
    {
        for(int j=0;j<hdr_img.cols;j++)
        {
			log_y_detail.at<float>(i,j)=log_y_image.at<float>(i,j)-log_base.at<float>(i,j);
			log_y_out.at<float>(i,j)=log_y_detail.at<float>(i,j)+(log_base.at<float>(i,j))*compression_factor;
			out_image.at<Vec3f>(i,j)[2]=color_imge.at<Vec3f>(i,j)[2]*pow(10,log_y_out.at<float>(i,j));			
			out_image.at<Vec3f>(i,j)[1]=color_imge.at<Vec3f>(i,j)[1]*pow(10,log_y_out.at<float>(i,j));
		     out_image.at<Vec3f>(i,j)[0]=color_imge.at<Vec3f>(i,j)[0]*pow(10,log_y_out.at<float>(i,j));
			y_imge.at<float>(i,j)=0.299*out_image.at<Vec3f>(i,j)[2]+0.587*out_image.at<Vec3f>(i,j)[1]+0.144*out_image.at<Vec3f>(i,j)[0];
        }
    }
    return out_image;
}

Mat drc_hdr_img_kang2014_local_bilateral(Mat hdr_img)
{
    float compression_factor=2;
    float log_absolute_scaler=0;//log(0xffff*4096)*compression_factor;   
    float singma_s=0.02*1920;
    float singama_r=0.4;
    log_info("compression_factor=%f  log_absolute_scaler=%f",compression_factor,log_absolute_scaler);
    Mat y_imge(hdr_img.rows,hdr_img.cols,CV_32FC1);
    Mat color_imge(hdr_img.rows,hdr_img.cols,CV_32FC3);
    Mat log_base;
    Mat log_y_image(hdr_img.rows,hdr_img.cols,CV_32FC1);
    Mat log_y_detail(hdr_img.rows,hdr_img.cols,CV_32FC1);
    Mat log_y_out(hdr_img.rows,hdr_img.cols,CV_32FC1);;
    Mat out_image(hdr_img.rows,hdr_img.cols,CV_32FC3);
    for(int i=0;i<hdr_img.rows;i++)
    {
        for(int j=0;j<hdr_img.cols;j++)
        {
            y_imge.at<float>(i,j)=(0.299*hdr_img.at<Vec3f>(i,j)[2]+0.587*hdr_img.at<Vec3f>(i,j)[1]+0.144*hdr_img.at<Vec3f>(i,j)[0]);
		 color_imge.at<Vec3f>(i,j)[2]=hdr_img.at<Vec3f>(i,j)[2]/y_imge.at<float>(i,j);		 
		 color_imge.at<Vec3f>(i,j)[1]=hdr_img.at<Vec3f>(i,j)[1]/y_imge.at<float>(i,j);
		 color_imge.at<Vec3f>(i,j)[0]=hdr_img.at<Vec3f>(i,j)[0]/y_imge.at<float>(i,j);
		 log_y_image.at<float>(i,j)=y_imge.at<float>(i,j)/(line_bits*drc_exp_bit);
        }
    }
    log_info("start bilateralFilter");
    cv::bilateralFilter(log_y_image,log_base,5,singama_r,singma_s);
    for(int i=0;i<hdr_img.rows;i++)
    {
        for(int j=0;j<hdr_img.cols;j++)
        {
			log_y_detail.at<float>(i,j)=log_y_image.at<float>(i,j)/log_base.at<float>(i,j);
			log_y_out.at<float>(i,j)=log_y_detail.at<float>(i,j)*pow(log_base.at<float>(i,j),1/compression_factor)/log_base.at<float>(i,j)*log_y_image.at<float>(i,j);
			
			//log_y_out.at<float>(i,j)=pow(log_base.at<float>(i,j),1/compression_factor)/log_base.at<float>(i,j)*log_y_image.at<float>(i,j);
			out_image.at<Vec3f>(i,j)[2]=color_imge.at<Vec3f>(i,j)[2]*log_y_out.at<float>(i,j)*256;			
			out_image.at<Vec3f>(i,j)[1]=color_imge.at<Vec3f>(i,j)[1]*log_y_out.at<float>(i,j)*256;
		     out_image.at<Vec3f>(i,j)[0]=color_imge.at<Vec3f>(i,j)[0]*log_y_out.at<float>(i,j)*256;
        }
    }
    return out_image;
}


