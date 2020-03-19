#include "cv.h"
#include "highgui.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "isp_pipeline.h"
#include "isp_log.h"
#include <fcntl.h>
int save_mat_to_bin(char *picname,char *func_name,cv::Mat img);

char *load_yuv3p4b_10bit_img(char *name,int w,int h, int stride_w,char *format)
{
   int len=0;
   if(!strcmp(format,"yuv_420"))
   {
      len=stride_w*h*3/2;
   }else if(!strcmp(format,"yuv_422"))
   {
      len=stride_w*h*2;
   }
   char *buf=(char *)malloc(len*2);
   load_bin(name,buf,len);
   return buf;
}


vector<cv::Mat> convert_yuv3p4b_10bit_420sp_bin_to_yuv1p2b_420sp_2(char *buffer,int w,int h,int stride_w)
{
   log_info("enter");
   vector<cv::Mat> yuv_vector;
   cv::Mat y_img(h,w,CV_16UC1);
   cv::Mat u_img(h/2,w/2,CV_16UC1);
   cv::Mat v_img(h/2,w/2,CV_16UC1);
   char *p_line=buffer;
   u_int64_t *p_8b;
   u_int16_t val;
   u_int64_t val_8b;
   int i=0;
   int j=0;
   int k=0;

   //y image
   for(i=0;i<h;i++)
   {
      p_line=buffer+stride_w*i;      
      p_8b=(u_int64_t *)p_line;      
      k=0;
      for(j=0;j<w;j+=12)
      {
          val_8b=p_8b[k];
          y_img.at<u_int16_t>(i,j+0)=(val_8b&(u_int64_t)0x3ff)<<6;          
          y_img.at<u_int16_t>(i,j+1)=((val_8b>>10)&(u_int64_t)0x3ff)<<6;
          y_img.at<u_int16_t>(i,j+2)=((val_8b>>20)&(u_int64_t)0x3ff)<<6;          
          y_img.at<u_int16_t>(i,j+3)=((val_8b>>30)&(u_int64_t)0x3ff)<<6;          
          y_img.at<u_int16_t>(i,j+4)=((val_8b>>40)&(u_int64_t)0x3ff)<<6;
          y_img.at<u_int16_t>(i,j+5)=((val_8b>>50)&(u_int64_t)0x3ff)<<6;
          y_img.at<u_int16_t>(i,j+6)=((val_8b>>60)&(u_int64_t)0xf);
          k++;
          val_8b=p_8b[k];
          y_img.at<u_int16_t>(i,j+6)=(((val_8b)&(u_int64_t)0x3f)<<4 | y_img.at<u_int16_t>(i,j+6))<<6;
          val_8b=val_8b>>6;
          y_img.at<u_int16_t>(i,j+7)=(val_8b&(u_int64_t)0x3ff)<<6;
          y_img.at<u_int16_t>(i,j+8)=((val_8b>>10)&(u_int64_t)0x3ff)<<6;
          y_img.at<u_int16_t>(i,j+9)=((val_8b>>20)&(u_int64_t)0x3ff)<<6;          
          y_img.at<u_int16_t>(i,j+10)=((val_8b>>30)&(u_int64_t)0x3ff)<<6;          
          y_img.at<u_int16_t>(i,j+11)=((val_8b>>40)&(u_int64_t)0x3ff)<<6;
          k++;
          
      }
   }
   //u 
   buffer+=stride_w*h;
   for(i=0;i<h/2;i++)
   {
      p_line=buffer+stride_w/2*i;      
      p_8b=(u_int64_t *)p_line;      
      k=0;
      for(j=0;j<w/2;j+=12)
      {
          val_8b=p_8b[k];
          u_img.at<u_int16_t>(i,j+0)=(val_8b&(u_int64_t)0x3ff)<<6;          
          u_img.at<u_int16_t>(i,j+1)=((val_8b>>10)&(u_int64_t)0x3ff)<<6;
          u_img.at<u_int16_t>(i,j+2)=((val_8b>>20)&(u_int64_t)0x3ff)<<6;          
          u_img.at<u_int16_t>(i,j+3)=((val_8b>>30)&(u_int64_t)0x3ff)<<6;          
          u_img.at<u_int16_t>(i,j+4)=((val_8b>>40)&(u_int64_t)0x3ff)<<6;
          u_img.at<u_int16_t>(i,j+5)=((val_8b>>50)&(u_int64_t)0x3ff)<<6;
          u_img.at<u_int16_t>(i,j+6)=((val_8b>>60)&(u_int64_t)0xf);
          k++;
          val_8b=p_8b[k];
          u_img.at<u_int16_t>(i,j+6)=(((val_8b)&(u_int64_t)0x3f)<<4 | u_img.at<u_int16_t>(i,j+6))<<6;
          val_8b=val_8b>>6;
          u_img.at<u_int16_t>(i,j+7)=(val_8b&(u_int64_t)0x3ff)<<6;
          u_img.at<u_int16_t>(i,j+8)=((val_8b>>10)&(u_int64_t)0x3ff)<<6;
          u_img.at<u_int16_t>(i,j+9)=((val_8b>>20)&(u_int64_t)0x3ff)<<6;          
          u_img.at<u_int16_t>(i,j+10)=((val_8b>>30)&(u_int64_t)0x3ff)<<6;          
          u_img.at<u_int16_t>(i,j+11)=((val_8b>>40)&(u_int64_t)0x3ff)<<6;
          k++;
      }
   }
   //v
   buffer+=stride_w/2*h/2;
   for(i=0;i<h/2;i++)
   {
      p_line=buffer+stride_w/2*i;      
      p_8b=(u_int64_t *)p_line;      
      k=0;
      for(j=0;j<w/2;j+=12)
      {
          val_8b=p_8b[k];
          v_img.at<u_int16_t>(i,j+0)=(val_8b&(u_int64_t)0x3ff)<<6;          
          v_img.at<u_int16_t>(i,j+1)=((val_8b>>10)&(u_int64_t)0x3ff)<<6;
          v_img.at<u_int16_t>(i,j+2)=((val_8b>>20)&(u_int64_t)0x3ff)<<6;          
          v_img.at<u_int16_t>(i,j+3)=((val_8b>>30)&(u_int64_t)0x3ff)<<6;          
          v_img.at<u_int16_t>(i,j+4)=((val_8b>>40)&(u_int64_t)0x3ff)<<6;
          v_img.at<u_int16_t>(i,j+5)=((val_8b>>50)&(u_int64_t)0x3ff)<<6;
          v_img.at<u_int16_t>(i,j+6)=((val_8b>>60)&(u_int64_t)0xf);
          k++;
          val_8b=p_8b[k];
          v_img.at<u_int16_t>(i,j+6)=(((val_8b)&(u_int64_t)0x3f)<<4 | v_img.at<u_int16_t>(i,j+6))<<6;
          val_8b=val_8b>>6;
          v_img.at<u_int16_t>(i,j+7)=(val_8b&(u_int64_t)0x3ff)<<6;
          v_img.at<u_int16_t>(i,j+8)=((val_8b>>10)&(u_int64_t)0x3ff)<<6;
          v_img.at<u_int16_t>(i,j+9)=((val_8b>>20)&(u_int64_t)0x3ff)<<6;          
          v_img.at<u_int16_t>(i,j+10)=((val_8b>>30)&(u_int64_t)0x3ff)<<6;          
          v_img.at<u_int16_t>(i,j+11)=((val_8b>>40)&(u_int64_t)0x3ff)<<6;
          k++;

      }
   }
   yuv_vector.push_back(y_img);   
   yuv_vector.push_back(u_img);
   yuv_vector.push_back(v_img);      
   log_info("exit");
   return yuv_vector;
}

