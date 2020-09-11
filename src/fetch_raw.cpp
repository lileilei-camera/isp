#include "cv.h"
#include "highgui.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "isp_pipeline.h"
#include "isp_log.h"

cv::Mat read_128bit_mipi_raw_to_dng16_raw(char *name,raw_type_file_dscr_t *raw_dscr);
cv::Mat read_mipi_raw_to_dng16_raw(char *name,raw_type_file_dscr_t *raw_dscr)
{
  cv::Mat ret;
  if(!name)
  {
    log_err("file name null");
    return ret;
  }
  int fd=open(name,O_RDONLY);
  if(fd<0)
  {
    log_err("open file failed");
    return ret;
  }
  int buf_width=ALIGN_TO(ALIGN_TO(raw_dscr->width,raw_dscr->width_algin)*raw_dscr->bitwidth/8,raw_dscr->line_length_algin);  
  int raw_size=buf_width*raw_dscr->hegiht;  
  log_info("buf_width=%d,hegiht=%d raw_size=%d",buf_width,raw_dscr->hegiht,raw_size);
  u_int8_t  *p_raw=(u_int8_t  *)malloc(raw_size);
  if(!p_raw)
  {
    log_err("malloc err");
    close(fd);
    return ret;
  }
  read(fd,p_raw,raw_size);
  //then conver the raw file buffer to unpacked uint16 raw
  cv::Mat raw_16_img(raw_dscr->hegiht,raw_dscr->width,CV_16UC1);
  int i,j;
  u_int16_t pix_val[4];
  for(i=0;i<raw_dscr->hegiht;i++)
  {
     if(raw_dscr->is_packed&&raw_dscr->bitwidth==10)
     {
       int pix_index=0;
       int line_offset=i*buf_width;       
       log_debug("line_offset=%d i=%d",line_offset,i);
       for(j=0;j<buf_width;)
       {
          SET_RAW10_PIX_HIGH(pix_val[0],p_raw[line_offset+j+0]);
          SET_RAW10_PIX_LOW(pix_val[0],GET_RAW10_PIX0_LOW(p_raw[line_offset+j+4]));
          raw_16_img.at<u_int16_t>(i,(pix_index+0))=pix_val[0]<<6;
          
          SET_RAW10_PIX_HIGH(pix_val[1],p_raw[line_offset+j+1]);
          SET_RAW10_PIX_LOW(pix_val[1],GET_RAW10_PIX1_LOW(p_raw[line_offset+j+4]));
          raw_16_img.at<u_int16_t>(i,(pix_index+1))=pix_val[1]<<6;
          
          SET_RAW10_PIX_HIGH(pix_val[2],p_raw[i*buf_width+j+2]);
          SET_RAW10_PIX_LOW(pix_val[2],GET_RAW10_PIX2_LOW(p_raw[line_offset+j+4]));
          raw_16_img.at<u_int16_t>(i,(pix_index+2))=pix_val[2]<<6;
          
          SET_RAW10_PIX_HIGH(pix_val[3],p_raw[i*buf_width+j+3]);
          SET_RAW10_PIX_LOW(pix_val[3],GET_RAW10_PIX3_LOW(p_raw[line_offset+j+4]));
          raw_16_img.at<u_int16_t>(i,(pix_index+3))=pix_val[3]<<6;
          
          j+=5;
          pix_index+=4;
       }   
       log_debug("pix_index=%d j=%d",pix_index,j);
    }else if(!raw_dscr->is_packed)
    {
      
    }
  }
  ret=raw_16_img;
  close(fd);
  free(p_raw);
  log_func_exit();
  return ret;
}


