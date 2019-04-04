#include "isp_pipeline.h"
raw_hist_t get_raw_hist(int bins,raw_ch_t raw_ch,bayer_format_t bayer)
{
   log_func_enter();
   int factor=65536/bins;
   int i=0,j=0;
   int val=0;
   raw_hist_t raw_hist;   
   cv::Mat sub_raw;
   raw_hist.bins=bins;
   log_info("bins=%d factor=%d",bins,factor);
   raw_hist.r_hist=(u_int32_t *)malloc(bins*sizeof(int)); 
   memset(raw_hist.r_hist,0,bins*sizeof(int));
   raw_hist.gr_hist=(u_int32_t *)malloc(bins*sizeof(int));
   memset(raw_hist.gr_hist,0,bins*sizeof(int));
   raw_hist.gb_hist=(u_int32_t *)malloc(bins*sizeof(int));
   memset(raw_hist.gb_hist,0,bins*sizeof(int));
   raw_hist.b_hist=(u_int32_t *)malloc(bins*sizeof(int));
   memset(raw_hist.b_hist,0,bins*sizeof(int));
   
   raw_hist.sum=raw_ch.raw_ch_00.rows*raw_ch.raw_ch_00.cols;
   for(i=0;i<raw_ch.raw_ch_00.rows;i++){
     for(j=0;j<raw_ch.raw_ch_00.cols;j++)
     {
         sub_raw=get_R_raw(raw_ch,bayer);
         val=sub_raw.at<u_int16_t>(i,j)/factor;
         raw_hist.r_hist[val]++;

         
         sub_raw=get_Gr_raw(raw_ch,bayer);
         val=sub_raw.at<u_int16_t>(i,j)/factor;
         raw_hist.gr_hist[val]++;
         
         sub_raw=get_Gb_raw(raw_ch,bayer);
         val=sub_raw.at<u_int16_t>(i,j)/factor;
         raw_hist.gb_hist[val]++;
         
         sub_raw=get_B_raw(raw_ch,bayer);
         val=sub_raw.at<u_int16_t>(i,j)/factor;
         raw_hist.b_hist[val]++;
     }
   }
   log_func_exit();
   return raw_hist;
}
int release_raw_hist(raw_hist_t *raw_hist)
{
    if(raw_hist->r_hist)
    {
       free(raw_hist->r_hist);
       raw_hist->r_hist=NULL;
    }
    if(raw_hist->gr_hist)
    {
       free(raw_hist->gr_hist);
       raw_hist->gr_hist=NULL;
    }
    if(raw_hist->gb_hist)
    {
       free(raw_hist->gb_hist);
       raw_hist->gb_hist=NULL;
    }
    if(raw_hist->b_hist)
    {
       free(raw_hist->b_hist);
       raw_hist->b_hist=NULL;
    }
    raw_hist->bins=0;    
    raw_hist->sum=0;
    return 0;
}

int log_raw_hist(raw_hist_t *raw_hist)
{
   int i=0;
   for(i=0;i<raw_hist->bins;i++)
   {
      printf("bin=%d [r:%d gr:%d gb:%d b:%d] [r:%f g:%f b:%f] \n",i,raw_hist->r_hist[i],raw_hist->gr_hist[i],raw_hist->gb_hist[i],raw_hist->b_hist[i],\
             raw_hist->r_hist[i]/(float)raw_hist->sum,(raw_hist->gr_hist[i]+raw_hist->gb_hist[i])/(float)(raw_hist->sum*2),raw_hist->b_hist[i]/(float)raw_hist->sum);
   }
   return 0;
}

int save_raw_hist(char *name,raw_hist_t *raw_hist)
{
  char name_f[128]={0};
  sprintf(name_f,"%s.raw_hist.bin",name);
  int fd=open(name_f,O_RDWR|O_CREAT);
  if(fd<0)
  {
    log_err("open file %s failed",name_f);
	return fd;
  }
  write(fd,raw_hist->r_hist,raw_hist->bins*sizeof(int));
  write(fd,raw_hist->gr_hist,raw_hist->bins*sizeof(int));
  write(fd,raw_hist->gb_hist,raw_hist->bins*sizeof(int));
  write(fd,raw_hist->b_hist,raw_hist->bins*sizeof(int));
  log_info("save %s success",name_f);
  close(fd);
  return 0;
}