vector<cv::Mat> convert_yuv3p4b_10bit_420sp_bin_to_yuv1p2b_420sp(char *buffer,int w,int h,int stride_w)
{
   log_info("enter");
   vector<cv::Mat> yuv_vector;
   cv::Mat y_img(h,w,CV_16UC1);
   cv::Mat u_img(h/2,w/2,CV_16UC1);
   cv::Mat v_img(h/2,w/2,CV_16UC1);
   char *p_line=buffer;
   u_int32_t *p_4b;
   u_int16_t val;
   u_int32_t val_4b;
   int i=0;
   int j=0;
   int k=0;

   //y image
   for(i=0;i<h;i++)
   {
      p_line=buffer+stride_w*i;      
      p_4b=(u_int32_t *)p_line;
      k=0;
      for(j=0;j<w;j+=3)
      {
          val_4b=p_4b[k];
          y_img.at<u_int16_t>(i,j+0)=(val_4b&0x3ff)<<6;          
          y_img.at<u_int16_t>(i,j+1)=((val_4b>>10)&0x3ff)<<6;
          y_img.at<u_int16_t>(i,j+2)=((val_4b>>20)&0x3ff)<<6;
          k++;
      }
   }
   //u 
   buffer+=stride_w*h;
   for(i=0;i<h/2;i++)
   {
      p_line=buffer+stride_w/2*i;      
      p_4b=(u_int32_t *)p_line;      
      k=0;
      for(j=0;j<w/2;j+=3)
      {
          val_4b=p_4b[k];
          u_img.at<u_int16_t>(i,j+0)=(val_4b&0x3ff)<<6;          
          u_img.at<u_int16_t>(i,j+1)=((val_4b>>10)&0x3ff)<<6;
          u_img.at<u_int16_t>(i,j+2)=((val_4b>>20)&0x3ff)<<6;
          k++;
      }
   }
   //v
   buffer+=stride_w/2*h/2;
   for(i=0;i<h/2;i++)
   {
      p_line=buffer+stride_w/2*i;      
      p_4b=(u_int32_t *)p_line;      
      k=0;
      for(j=0;j<w/2;j+=3)
      {
          val_4b=p_4b[k];
          v_img.at<u_int16_t>(i,j+0)=(val_4b&0x3ff)<<6;          
          v_img.at<u_int16_t>(i,j+1)=((val_4b>>10)&0x3ff)<<6;
          v_img.at<u_int16_t>(i,j+2)=((val_4b>>20)&0x3ff)<<6;
          k++;
      }
   }
   yuv_vector.push_back(y_img);   
   yuv_vector.push_back(u_img);
   yuv_vector.push_back(v_img);      
   log_info("exit");
   return yuv_vector;
}