cv::Mat read_128bit_mipi_raw_to_dng16_raw(char *name,raw_type_file_dscr_t *raw_dscr)
{
    log_func_enter();
    cv::Mat ret;
    u_int64_t val; 
    u_int64_t *p_line;
    int k=0;
    if(!name)
    {
      log_err("file name null");
      return ret;
    }
    int fd=open(name,O_RDONLY);
    if(fd<0)
    {
      log_err("open file failed");
      return ret;
    }
    
    int pix_count_per_128bit=128/raw_dscr->bitwidth;
    int burst_128_num_per_line=ceil((float)raw_dscr->width/(float)pix_count_per_128bit);
    int byte_num_per_line=burst_128_num_per_line*16;
    int buf_width=ALIGN_TO(byte_num_per_line,raw_dscr->line_length_algin);  
    int raw_size=buf_width*raw_dscr->hegiht;  
    log_info("buf_width=%d,hegiht=%d raw_size=%d pix_count_per_128bit=%d burst_128_num_per_line=%d byte_num_per_line=%d",\
        buf_width,raw_dscr->hegiht,raw_size,pix_count_per_128bit,burst_128_num_per_line,byte_num_per_line);
    u_int8_t  *p_raw=(u_int8_t  *)malloc(raw_size);
    if(!p_raw)
    {
      log_err("malloc err");
      close(fd);
      return ret;
    }
    read(fd,p_raw,raw_size);
    //then conver the raw file buffer to unpacked uint16 raw
    cv::Mat raw_16_img(raw_dscr->hegiht,raw_dscr->width,CV_16UC1);
    int i,j;
    if(raw_dscr->bitwidth==12){
         for(i=0;i<raw_dscr->hegiht;i++)
         {
            p_line=(u_int64_t *)(p_raw+buf_width*i);
            k=0;
            for(j=0;j<raw_dscr->width;j+=10)
            {
                val=p_line[k];
                raw_16_img.at<u_int16_t>(i,j+0)=(val&0xfff)<<4;
                raw_16_img.at<u_int16_t>(i,j+1)=((val>>12)&0xfff)<<4;
                raw_16_img.at<u_int16_t>(i,j+2)=((val>>24)&0xfff)<<4;                
                raw_16_img.at<u_int16_t>(i,j+3)=((val>>36)&0xfff)<<4;                
                raw_16_img.at<u_int16_t>(i,j+4)=((val>>48)&0xfff)<<4;
                raw_16_img.at<u_int16_t>(i,j+5)=((val>>60)&0xf);                
                k++;
                val=p_line[k];
                raw_16_img.at<u_int16_t>(i,j+5)=(raw_16_img.at<u_int16_t>(i,j+5)|((val&0xff)<<4))<<4;
                val=val>>8;
                raw_16_img.at<u_int16_t>(i,j+6)=(val&0xfff)<<4;
                raw_16_img.at<u_int16_t>(i,j+7)=((val>>12)&0xfff)<<4;
                raw_16_img.at<u_int16_t>(i,j+8)=((val>>24)&0xfff)<<4;                
                raw_16_img.at<u_int16_t>(i,j+9)=((val>>36)&0xfff)<<4;
                k++;
            }
         }
    }else if(raw_dscr->bitwidth==10)
    {
         for(i=0;i<raw_dscr->hegiht;i++)
         {
            p_line=(u_int64_t *)(p_raw+buf_width*i);
            k=0;
            for(j=0;j<raw_dscr->width;j+=12)
            {
                val=p_line[k];
                raw_16_img.at<u_int16_t>(i,j+0)=(val&0x3ff)<<6;
                raw_16_img.at<u_int16_t>(i,j+1)=((val>>10)&0x3ff)<<6;
                raw_16_img.at<u_int16_t>(i,j+2)=((val>>20)&0x3ff)<<6;                
                raw_16_img.at<u_int16_t>(i,j+3)=((val>>30)&0x3ff)<<6;                
                raw_16_img.at<u_int16_t>(i,j+4)=((val>>40)&0x3ff)<<6;
                raw_16_img.at<u_int16_t>(i,j+5)=((val>>50)&0x3ff)<<6;                
                raw_16_img.at<u_int16_t>(i,j+6)=((val>>60)&0xf);                
                k++;
                val=p_line[k];
                raw_16_img.at<u_int16_t>(i,j+6)=(raw_16_img.at<u_int16_t>(i,j+6)|((val&0x3f)<<4))<<6;
                val=val>>6;
                raw_16_img.at<u_int16_t>(i,j+7)=(val&0x3ff)<<6;
                raw_16_img.at<u_int16_t>(i,j+8)=((val>>10)&0x3ff)<<6;
                raw_16_img.at<u_int16_t>(i,j+9)=((val>>20)&0x3ff)<<6;                
                raw_16_img.at<u_int16_t>(i,j+10)=((val>>30)&0x3ff)<<6;                
                raw_16_img.at<u_int16_t>(i,j+11)=((val>>40)&0x3ff)<<6;
                k++;
            }
         }
    }else
    {
      log_err("not supported bits");
    }
    close(fd);
    free(p_raw);
    log_func_exit();
    return raw_16_img;
}