vector<cv::Mat> convert_yuv3p4b_10bit_420simi_bin_to_yuv1p2b_420sp(char *buffer,int w,int h,int stride_w)
{
   log_info("enter");
   vector<cv::Mat> yuv_vector;
   cv::Mat y_img(h,w,CV_16UC1);
   cv::Mat u_img(h/2,w/2,CV_16UC1);
   cv::Mat v_img(h/2,w/2,CV_16UC1);
   char *p_line=buffer;
   u_int32_t *p_4b;
   u_int16_t val;
   u_int32_t val_4b;
   int i=0;
   int j=0;
   int k=0;

   //y image
   for(i=0;i<h;i++)
   {
      p_line=buffer+stride_w*i;      
      p_4b=(u_int32_t *)p_line;
      k=0;
      for(j=0;j<w;j+=3)
      {
          val_4b=p_4b[k];
          y_img.at<u_int16_t>(i,j+0)=(val_4b&0x3ff)<<6;          
          y_img.at<u_int16_t>(i,j+1)=((val_4b>>10)&0x3ff)<<6;
          y_img.at<u_int16_t>(i,j+2)=((val_4b>>20)&0x3ff)<<6;
          k++;
      }
   }
   //u  v
   buffer+=stride_w*h;
   for(i=0;i<h/2;i++)
   {
      p_line=buffer+stride_w*i;      
      p_4b=(u_int32_t *)p_line;      
      k=0;
      for(j=0;j<w/2;j+=3)
      {
          val_4b=p_4b[k];
          u_img.at<u_int16_t>(i,j+0)=(val_4b&0x3ff)<<6;          
          v_img.at<u_int16_t>(i,j+0)=((val_4b>>10)&0x3ff)<<6;
          u_img.at<u_int16_t>(i,j+1)=((val_4b>>20)&0x3ff)<<6;
          k++;          
          val_4b=p_4b[k];          
          v_img.at<u_int16_t>(i,j+1)=(val_4b&0x3ff)<<6;          
          u_img.at<u_int16_t>(i,j+2)=((val_4b>>10)&0x3ff)<<6;
          v_img.at<u_int16_t>(i,j+2)=((val_4b>>20)&0x3ff)<<6;
          k++;  
          
      }
   }
   yuv_vector.push_back(y_img);   
   yuv_vector.push_back(u_img);
   yuv_vector.push_back(v_img);      
   log_info("exit");
   return yuv_vector;
}

vector<cv::Mat> convert_yuv3p4b_10bit_422simi_bin_to_yuv1p2b_422sp(char *buffer,int w,int h,int stride_w)
{
   log_info("enter");
   vector<cv::Mat> yuv_vector;
   cv::Mat y_img(h,w,CV_16UC1);
   cv::Mat u_img(h/2,w,CV_16UC1);
   cv::Mat v_img(h/2,w,CV_16UC1);
   char *p_line=buffer;
   u_int32_t *p_4b;
   u_int16_t val;
   u_int32_t val_4b;
   int i=0;
   int j=0;
   int k=0;

   //y image
   for(i=0;i<h;i++)
   {
      p_line=buffer+stride_w*i;      
      p_4b=(u_int32_t *)p_line;
      k=0;
      for(j=0;j<w;j+=3)
      {
          val_4b=p_4b[k];
          y_img.at<u_int16_t>(i,j+0)=(val_4b&0x3ff)<<6;          
          y_img.at<u_int16_t>(i,j+1)=((val_4b>>10)&0x3ff)<<6;
          y_img.at<u_int16_t>(i,j+2)=((val_4b>>20)&0x3ff)<<6;
          k++;
      }
   }
   //u  v
   buffer+=stride_w*h;
   for(i=0;i<h/2;i++)
   {
      p_line=buffer+stride_w*2*i;      
      p_4b=(u_int32_t *)p_line;      
      k=0;
      for(j=0;j<w;j+=3)
      {
          val_4b=p_4b[k];
          u_img.at<u_int16_t>(i,j+0)=(val_4b&0x3ff)<<6;          
          v_img.at<u_int16_t>(i,j+0)=((val_4b>>10)&0x3ff)<<6;
          u_img.at<u_int16_t>(i,j+1)=((val_4b>>20)&0x3ff)<<6;
          k++;          
          val_4b=p_4b[k];          
          v_img.at<u_int16_t>(i,j+1)=(val_4b&0x3ff)<<6;          
          u_img.at<u_int16_t>(i,j+2)=((val_4b>>10)&0x3ff)<<6;
          v_img.at<u_int16_t>(i,j+2)=((val_4b>>20)&0x3ff)<<6;
          k++;  
          
      }
   }
   yuv_vector.push_back(y_img);   
   yuv_vector.push_back(u_img);
   yuv_vector.push_back(v_img);      
   log_info("exit");
   return yuv_vector;
}