cv::Mat read_dng_raw_to_dng16_raw(char *name,raw_type_file_dscr_t *raw_dscr)
{
  cv::Mat ret;
  u_int16_t *p_val=NULL;
  if(!name)
  {
    log_err("file name null");
    return ret;
  }
  int fd=open(name,O_RDONLY);
  if(fd<0)
  {
    log_err("open file failed");
    return ret;
  }
  int buf_width=ALIGN_TO(raw_dscr->width*2,raw_dscr->line_length_algin);  
  int raw_size=buf_width*raw_dscr->hegiht;  
  log_info("buf_width=%d,hegiht=%d raw_size=%d",buf_width,raw_dscr->hegiht,raw_size);
  u_int8_t  *p_raw=(u_int8_t  *)malloc(raw_size);
  if(!p_raw)
  {
    log_err("malloc err");
    close(fd);
    return ret;
  }
  read(fd,p_raw,raw_size);
  //then conver the raw file buffer to unpacked uint16 raw
  cv::Mat raw_16_img(raw_dscr->hegiht,raw_dscr->width,CV_16UC1);
  int i,j;
  for(i=0;i<raw_dscr->hegiht;i++)
  {
       int line_offset=i*buf_width;   
       p_val=(u_int16_t *)(p_raw+line_offset);
       log_debug("line_offset=%d i=%d",line_offset,i);
       for(j=0;j<raw_dscr->width;j++)
       {
          raw_16_img.at<u_int16_t>(i,j)=p_val[j]<<(16-raw_dscr->bitwidth);
       }   
  }
  ret=raw_16_img;
  close(fd);
  free(p_raw);
  log_func_exit();
  return ret;
}