int save_yuv4201p2b_to_yuv420_8bit_bin(char *name,vector<cv::Mat> yuv_image)
{
    char name_l[128];
    cv::Mat tmp_mat=yuv_image[0];
    cv::Mat y_img(tmp_mat.rows,tmp_mat.cols,CV_8UC1);
    cv::Mat u_img(tmp_mat.rows/2,tmp_mat.cols/2,CV_8UC1);
    cv::Mat v_img(tmp_mat.rows/2,tmp_mat.cols/2,CV_8UC1);
    log_info("enter");
    sprintf(name_l,"%s_%dx%d.yuv",name,tmp_mat.cols,tmp_mat.rows);    
    log_info("open file %s",name_l);
    int fd=open(name_l,O_RDWR|O_CREAT);
    if(fd<0)
    {
      log_err("open file failed");
      return fd;
    }    
    log_info("start write %s",name_l);
    int i=0;
    int j=0;
    for(i=0;i<tmp_mat.rows;i++)
    {
        for(j=0;j<tmp_mat.cols;j++)
        {
            y_img.at<u_int8_t>(i,j)=yuv_image[0].at<u_int16_t>(i,j)>>8;
        }
    }
    for(i=0;i<tmp_mat.rows/2;i++)
    {
        for(j=0;j<tmp_mat.cols/2;j++)
        {
            u_img.at<u_int8_t>(i,j)=yuv_image[1].at<u_int16_t>(i,j)>>8;
            v_img.at<u_int8_t>(i,j)=yuv_image[2].at<u_int16_t>(i,j)>>8;
        }
    }
    
    write(fd,y_img.data,y_img.cols*y_img.rows*y_img.elemSize());    
    write(fd,u_img.data,u_img.cols*u_img.rows*u_img.elemSize());
    write(fd,v_img.data,v_img.cols*v_img.rows*v_img.elemSize());    
    log_info("save success %s",name_l);    
    close(fd); 
    save_mat_to_bin(name_l,(char *)".y.yuv",y_img);
    save_mat_to_bin(name_l,(char *)".u.yuv",u_img);    
    save_mat_to_bin(name_l,(char *)".v.yuv",v_img);      
    log_info("save y u v success %s",name_l);
    return 0;
}

vector<cv::Mat> yuv4221p2b_to_yuv422_8bit(vector<cv::Mat> yuv_image)
{
    cv::Mat tmp_mat=yuv_image[0];
    cv::Mat y_img(tmp_mat.rows,tmp_mat.cols,CV_8UC1);
    cv::Mat u_img(tmp_mat.rows/2,tmp_mat.cols,CV_8UC1);
    cv::Mat v_img(tmp_mat.rows/2,tmp_mat.cols,CV_8UC1);
    vector<cv::Mat> yuv_8_img;
    int i=0;
    int j=0;
    for(i=0;i<tmp_mat.rows;i++)
    {
        for(j=0;j<tmp_mat.cols;j++)
        {
            y_img.at<u_int8_t>(i,j)=yuv_image[0].at<u_int16_t>(i,j)>>8;
        }
    }
    for(i=0;i<tmp_mat.rows/2;i++)
    {
        for(j=0;j<tmp_mat.cols;j++)
        {
            u_img.at<u_int8_t>(i,j)=yuv_image[1].at<u_int16_t>(i,j)>>8;
            v_img.at<u_int8_t>(i,j)=yuv_image[2].at<u_int16_t>(i,j)>>8;
        }
    }
    yuv_8_img.push_back(y_img);   
    yuv_8_img.push_back(u_img);
    yuv_8_img.push_back(v_img);   
    return yuv_8_img;
}