cv::Mat  convert_raw12_to_hdr_encoder_frame(cv::Mat raw_imge,raw_type_file_dscr_t *raw_dscr)
{    
     raw_imge=raw_imge/16;   
     int raw_encoder_img_height=ALIGN_TO(raw_imge.cols*raw_dscr->bitwidth/8,256);
	cv::Mat raw_encoder_img(raw_imge.rows,raw_encoder_img_height,CV_8UC1);
	//per 32 pix use 32*12/8  byte =48 byte = 12 uint;
	//32 pix 3 * 128
	u_int32_t *p_row=NULL;	
	u_int32_t *p_row_128=NULL;
	u_int32_t  val_src;
	u_int32_t  val_pix;
	for(int i=0;i<raw_imge.rows;i++)
	{	
	   p_row=(u_int32_t *)raw_encoder_img.ptr(i);
	   for(int j=0;j<raw_imge.cols;j+=32)
	   {
	   
	      //first 128 bit=4 int
	      p_row_128=p_row;
	      val_pix=raw_imge.at<u_int16_t>(i,j+0);
	      val_src=AR_GET_REG_BITS(val_pix,0,11);
		 AR_SET_REG_BITS(p_row_128[3],val_src,20,31);
		 
		 val_pix=raw_imge.at<u_int16_t>(i,j+1);
		 val_src=AR_GET_REG_BITS(val_pix,0,11);
		 AR_SET_REG_BITS(p_row_128[3],val_src,8,19);
		 
		 val_pix=raw_imge.at<u_int16_t>(i,j+2);
		 val_src=AR_GET_REG_BITS(val_pix,4,11);		 
		 AR_SET_REG_BITS(p_row_128[3],val_src,0,7);
		 val_src=AR_GET_REG_BITS(val_pix,0,3);		
		 AR_SET_REG_BITS(p_row_128[2],val_src,28,31);
		 
		 val_pix=raw_imge.at<u_int16_t>(i,j+3);
		 val_src=AR_GET_REG_BITS(val_pix,0,11);
		 AR_SET_REG_BITS(p_row_128[2],val_src,16,27);
		 
		 val_pix=raw_imge.at<u_int16_t>(i,j+4);
		 val_src=AR_GET_REG_BITS(val_pix,0,11);
		 AR_SET_REG_BITS(p_row_128[2],val_src,4,15);

		 val_pix=raw_imge.at<u_int16_t>(i,j+5);
		 val_src=AR_GET_REG_BITS(val_pix,8,11);
		 AR_SET_REG_BITS(p_row_128[2],val_src,0,3);		 
		 val_src=AR_GET_REG_BITS(val_pix,0,7);		 
		 AR_SET_REG_BITS(p_row_128[1],val_src,24,31);	

		 val_pix=raw_imge.at<u_int16_t>(i,j+6);
		 val_src=AR_GET_REG_BITS(val_pix,0,11);
		 AR_SET_REG_BITS(p_row_128[1],val_src,12,23);
		 
		 val_pix=raw_imge.at<u_int16_t>(i,j+7);
		 val_src=AR_GET_REG_BITS(val_pix,0,11);
		 AR_SET_REG_BITS(p_row_128[1],val_src,0,11);

		 val_pix=raw_imge.at<u_int16_t>(i,j+8);
	      val_src=AR_GET_REG_BITS(val_pix,0,11);
		 AR_SET_REG_BITS(p_row_128[0],val_src,20,31);
		 
		 val_pix=raw_imge.at<u_int16_t>(i,j+9);
		 val_src=AR_GET_REG_BITS(val_pix,0,11);
		 AR_SET_REG_BITS(p_row_128[0],val_src,8,19);
		 
		 val_pix=raw_imge.at<u_int16_t>(i,j+10);
		 val_src=AR_GET_REG_BITS(val_pix,4,11);		 
		 AR_SET_REG_BITS(p_row_128[0],val_src,0,7);
		 //second 128 bit
		 p_row_128+=4;
		 val_src=AR_GET_REG_BITS(val_pix,0,3);		
		 AR_SET_REG_BITS(p_row_128[3],val_src,28,31);

		 val_pix=raw_imge.at<u_int16_t>(i,j+11);
		 val_src=AR_GET_REG_BITS(val_pix,0,11);
		 AR_SET_REG_BITS(p_row_128[3],val_src,16,27);

		 
		 val_pix=raw_imge.at<u_int16_t>(i,j+12);
		 val_src=AR_GET_REG_BITS(val_pix,0,11);
		 AR_SET_REG_BITS(p_row_128[3],val_src,4,15);
 
		 val_pix=raw_imge.at<u_int16_t>(i,j+13);
		 val_src=AR_GET_REG_BITS(val_pix,8,11);
		 AR_SET_REG_BITS(p_row_128[3],val_src,0,3);
		 val_src=AR_GET_REG_BITS(val_pix,0,7);		 
		 AR_SET_REG_BITS(p_row_128[2],val_src,24,31);

		 val_pix=raw_imge.at<u_int16_t>(i,j+14);
		 val_src=AR_GET_REG_BITS(val_pix,0,11);
		 AR_SET_REG_BITS(p_row_128[2],val_src,12,23);
		 
		 val_pix=raw_imge.at<u_int16_t>(i,j+15);
		 val_src=AR_GET_REG_BITS(val_pix,0,11);
		 AR_SET_REG_BITS(p_row_128[2],val_src,0,11);
		 
		 val_pix=raw_imge.at<u_int16_t>(i,j+16);
		 val_src=AR_GET_REG_BITS(val_pix,0,11);
		 AR_SET_REG_BITS(p_row_128[1],val_src,20,31);
		 
		 val_pix=raw_imge.at<u_int16_t>(i,j+17);
		 val_src=AR_GET_REG_BITS(val_pix,0,11);
		 AR_SET_REG_BITS(p_row_128[1],val_src,8,19);

		 val_pix=raw_imge.at<u_int16_t>(i,j+18);
		 val_src=AR_GET_REG_BITS(val_pix,4,11);
		 AR_SET_REG_BITS(p_row_128[1],val_src,0,7);
		 val_src=AR_GET_REG_BITS(val_pix,0,3);
		 AR_SET_REG_BITS(p_row_128[0],val_src,28,31);

		 val_pix=raw_imge.at<u_int16_t>(i,j+19);
		 val_src=AR_GET_REG_BITS(val_pix,0,11);
		 AR_SET_REG_BITS(p_row_128[0],val_src,16,27);
		 
		 val_pix=raw_imge.at<u_int16_t>(i,j+20);
		 val_src=AR_GET_REG_BITS(val_pix,0,11);
		 AR_SET_REG_BITS(p_row_128[0],val_src,4,15);

		 
		 val_pix=raw_imge.at<u_int16_t>(i,j+21);
		 val_src=AR_GET_REG_BITS(val_pix,8,11);
		 AR_SET_REG_BITS(p_row_128[0],val_src,0,3);
		 p_row_128+=4;
		 val_src=AR_GET_REG_BITS(val_pix,0,7);		 
		 AR_SET_REG_BITS(p_row_128[3],val_src,24,31);

		 
		 val_pix=raw_imge.at<u_int16_t>(i,j+22);
		 val_src=AR_GET_REG_BITS(val_pix,0,11);
		 AR_SET_REG_BITS(p_row_128[3],val_src,12,23);

		 val_pix=raw_imge.at<u_int16_t>(i,j+23);
		 val_src=AR_GET_REG_BITS(val_pix,0,11);
		 AR_SET_REG_BITS(p_row_128[3],val_src,0,11);

		 val_pix=raw_imge.at<u_int16_t>(i,j+24);
		 val_src=AR_GET_REG_BITS(val_pix,0,11);
		 AR_SET_REG_BITS(p_row_128[2],val_src,20,31);

		 val_pix=raw_imge.at<u_int16_t>(i,j+25);
		 val_src=AR_GET_REG_BITS(val_pix,0,11);
		 AR_SET_REG_BITS(p_row_128[2],val_src,8,19);
		 
		 val_pix=raw_imge.at<u_int16_t>(i,j+26);
		 val_src=AR_GET_REG_BITS(val_pix,4,11);
		 AR_SET_REG_BITS(p_row_128[2],val_src,0,7);
		 val_src=AR_GET_REG_BITS(val_pix,0,3);
		 AR_SET_REG_BITS(p_row_128[1],val_src,28,31);

		  val_pix=raw_imge.at<u_int16_t>(i,j+27);
		 val_src=AR_GET_REG_BITS(val_pix,0,11);
		 AR_SET_REG_BITS(p_row_128[1],val_src,16,27);

		 val_pix=raw_imge.at<u_int16_t>(i,j+28);
		 val_src=AR_GET_REG_BITS(val_pix,0,11);
		 AR_SET_REG_BITS(p_row_128[1],val_src,4,15);

		  val_pix=raw_imge.at<u_int16_t>(i,j+29);
		 val_src=AR_GET_REG_BITS(val_pix,8,11);
		 AR_SET_REG_BITS(p_row_128[1],val_src,0,3);
		 val_src=AR_GET_REG_BITS(val_pix,0,7);
		  AR_SET_REG_BITS(p_row_128[0],val_src,24,31);

		 val_pix=raw_imge.at<u_int16_t>(i,j+30);
		 val_src=AR_GET_REG_BITS(val_pix,0,11);
		 AR_SET_REG_BITS(p_row_128[0],val_src,12,23);

		 val_pix=raw_imge.at<u_int16_t>(i,j+31);
		 val_src=AR_GET_REG_BITS(val_pix,0,11);
		 AR_SET_REG_BITS(p_row_128[0],val_src,0,11);		 
		 p_row+=12;
	   }
	}
	return raw_encoder_img;
}


cv::Mat  convert_raw_to_hdr_encoder_frame(cv::Mat raw_imge,raw_type_file_dscr_t *raw_dscr)
{    
     cv::Mat raw_encoder_img;
     if(raw_dscr->bitwidth==12)
	{
          raw_encoder_img=convert_raw12_to_hdr_encoder_frame(raw_imge,raw_dscr);
	}
	return raw_encoder_img;
}

cv::Mat fetch_raw(char *name,raw_type_file_dscr_t *raw_dscr)
{
  log_func_enter();
  cv::Mat ret;
  if(!name)
  {
    log_err("file name null");
	return ret;
  }
  if(raw_dscr->is_packed)
  {
     if(raw_dscr->packed_type==PACKED_TYPE_128BIT){
        ret=read_128bit_mipi_raw_to_dng16_raw(name,raw_dscr);
     }else if(raw_dscr->packed_type==PACKED_TYPE_STD_MIPI){
        ret=read_mipi_raw_to_dng16_raw(name,raw_dscr);
     }
  }else
  {
     ret=read_dng_raw_to_dng16_raw(name,raw_dscr);
  }
  log_func_exit();
  return ret;
}

std::vector<cv::Mat> split_hdr_dol_encoder_raw(char *name,int frame_num,int height,int offset1,int offset2,int offset3,raw_type_file_dscr_t *raw_dscr)
{
    std::vector<cv::Mat> split_raw_img;
    cv::Mat ret;
    u_int8_t *p_val_1=NULL;	
    u_int8_t *p_val_2=NULL;
    if(!name)
    {
      log_err("file name null");
      return ret;
    }
    int fd=open(name,O_RDONLY);
    if(fd<0)
    {
      log_err("open file failed");
      return ret;
    }
    int buf_width=ALIGN_TO(raw_dscr->width*raw_dscr->bitwidth/8,raw_dscr->line_length_algin);  
    int raw_size=buf_width*raw_dscr->hegiht;  
    log_info("buf_width=%d,hegiht=%d raw_size=%d offset1=%d offset2=%d offset3=%d",buf_width,raw_dscr->hegiht,raw_size,offset1,offset2,offset3);
    u_int8_t  *p_raw=(u_int8_t  *)malloc(raw_size);
    if(!p_raw)
    {
      log_err("malloc err");
      close(fd);
      return ret;
    }
    read(fd,p_raw,raw_size);
    //then conver the raw file buffer to unpacked uint16 raw
    if(frame_num==2){
        cv::Mat raw_8_img_1(height,buf_width,CV_8UC1);		
        cv::Mat raw_8_img_2(height,buf_width,CV_8UC1);
	   u_int8_t  *p_raw_1=p_raw+offset1*buf_width;	   
	   u_int8_t  *p_raw_2=p_raw+offset2*buf_width;
        int i,j;
        for(i=0;i<height;i++)
        {
             int line_offset=i*(buf_width*2);   
             p_val_1=(u_int8_t *)(p_raw_1+line_offset);			 
             p_val_2=(u_int8_t *)(p_raw_2+line_offset);
             for(j=0;j<buf_width;j++)
             {
                raw_8_img_1.at<u_int8_t>(i,j)=p_val_1[j];				
                raw_8_img_2.at<u_int8_t>(i,j)=p_val_2[j];
             }   
        }
        split_raw_img.push_back(raw_8_img_1);		
        split_raw_img.push_back(raw_8_img_2);
    }else
    {
        log_err("now only supported 2 frame hdr split");
    }
    close(fd);
    free(p_raw);
    log_func_exit();
    return split_raw_img; 
}