int save_yuv4221p2b_to_yuv422_8bit_bin(char *name,vector<cv::Mat> yuv_image)
{
    log_info("enter");
    char name_l[128];
    cv::Mat tmp_mat=yuv_image[0];
    cv::Mat y_img(tmp_mat.rows,tmp_mat.cols,CV_8UC1);
    cv::Mat u_img(tmp_mat.rows/2,tmp_mat.cols,CV_8UC1);
    cv::Mat v_img(tmp_mat.rows/2,tmp_mat.cols,CV_8UC1);
    sprintf(name_l,"%s_%dx%d.yuv",name,tmp_mat.cols,tmp_mat.rows);    
    log_info("open file %s",name_l);
    int fd=open(name_l,O_RDWR|O_CREAT);
    if(fd<0)
    {
      log_err("open file failed");
      return fd;
    }    
    log_info("start write %s",name_l);
    int i=0;
    int j=0;
    for(i=0;i<tmp_mat.rows;i++)
    {
        for(j=0;j<tmp_mat.cols;j++)
        {
            y_img.at<u_int8_t>(i,j)=yuv_image[0].at<u_int16_t>(i,j)>>8;
        }
    }
    for(i=0;i<tmp_mat.rows/2;i++)
    {
        for(j=0;j<tmp_mat.cols;j++)
        {
            u_img.at<u_int8_t>(i,j)=yuv_image[1].at<u_int16_t>(i,j)>>8;
            v_img.at<u_int8_t>(i,j)=yuv_image[2].at<u_int16_t>(i,j)>>8;
        }
    }
    
    write(fd,y_img.data,y_img.cols*y_img.rows*y_img.elemSize());    
    write(fd,u_img.data,u_img.cols*u_img.rows*u_img.elemSize());
    write(fd,v_img.data,v_img.cols*v_img.rows*v_img.elemSize());    
    log_info("save success %s",name_l);    
    close(fd); 
    save_mat_to_bin(name_l,(char *)".y.yuv",y_img);
    save_mat_to_bin(name_l,(char *)".u.yuv",u_img);    
    save_mat_to_bin(name_l,(char *)".v.yuv",v_img);      
    log_info("save y u v success %s",name_l);
    return 0;
}


int save_yuv4201p2b_to_bin(char *name,vector<cv::Mat> yuv_image)
{
    char name_l[128];
    cv::Mat tmp_mat=yuv_image[0];
    log_info("enter");
    sprintf(name_l,"%s_%dx%d.yuv",name,tmp_mat.cols,tmp_mat.rows);    
    log_info("open file %s",name_l);
    int fd=open(name_l,O_RDWR|O_CREAT);
    if(fd<0)
    {
      log_err("open file failed");
      return fd;
    }    
    log_info("start write %s",name_l);
    for(int i=0;i<3;i++){
       tmp_mat=yuv_image[i];
       write(fd,tmp_mat.data,tmp_mat.cols*tmp_mat.rows*tmp_mat.elemSize());
    }
    log_info("save success %s",name_l);
    close(fd);     
    save_mat_to_bin(name_l,(char *)".y.yuv",yuv_image[0]);
    save_mat_to_bin(name_l,(char *)".u.yuv",yuv_image[1]);    
    save_mat_to_bin(name_l,(char *)".v.yuv",yuv_image[2]);      
    log_info("save y u v success %s",name_l);
    return 0;
}

int yuv422simi_3p4b_save_to_yuv422sp_8bit(char *name,int w,int h,int stride,int num)
{
  int i=0;
  int len=stride*h*2;
  char *buf=(char *)malloc(len*2);
  vector<cv::Mat> yuv_img; 
  vector<cv::Mat> yuv_8_img;
  cv::Mat tmp_mat;  
  char name_l[128];
  int fd=open(name,O_RDONLY);
  if(fd<0)
  {
    log_err("open file failed,please firset gen the data");
	return fd;
  }  
  sprintf(name_l,"%s_8bit420_%dx%d.yuv",name,w,h);    
  int fd_save=open(name,O_RDWR|O_CREAT);
  if(fd_save<0)
  {
    log_err("open file failed,please firset gen the data");
	return fd_save;
  }

  
  for(i=0;i<num;i++)
  {     
     read(fd,buf,len); 
     yuv_img=convert_yuv3p4b_10bit_422simi_bin_to_yuv1p2b_422sp(buf,w,h,stride);
     yuv_8_img=yuv4221p2b_to_yuv422_8bit(yuv_img);
     for(int j=0;j<3;j++)
     {
       tmp_mat=yuv_8_img[i];
       write(fd_save,tmp_mat.data,tmp_mat.cols*tmp_mat.rows*tmp_mat.elemSize());
    }
  }
  close(fd);  
  close(fd_save);
}