std::vector<cv::Mat> split_hdr_dol_dng_raw(char *name,int frame_num,int height,int offset1,int offset2,int offset3,raw_type_file_dscr_t *raw_dscr)
{
    std::vector<cv::Mat> split_raw_img;
    cv::Mat ret;
    u_int8_t *p_val_1=NULL;	
    u_int8_t *p_val_2=NULL;
    if(!name)
    {
      log_err("file name null");
      return ret;
    }
    int fd=open(name,O_RDONLY);
    if(fd<0)
    {
      log_err("open file failed");
      return ret;
    }
    int buf_width=ALIGN_TO(raw_dscr->width*2,raw_dscr->line_length_algin);	
    int raw_size=buf_width*raw_dscr->hegiht;  
    log_info("buf_width=%d,hegiht=%d raw_size=%d offset1=%d offset2=%d offset3=%d",buf_width,raw_dscr->hegiht,raw_size,offset1,offset2,offset3);
    u_int8_t  *p_raw=(u_int8_t  *)malloc(raw_size);
    if(!p_raw)
    {
      log_err("malloc err");
      close(fd);
      return ret;
    }
    read(fd,p_raw,raw_size);
    //then conver the raw file buffer to unpacked uint16 raw
    if(frame_num==2){
        cv::Mat raw_8_img_1(height,buf_width,CV_8UC1);		
        cv::Mat raw_8_img_2(height,buf_width,CV_8UC1);
	   u_int8_t  *p_raw_1=p_raw+offset1*buf_width;	   
	   u_int8_t  *p_raw_2=p_raw+offset2*buf_width;
        int i,j;
        for(i=0;i<height;i++)
        {
             int line_offset=i*(buf_width*2);   
             p_val_1=(u_int8_t *)(p_raw_1+line_offset);			 
             p_val_2=(u_int8_t *)(p_raw_2+line_offset);
             for(j=0;j<buf_width;j++)
             {
                raw_8_img_1.at<u_int8_t>(i,j)=p_val_1[j];				
                raw_8_img_2.at<u_int8_t>(i,j)=p_val_2[j];
             }   
        }
        split_raw_img.push_back(raw_8_img_1);		
        split_raw_img.push_back(raw_8_img_2);
    }else
    {
        log_err("now only supported 2 frame hdr split");
    }
    close(fd);
    free(p_raw);
    log_func_exit();
    return split_raw_img; 
}

int dump_raw_dng(char *name,cv::Mat img,bayer_format_t bayer,raw_type_file_dscr_t *raw_dscr)
{
	char *f_name=(char *)malloc(128);
	sprintf(f_name,"%s_dng.raw",name);
     cv::Mat raw_16_img(img.rows,img.cols,CV_16UC1);
     int i=0,j=0;
	for(i=0;i<img.rows;i++)
	{
		 for(j=0;j<img.cols;j++)
		 {
			raw_16_img.at<u_int16_t>(i,j)=img.at<u_int16_t>(i,j)>>(16-raw_dscr->bitwidth);
		 }	 
	}
	save_bin(f_name,raw_16_img.data,raw_16_img.cols*raw_16_img.rows*raw_16_img.elemSize());   
	return 0;
}

int dump_raw_bye(char *name,cv::Mat img,bayer_format_t bayer,raw_type_file_dscr_t *raw_dscr)
{
	char *f_name=(char *)malloc(128);
	sprintf(f_name,"%s_byte.raw",name);
	save_bin(f_name,img.data,img.cols*img.rows*img.elemSize());   
	return 0;
}



int dump_raw_to_png(char *name,cv::Mat img,bayer_format_t bayer)
{
  log_func_enter();
  char *f_name=(char *)malloc(128);
  sprintf(f_name,"%s.png",name);
  std::vector<int> compression_params;
  compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION); 
  compression_params.push_back(0);
  log_info("name : %s",f_name);
  cv::imwrite(f_name,img,compression_params); 
  int code=CV_BayerBG2RGB;  
  cv::Mat out;
  switch(bayer)
  {
    case CV_BayerBG:
    code=CV_BayerBG2BGR;
    break;
    case CV_BayerGB:
    code=CV_BayerGB2BGR;
    break;    
    case CV_BayerRG:
    code=CV_BayerRG2BGR;
    break;    
    case CV_BayerGR:
    code=CV_BayerGR2BGR;
    break;
  }
  cv::cvtColor(img,out,code);  
  sprintf(f_name,"%s_demosic.png",name);  
  log_info("name : %s",f_name);
  cv::imwrite(f_name,out,compression_params); 
  free(f_name);
  return 0;
}

int show_raw_image(char *name,cv::Mat img,bayer_format_t bayer)
{
  IplImage imge;
  cv::Mat out;
  int code=CV_BayerBG2RGB;
  log_func_enter();
  cvNamedWindow(name,0);  
  switch(bayer)
  {
    case CV_BayerBG:
    code=CV_BayerBG2BGR;
    break;
    case CV_BayerGB:
    code=CV_BayerGB2BGR;
    break;    
    case CV_BayerRG:
    code=CV_BayerRG2BGR;
    break;    
    case CV_BayerGR:
    code=CV_BayerGR2BGR;
    break;
  }
  cv::cvtColor(img,out,code);
  imge=out;
  cvShowImage(name,&imge);  
  log_func_exit();
  return 0;
}
int dump_to_exr(char *name,char *sub_name,cv::Mat img)
{
  log_func_enter();
  char *f_name=(char *)malloc(128);
  sprintf(f_name,"%s_%s.exr",name,sub_name);
  std::vector<int> compression_params;
  compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION); 
  compression_params.push_back(0);
  log_info("name : %s",f_name);
  cv::imwrite(f_name,img,compression_params);  
  free(f_name);
  return 0;
}

cv::Mat read_from_exr(char *name)
{
  return cv::imread(name,-1);  
}

cv::Mat demosic_raw_image(cv::Mat img,bayer_format_t bayer)
{
  cv::Mat out;
  int code=CV_BayerBG2RGB;
  log_func_enter();
  switch(bayer)
  {
    case CV_BayerBG:
    code=CV_BayerBG2BGR;
    break;
    case CV_BayerGB:
    code=CV_BayerGB2BGR;
    break;    
    case CV_BayerRG:
    code=CV_BayerRG2BGR;
    break;    
    case CV_BayerGR:
    code=CV_BayerGR2BGR;
    break;
  }
  cv::cvtColor(img,out,code);
  log_func_exit();
  return out;
}

raw_ch_t split_raw_ch(cv::Mat img,bayer_format_t bayer)
{
   int i,j,ch_i,ch_j;
   raw_ch_t raw_ch;
   
   cv::Mat tmp1(img.rows/2,img.cols/2,CV_16UC1);   
   cv::Mat tmp2(img.rows/2,img.cols/2,CV_16UC1);
   cv::Mat tmp3(img.rows/2,img.cols/2,CV_16UC1);
   cv::Mat tmp4(img.rows/2,img.cols/2,CV_16UC1);

   log_info("rows=%d, cols=%d",img.rows,img.cols);
   
   raw_ch.raw_ch_00=tmp1;
   raw_ch.raw_ch_01=tmp2;
   raw_ch.raw_ch_10=tmp3;
   raw_ch.raw_ch_11=tmp4;
   
   log_info("rows=%d, cols=%d data=%p",tmp1.rows,tmp1.cols,tmp1.data);
   ch_i=0;
   for(i=0;i<img.rows;)
   {
     ch_j=0;
     log_debug("i=%d, ch_i=%d",i,ch_i);
     for(j=0;j<img.cols;)
     {
       
       log_debug("j=%d, ch_j=%d",j,ch_j);
       raw_ch.raw_ch_00.at<u_int16_t>(ch_i,ch_j)=img.at<u_int16_t>(i+0,j+0);
       raw_ch.raw_ch_01.at<u_int16_t>(ch_i,ch_j)=img.at<u_int16_t>(i+0,j+1);       
       raw_ch.raw_ch_10.at<u_int16_t>(ch_i,ch_j)=img.at<u_int16_t>(i+1,j+0);
       raw_ch.raw_ch_11.at<u_int16_t>(ch_i,ch_j)=img.at<u_int16_t>(i+1,j+1);
       j+=2;
       ch_j++;
     }
     i+=2;
     ch_i++;
   }
   log_func_exit();
   return raw_ch;
}
int merge_raw_ch(raw_ch_t raw_ch,bayer_format_t bayer,cv::Mat img)
{
   int i,j,ch_i,ch_j;
   log_info("rows=%d, cols=%d",img.rows,img.cols);
   ch_i=0;
   for(i=0;i<img.rows;)
   {
     ch_j=0;
     for(j=0;j<img.cols;)
     {
       img.at<u_int16_t>(i+0,j+0)=raw_ch.raw_ch_00.at<u_int16_t>(ch_i,ch_j);
       img.at<u_int16_t>(i+0,j+1)=raw_ch.raw_ch_01.at<u_int16_t>(ch_i,ch_j);
       img.at<u_int16_t>(i+1,j+0)=raw_ch.raw_ch_10.at<u_int16_t>(ch_i,ch_j);
       img.at<u_int16_t>(i+1,j+1)=raw_ch.raw_ch_11.at<u_int16_t>(ch_i,ch_j);
       j+=2;
       ch_j++;
     }
     i+=2;
     ch_i++;
   }
   return 0;
}


cv::Mat get_Gr_raw(raw_ch_t ch,bayer_format_t bayer)
{
  cv::Mat ret;
  switch(bayer)
  {
    case CV_BayerBG:
    ret=ch.raw_ch_10;
    break;
    case CV_BayerGB:
    ret=ch.raw_ch_11;
    break;    
    case CV_BayerRG:
    ret=ch.raw_ch_01;
    break;    
    case CV_BayerGR:
    ret=ch.raw_ch_00;
    break;
  }
  return ret;
}

cv::Mat get_Gb_raw(raw_ch_t ch,bayer_format_t bayer)
{
  cv::Mat ret;
  switch(bayer)
  {
    case CV_BayerBG:
    ret=ch.raw_ch_01;
    break;
    case CV_BayerGB:
    ret=ch.raw_ch_00;
    break;    
    case CV_BayerRG:
    ret=ch.raw_ch_10;
    break;    
    case CV_BayerGR:
    ret=ch.raw_ch_11;
    break;
  }
  return ret;
}

cv::Mat get_R_raw(raw_ch_t ch,bayer_format_t bayer)
{
  cv::Mat ret;
  switch(bayer)
  {
    case CV_BayerBG:
    ret=ch.raw_ch_11;
    break;
    case CV_BayerGB:
    ret=ch.raw_ch_10;
    break;    
    case CV_BayerRG:
    ret=ch.raw_ch_00;
    break;    
    case CV_BayerGR:
    ret=ch.raw_ch_01;
    break;
  }
  return ret;
}

cv::Mat get_B_raw(raw_ch_t ch,bayer_format_t bayer)
{
  cv::Mat ret;
  switch(bayer)
  {
    case CV_BayerBG:
    ret=ch.raw_ch_00;
    break;
    case CV_BayerGB:
    ret=ch.raw_ch_01;
    break;    
    case CV_BayerRG:
    ret=ch.raw_ch_11;
    break;    
    case CV_BayerGR:
    ret=ch.raw_ch_10;
    break;
  }
  return ret;
}

