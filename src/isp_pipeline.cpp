#include "isp_pipeline.h"
#include "plot.h"
#include "func_def.h"
//#define DEBUG_MIPI_RAW

typedef struct
{
  raw_type_file_dscr_t raw_dscr;
  isp_pra_t *isp_pra;
  raw_ch_t ch;  
  raw_hist_t raw_hsit;
  cv::Mat raw_imag; //fetch raw will return the image    
  cv::Mat blc_imag; //store the img after blc
  cv::Mat lsc_img; //store the img after lsc
}isp_t;

typedef struct
{
   const char *cmd;
   const char *help[64];
   void (*process_cmd)(int argc, char *argv[]);
}process_cmd_t;

static isp_t *p_isp_server=NULL;

static isp_t *get_isp(void)
{
  return p_isp_server;
}
static int process_fetch_raw(char *name)
{
   int i=0;
   char f_name[128];
   isp_t *p_isp=get_isp();  
   p_isp->raw_imag=fetch_raw(name,&p_isp->raw_dscr); 
   sprintf(f_name,"%s_fetch",name);
   dump_raw_to_png(f_name,p_isp->raw_imag,p_isp->raw_dscr.bayer_format);   
   dump_raw_dng(f_name,p_isp->raw_imag,p_isp->raw_dscr.bayer_format,&p_isp->raw_dscr);
   dump_raw_bye(f_name, p_isp->raw_imag,p_isp->raw_dscr.bayer_format,&p_isp->raw_dscr);
} 

static cv::Mat  process_fetch_raw_and_crop(char *name,int x,int y,int w,int h)
{
   int i=0;
   char f_name[128];   
   cv::Mat crop_raw_imag; //fetch raw will return the image	  
   isp_t *p_isp=get_isp();  
   p_isp->raw_imag=fetch_raw(name,&p_isp->raw_dscr); 
   sprintf(f_name,"%s_fetch",name);
   dump_raw_to_png(f_name,p_isp->raw_imag,p_isp->raw_dscr.bayer_format);   
   dump_raw_dng(f_name,p_isp->raw_imag,p_isp->raw_dscr.bayer_format,&p_isp->raw_dscr);
   //crop the raw
   crop_raw_imag= p_isp->raw_imag(Rect(x,y,w,h));      
   sprintf(f_name,"%s_fetch_crop_%d_%d_%d_%d",name,x,y,w,h);
   dump_raw_to_png(f_name,crop_raw_imag,p_isp->raw_dscr.bayer_format);   
   dump_raw_dng(f_name,crop_raw_imag,p_isp->raw_dscr.bayer_format,&p_isp->raw_dscr);
   return crop_raw_imag;
} 

static cv::Mat  process_fetch_raw_and_scaler_to(char *name,int w,int h)
{
	int i=0;	
	isp_t *p_isp=get_isp();  
	char f_name[128];	
	cv::Mat crop_raw_imag; //fetch raw will return the image   
	int x,y,mid_w,mid_h;	
	cv::Mat imag(h,w,CV_16UC1);		
	//fetch raw
	p_isp->raw_imag=fetch_raw(name,&p_isp->raw_dscr); 	
	sprintf(f_name,"%s_fetch",name);
	dump_raw_to_png(f_name,p_isp->raw_imag,p_isp->raw_dscr.bayer_format);	
	dump_raw_dng(f_name,p_isp->raw_imag,p_isp->raw_dscr.bayer_format,&p_isp->raw_dscr);
	//crop
     float src_w=p_isp->raw_imag.cols;
	float src_h=p_isp->raw_imag.rows;
     float out_ration=(float)w/(float)h;
	float in_ration=src_w/src_h;
	if(in_ration>out_ration)
	{
	   mid_w=src_h*out_ration;
	   mid_h=src_h;
	}else
	{
	  mid_w=src_w;
	  mid_h=src_w/out_ration;
	}
	x=(src_w-mid_w)/2;
	y=(src_h-mid_h)/2;	

	log_info("xywh = %d %d %d %d",x,y,mid_w,mid_h);
	crop_raw_imag= p_isp->raw_imag(Rect(x,y,mid_w,mid_h));		
	//split and scaler
	raw_ch_t raw_ch_orig;	
	raw_ch_t raw_ch_dest;
	raw_ch_orig=split_raw_ch(crop_raw_imag,p_isp->raw_dscr.bayer_format);
	cv::resize(raw_ch_orig.raw_ch_00,raw_ch_dest.raw_ch_00,Size(w/2,h/2),0,0,INTER_CUBIC);	
	cv::resize(raw_ch_orig.raw_ch_01,raw_ch_dest.raw_ch_01,Size(w/2,h/2),0,0,INTER_CUBIC);	
	cv::resize(raw_ch_orig.raw_ch_10,raw_ch_dest.raw_ch_10,Size(w/2,h/2),0,0,INTER_CUBIC);		
	cv::resize(raw_ch_orig.raw_ch_11,raw_ch_dest.raw_ch_11,Size(w/2,h/2),0,0,INTER_CUBIC);	
	merge_raw_ch(raw_ch_dest,p_isp->raw_dscr.bayer_format,imag);
	sprintf(f_name,"%s_fetch_scaler_%d_%d",name,w,h);
	dump_raw_to_png(f_name,imag,p_isp->raw_dscr.bayer_format);   
	dump_raw_dng(f_name,imag,p_isp->raw_dscr.bayer_format,&p_isp->raw_dscr);
	return imag;
}

static int convert_raw_2_hdr_encoder_frame(char *name)
{
    isp_t *p_isp=get_isp(); 
    char f_name[128];
    cv::Mat raw_imag=fetch_raw(name,&p_isp->raw_dscr); 	
    show_and_save_img(name,(char *)"input",raw_imag);
    cv::Mat hdr_encoder_raw=convert_raw_to_hdr_encoder_frame(raw_imag,&p_isp->raw_dscr);	
    sprintf(f_name,"%s_2",name);
    dump_raw_bye(f_name, hdr_encoder_raw,p_isp->raw_dscr.bayer_format,&p_isp->raw_dscr);
}
static int process_get_blc_pra(char *name)
{
   int i=0;  
   cv::Mat raw_imag;  
   isp_t *p_isp=get_isp(); 
   raw_imag=fetch_raw(name,&p_isp->raw_dscr); 
   p_isp->ch=split_raw_ch(raw_imag,p_isp->raw_dscr.bayer_format);
   p_isp->isp_pra->blc_pra=get_blc_value(p_isp->ch,p_isp->raw_dscr.bayer_format);
   save_isp_pra(p_isp->isp_pra,sizeof(isp_pra_t));
   ofstream blc_h("blc_pra.h");
   blc_h << "/*blc_pra_t */"<<endl;
   blc_h << "{ "<<endl;
   blc_h << "   /* u_int16_t r_blc;*/"<<endl;
   blc_h << "   "<<dec<<p_isp->isp_pra->blc_pra.r_blc<<","<<endl;
   blc_h << "   /* u_int16_t gr_blc;*/"<<endl;
   blc_h << "   "<<dec<<p_isp->isp_pra->blc_pra.gr_blc<<","<<endl;
   blc_h << "   /* u_int16_t gb_blc;*/"<<endl;
   blc_h << "   "<<dec<<p_isp->isp_pra->blc_pra.gb_blc<<","<<endl;
   blc_h << "   /* u_int16_t b_blc;*/"<<endl;
   blc_h << "   "<<dec<<p_isp->isp_pra->blc_pra.b_blc<<","<<endl;
   blc_h << "}"<<endl;
   blc_h.close();
   return 0;
}
static int process_proc_blc(char *name)
{
   int i=0;   
   cv::Mat raw_imag; 
   char f_name[128];
   isp_t *p_isp=get_isp();
   p_isp->ch=split_raw_ch(p_isp->raw_imag,p_isp->raw_dscr.bayer_format);
   p_isp->ch=process_blc(p_isp->ch,p_isp->isp_pra->blc_pra,p_isp->raw_dscr.bayer_format);
   merge_raw_ch(p_isp->ch,p_isp->raw_dscr.bayer_format,p_isp->blc_imag); 
   sprintf(f_name,"%s_blc",name);
   dump_raw_to_png(f_name,p_isp->blc_imag,p_isp->raw_dscr.bayer_format);
}
static int process_get_hist(char *name)
{
   int i=0;   
   cv::Mat raw_imag;  
   isp_t *p_isp=get_isp(); 
   raw_imag=fetch_raw(name,&p_isp->raw_dscr); 
   p_isp->ch=split_raw_ch(raw_imag,p_isp->raw_dscr.bayer_format);
   p_isp->raw_hsit=get_raw_hist(128,p_isp->ch,p_isp->raw_dscr.bayer_format);
   save_raw_hist(name,&p_isp->raw_hsit);
   return 0;
}

static int process_plot_raw_hist(char *name)
{ 
   cv::Mat raw_imag;
   cv::Mat ch_img; 
   isp_t *p_isp=get_isp(); 
   raw_imag=fetch_raw(name,&p_isp->raw_dscr); 
   p_isp->ch=split_raw_ch(raw_imag,p_isp->raw_dscr.bayer_format);

   plot_hist(name,"r_hist",get_R_raw(p_isp->ch,p_isp->raw_dscr.bayer_format),0,256<<8);
   plot_hist(name,"gr_hist",get_Gr_raw(p_isp->ch,p_isp->raw_dscr.bayer_format),0,256<<8);
   plot_hist(name,"gb_hist",get_Gb_raw(p_isp->ch,p_isp->raw_dscr.bayer_format),0,256<<8);
   plot_hist(name,"b_hist",get_B_raw(p_isp->ch,p_isp->raw_dscr.bayer_format),0,256<<8);
   return 0;
}


static int process_raw_haar_denoise(char *name,int is_multi)
{
    cv::Mat raw_imag;
    cv::Mat ch_img; 
    isp_t *p_isp=get_isp(); 
    raw_imag=fetch_raw(name,&p_isp->raw_dscr); 
    p_isp->ch=split_raw_ch(raw_imag,p_isp->raw_dscr.bayer_format);
    ch_img=get_R_raw(p_isp->ch,p_isp->raw_dscr.bayer_format);    
    show_and_save_img(name,(char *)"input",ch_img);
    
    cv::Mat ch_img_float=get_u16_float_img(ch_img);
    cv::Mat wave1_img=haar_wavelet_decompose(ch_img_float,1,is_multi);
    cv::Mat wave1_u16_img=get_float_u16_img(wave1_img);    
    show_and_save_img(name,(char *)"wave_1_layer",wave1_u16_img);

    #if 1
    cv::Mat wave1_sub_img_0=get_sub_wave_img(wave1_img,0);
    cv::Mat wave2_img=haar_wavelet_decompose(wave1_sub_img_0,1,is_multi);
    cv::Mat wave2_u16_img=get_float_u16_img(wave2_img);    
    show_and_save_img(name,(char *)"wave_2_layer",wave2_u16_img);
 
    cv::Mat wave2_sub_img_0=get_sub_wave_img(wave2_img,0);
    cv::Mat wave3_img=haar_wavelet_decompose(wave2_sub_img_0,1,is_multi);
    cv::Mat wave3_u16_img=get_float_u16_img(wave3_img);    
    show_and_save_img(name,(char *)"wave_3_layer",wave3_u16_img);

    
    cv::Mat wave3_sub_img_0=get_sub_wave_img(wave3_img,0);
    cv::Mat wave4_img=haar_wavelet_decompose(wave3_sub_img_0,1,is_multi);
    cv::Mat wave4_u16_img=get_float_u16_img(wave4_img);    
    show_and_save_img(name,(char *)"wave_4_layer",wave4_u16_img);


    cv::Mat wave4_sub_img_0=get_sub_wave_img(wave4_img,0);
    cv::Mat wave5_img=haar_wavelet_decompose(wave4_sub_img_0,1,is_multi);
    cv::Mat wave5_u16_img=get_float_u16_img(wave5_img);    
    show_and_save_img(name,(char *)"wave_5_layer",wave5_u16_img);


    cv::Mat wave5_sub_img_0=get_sub_wave_img(wave5_img,0);
    cv::Mat wave6_img=haar_wavelet_decompose(wave5_sub_img_0,1,is_multi);
    cv::Mat wave6_u16_img=get_float_u16_img(wave6_img);    
    show_and_save_img(name,(char *)"wave_6_layer",wave6_u16_img);
    #endif
    return 0;
}

static int process_hdr_merge(char *name)
{
   cv::Mat raw_imag;
   cv::Mat ch_img; 
   isp_t *p_isp=get_isp(); 
   raw_imag=fetch_raw(name,&p_isp->raw_dscr); 
   p_isp->ch=split_raw_ch(raw_imag,p_isp->raw_dscr.bayer_format);
   ch_img=get_R_raw(p_isp->ch,p_isp->raw_dscr.bayer_format);    
   show_and_save_img(name,(char *)"r_ch",ch_img);
   cv::Mat l_img=get_long_exp_img(ch_img,1080/2,8);   
   cv::Mat s_img=get_short_exp_img(ch_img,1080/2,9);
   show_and_save_img(name,(char *)"r_ch_long",l_img);   
   show_and_save_img(name,(char *)"r_ch_short",s_img);
   hdr_merge_pra_t hdr_pra;
   hdr_pra.long_exp_th=240<<8;   
   hdr_pra.long_exp_th_low=200<<8;
   cv::Mat merg_img=merge_hdr_img(l_img,s_img,4,&hdr_pra);
   drc_pra_t drc_pra;
   drc_pra.methed=DRC_GAMMA;
   //drc_pra.methed=DRC_GAMMA;
   drc_pra.gamma=3.0;
   drc_pra.k1=1;
   drc_pra.k2=8;
   drc_pra.k3=0.4;
   cv::Mat hdr_img=drc_hdr_img(merg_img,4,&drc_pra);   
   show_and_save_img(name,(char *)"r_ch_hdr",hdr_img);
}

static int process_hdr_dol_split(char *name)
{
	cv::Mat raw_imag;
	isp_t *p_isp=get_isp(); 
	raw_imag=fetch_raw(name,&p_isp->raw_dscr); 	
	show_and_save_img(name,(char *)"raw_imag",raw_imag);
	cv::Mat l_img=get_long_exp_img(raw_imag,1080,14);   
	cv::Mat s_img=get_short_exp_img(raw_imag,1080,25);
	s_img=s_img*4;
	show_and_save_img(name,(char *)"l_img",l_img);	 
	show_and_save_img(name,(char *)"s_img",s_img);
     return 0;
}

static int  process_hdr_merge_kang2014(char *name1,char *name2,char *name3,char *name4,int frame_num)
{
   cv::Mat raw_imge1;   
   cv::Mat raw_imge2;
   cv::Mat raw_imge3;
   cv::Mat raw_imge4;
   float ration[3]={16,16,16};
   isp_t *p_isp=get_isp(); 
   raw_imge1=fetch_raw(name1,&p_isp->raw_dscr);    
   raw_imge2=fetch_raw(name2,&p_isp->raw_dscr); 
   raw_imge3=fetch_raw(name3,&p_isp->raw_dscr); 
   raw_imge4=fetch_raw(name4,&p_isp->raw_dscr); 
   raw_imge1=process_blc_sample(raw_imge1,16<<8);
   raw_imge2=process_blc_sample(raw_imge2,16<<8);
   raw_imge3=process_blc_sample(raw_imge3,16<<8);
   raw_imge4=process_blc_sample(raw_imge4,16<<8);

   #if 0
   cv::Mat raw_imge3_float;
   raw_imge3.convertTo(raw_imge3_float,CV_32FC1,1);
   cv::Mat raw_imge3_float_cfa=isp_cfa_3x3_region(raw_imge3_float,p_isp->raw_dscr.bayer_format);   
   cv::Mat  raw_imge3_rgb;
   raw_imge3_float_cfa.convertTo(raw_imge3_rgb,CV_16UC3,1);
   show_and_save_img(name3,(char *)"raw_imge3_rgb",raw_imge3_rgb);   
   cv::Mat  opencv_cfa;   
   cv::cvtColor(raw_imge3,opencv_cfa,CV_BayerBG2BGR);	   
   show_and_save_img(name3,(char *)"opencv_cfa",opencv_cfa);   
   #endif
   #if 1
   hdr_merge_pra_kang2014_t hdr_pra;
   hdr_pra.rou=1/3.0*0xffff;
   hdr_pra.beta=0;  //blc level have 
   hdr_pra.deta=0.01;
   hdr_pra.frame_num=frame_num;
   //merg_img 32bit float
   cv::Mat merg_img=merge_hdr_img_kang2014(raw_imge1,raw_imge2,raw_imge3,raw_imge4,ration,&hdr_pra);   
   cv::Mat merg_img_demosic=isp_cfa_3x3_region(merg_img,p_isp->raw_dscr.bayer_format);
   
   //only exr format supported float format or 32int
   //dump_to_exr(name1,(char *)"kanghdr_demosic",merg_img);
   cv::Mat hdr_img=drc_hdr_img_kang2014_gamma(merg_img,2.0);    
   cv::Mat hdr_img_demosic=drc_hdr_img_kang2014_gamma_rgb(merg_img_demosic,4.0); 
   //cv::Mat hdr_img_line=drc_hdr_img_kang2014_linear(merg_img);
   show_and_save_img(name1,(char *)"kanghdr",hdr_img);   
   show_and_save_img(name1,(char *)"kanghdr_demosic_2.0_gamma",hdr_img_demosic);      
   // show_and_save_img(name1,(char *)"hdr_img_line",hdr_img_line);   

   cv::Mat hdr_img_bilter=drc_hdr_img_kang2014_bilateral(merg_img_demosic);
   
   //cv::Mat hdr_img_bilter=drc_hdr_img_kang2014_local_bilateral(merg_img_demosic);
   cv::Mat hdr_img_bilter_8bit;
   hdr_img_bilter.convertTo(hdr_img_bilter_8bit,CV_8UC3,1);   
   char name_frame[128];
   sprintf(name_frame,"hdr_img_bilter_8bit_%d",frame_num);
   show_and_save_img(name1,(char *)name_frame,hdr_img_bilter_8bit);	   
 
   dump_raw_to_png(name1,hdr_img,p_isp->raw_dscr.bayer_format);
   #endif
}

Mat addGaussianNoise(Mat &srcImag,float avg,float std);
static int  process_add_noise(char *name1,int avg,int std)
{
   char name[256]="";
   cv::Mat src_img=imread(name1,CV_8U);
   float sigma=(float)std/255.0;
   float avg_f=(float)avg/255.0;
   log_info("name1=%s avg=%d std=%d src_img.chans=%d",name1,avg,std,src_img.channels());
   cv::Mat noise_img=addGaussianNoise(src_img,avg_f,sigma);
   sprintf(name,"noise_avg_%d_%d",avg,std);
   show_and_save_img(name1,(char *)name,noise_img);  
   return 0;
}
int save_yuv_img_planner(char *picname,char *func_name,vector<Mat> channels);
static int  process_rgb_to_yuv(char *name1)
{
   char name[256]="";
   cv::Mat src_img=imread(name1);
   cv::Mat yuv_img;   
   vector<Mat> channels;

   sprintf(name,"w_%d_h_%d.rgb",src_img.cols,src_img.rows);
   show_and_save_img(name1,(char *)name,src_img); 

   sprintf(name,"w_%d_h_%d.yuv",src_img.cols,src_img.rows);
   cv::cvtColor(src_img,yuv_img,CV_BGR2YUV); 
   show_and_save_img(name1,(char *)name,yuv_img); 
   save_yuv_img(name1,name,yuv_img);
   split(yuv_img,channels);
   save_yuv_img_planner(name1,name,channels);
   
   sprintf(name,"w_%d_h_%d.y",src_img.cols,src_img.rows);
   show_and_save_img(name1,(char *)name,channels.at(0));  
   
   sprintf(name,"w_%d_h_%d.u",src_img.cols,src_img.rows);
   show_and_save_img(name1,(char *)name,channels.at(1));  

   sprintf(name,"w_%d_h_%d.v",src_img.cols,src_img.rows);
   show_and_save_img(name1,(char *)name,channels.at(2));  
   return 0;
}

static vector<Mat> get_all_mesh_block(char *name1,int mesh_size,int step)
{
  int i=0;
  int j=0;
  vector<Mat> all_block;
  cv::Mat sub_mesh_block;
  char name[256]="";
  cv::Mat src_img=imread(name1);
  int block_w=(src_img.cols-mesh_size)/step;
  int block_h=(src_img.rows-mesh_size)/step;  
  log_info("we will construct %d block with step=%d mesh=%d",block_w*block_h,step,mesh_size);
  for(i=0;i<block_h;i++)
  {
      for(j=0;j<block_w;j++)
      {         
         //log_info("[%d][%d]",i,j);
         sub_mesh_block=src_img(Rect(j*step,i*step,mesh_size,mesh_size));
         all_block.push_back(sub_mesh_block); 
         //sprintf(name,"_%d_%d_",i,j);
         //save_img(name1,(char *)name,sub_mesh_block);   
         //show_and_save_img(name1,(char *)name,sub_mesh_block);  
      }
  }
  return all_block;
}


static vector<Mat> get_all_mesh_block(char *name1,int mesh_size)
{
  int i=0;
  int j=0;
  vector<Mat> all_block;
  cv::Mat sub_mesh_block;
  char name[256]="";
  cv::Mat src_img=imread(name1);
  int block_w=src_img.cols/mesh_size;
  int block_h=src_img.rows/mesh_size;
  log_info("we will construct %d block with step=%d mesh_size=%d",block_w*block_h,mesh_size,mesh_size);
  for(i=0;i<block_h;i++)
  {
      for(j=0;j<block_w;j++)
      {         
         //log_info("[%d][%d]",i,j);
         sub_mesh_block=src_img(Rect(j*mesh_size,i*mesh_size,mesh_size,mesh_size));
         all_block.push_back(sub_mesh_block); 
         //sprintf(name,"_%d_%d_",i,j);
         //save_img(name1,(char *)name,sub_mesh_block);   
         //show_and_save_img(name1,(char *)name,sub_mesh_block);  
      }
  }
  return all_block;
}

static vector<Mat> get_match_block(Mat ref_block,vector<Mat> all_block,int th)
{
   char name[256]="";
   vector<Mat> ref_match;
   double odis=0;
   int i=0;
   int block_count=0;
   CvMat ref=ref_block;
   CvMat all;
   for(i=0;i<all_block.size();i++)
   {
       all=all_block[i];       
       odis=cvNorm(&ref, &all, CV_L2);
       //log_info("odis=%f",odis);
       if(odis<th&&odis!=0)
       {
          block_count++;
          ref_match.push_back(all_block[i]);
          sprintf(name,"_%d_dis=%d_",i,(int)odis);
          //save_img((char *)"../file/pic/mesh/ref_match",(char *)name,all_block[i]);    
       }
   }
   log_info("match block_count=%d in %ld",block_count,all_block.size());
   return ref_match;
}
static int paint_match_block_to_orig(char *name1,Mat ref_block,vector<Mat> match_block)
{  
     int i=0;
     cv::Mat src_img=imread(name1);
     int w=ref_block.cols;
     int h=ref_block.rows;
     Size wholeSize;
     Point ofs;
     ref_block.locateROI(wholeSize,ofs);             
     rectangle(src_img,Rect(ofs.x,ofs.y,w,h),Scalar(0,0,255),1,8,0);        
     for(i=0;i<match_block.size();i++)         
     {         
        match_block[i].locateROI(wholeSize,ofs);        
        rectangle(src_img,Rect(ofs.x,ofs.y,w,h),Scalar(255,0,0),1,8,0);        
     }
     save_img(name1,(char *)"with_rect",src_img);    
}

static int process_dct(char *name1)
{
   char name[256]="";
   cv::Mat src_img=imread(name1,CV_8U);
   src_img.convertTo(src_img, CV_32F, 1.0/255);
   Mat src_dct;
   dct(src_img, src_dct);
   sprintf(name,"dct");
   show_and_save_img(name1,(char *)name,abs(src_dct));  
   //we print the dect mat
   //cout<<endl<<src_dct(Rect(0,0,64,64))<<endl<<endl;
   
   return 0;
}

static int process_haar_denoise(char *name1,int is_multi)
{
    char name[256]="";
    cv::Mat src_img=imread(name1,CV_8U);
    src_img.convertTo(src_img, CV_32F, 1.0/255);
    cv::Mat wave1_img=haar_wavelet_decompose(src_img,1,is_multi); 
    cv::Mat wave1_sub_img_0=get_sub_wave_img(wave1_img,0);
    wave1_img=abs(wave1_img);
    wave1_img.convertTo(wave1_img, CV_8U, 255);
    sprintf(name,"haar");    
    show_and_save_img(name1,(char *)name,wave1_img);  

    cv::Mat wave2_img=haar_wavelet_decompose(wave1_sub_img_0,1,is_multi); 
    cv::Mat wave2_sub_img_0=get_sub_wave_img(wave2_img,0);
    wave2_img=abs(wave2_img);
    wave2_img.convertTo(wave2_img, CV_8U, 255);
    sprintf(name,"haar2");    
    show_and_save_img(name1,(char *)name,wave2_img);  

    
    cv::Mat wave3_img=haar_wavelet_decompose(wave2_sub_img_0,1,is_multi); 
    cv::Mat wave3_sub_img_0=get_sub_wave_img(wave3_img,0);
    wave3_img=abs(wave3_img);
    wave3_img.convertTo(wave3_img, CV_8U, 255);
    sprintf(name,"haar3");    
    show_and_save_img(name1,(char *)name,wave3_img);  


    cv::Mat wave4_img=haar_wavelet_decompose(wave3_sub_img_0,1,is_multi); 
    cv::Mat wave4_sub_img_0=get_sub_wave_img(wave4_img,0);
    wave4_img=abs(wave4_img);
    wave4_img.convertTo(wave4_img, CV_8U, 255);
    sprintf(name,"haar4");    
    show_and_save_img(name1,(char *)name,wave4_img); 


    cv::Mat wave5_img=haar_wavelet_decompose(wave4_sub_img_0,1,is_multi); 
    cv::Mat wave5_sub_img_0=get_sub_wave_img(wave5_img,0);
    wave5_img=abs(wave5_img);
    wave5_img.convertTo(wave5_img, CV_8U, 255);
    sprintf(name,"haar5");    
    show_and_save_img(name1,(char *)name,wave5_img); 

    
    cv::Mat wave6_img=haar_wavelet_decompose(wave5_sub_img_0,1,is_multi); 
    wave6_img=abs(wave6_img);
    wave6_img.convertTo(wave6_img, CV_8U, 255);
    sprintf(name,"haar6");    
    show_and_save_img(name1,(char *)name,wave6_img); 

}

int save_mat_to_bin(char *picname,char *func_name,Mat img);
static int process_convert(char *name1,char *to,int save_to_bin,int o_w,int o_h)
{
   char name[256]="";
   cv::Mat src_img=imread(name1);
   Mat dst_img;
   string to_(to);
   Mat scaler_img;

   if(o_w&&o_h)
   {     
     cv::resize(src_img,scaler_img,Size(o_w,o_h));
   }
   show_and_save_img(name1,(char *)"scaler img",scaler_img); 
   
   if(to_=="rgb565"){
       cv::cvtColor(scaler_img,dst_img,CV_BGR2BGR565); 
   }else if(to_=="rgb1555")
   {
       cv::cvtColor(scaler_img,dst_img,CV_BGR2BGR555); 
       //we add the alfa 1 to the picture
       int i=0,j=0;
       for(i=0;i<dst_img.rows;i++)
       {
          for(j=0;j<dst_img.cols;j++)
          {
            if((j<dst_img.cols/4)||j>dst_img.cols/4*3)
            {
                dst_img.ptr<u_int16_t>(i)[j]=dst_img.ptr<u_int16_t>(i)[j]|0x8000;
            }else
            {               
                dst_img.ptr<u_int16_t>(i)[j]=dst_img.ptr<u_int16_t>(i)[j]&(~0x8000);
            }
          }
       }
   }else
   {
      cout<<"err not supported str:"<<to_<<endl;
   }
   if(o_w&&o_h)
   {    
       sprintf(name,"%s_%d_%d",to_.c_str(),o_w,o_h);
       if(save_to_bin)
       {
           save_mat_to_bin(name1,name,dst_img);
       }
   }else
   {
       sprintf(name,"%s",to_.c_str());
       if(save_to_bin)
       {
           save_mat_to_bin(name1,name,dst_img);
       }
   }
   return 0;
}


static int get_arg_index_by_name(const char *name, int argc,char *argv[])
{
     int i=0;
     for(i=0;i<argc;i++)
     {
         if(!strcmp(name,argv[i]))
         {
            log_debug("find pra %s @ %d",name,i);
            return i;
         }
     }
     return -1;    
}

void process_cmd_fetch_raw(int argc, char *argv[])
{
    isp_t *p_isp=NULL;
    char f_name[128];	
    p_isp=get_isp();
    int arg_index=get_arg_index_by_name("--fetch_raw",argc,argv);
    if(arg_index>0)
    {
        if((arg_index+1)<=(argc-1)){
           process_fetch_raw(argv[arg_index+1]);           
           sprintf(f_name,"%s_fetch",argv[arg_index+1]);
           show_raw_image(f_name,p_isp->raw_imag,p_isp->raw_dscr.bayer_format);
        }else
        {
           log_err("too less pra for fetch raw");
        }
    }
}
void process_cmd_fetch_raw_and_crop(int argc, char *argv[])
{
     isp_t *p_isp=NULL;
     char f_name[128];	
     char f_name_1[128];	
     p_isp=get_isp();
     int arg_index=0;
     arg_index=get_arg_index_by_name("--fetch_raw_and_crop",argc,argv);
	 if(arg_index>0)
	 {
		 if((arg_index+1)<=(argc-1)){			 
		 sprintf(f_name,"%s_fetch",argv[arg_index+1]);		 
		 sprintf(f_name_1,"%s_fetch_crop",argv[arg_index+1]);
		 int x=atoi(argv[arg_index+2]);
		 int y=atoi(argv[arg_index+3]);
		 int w=atoi(argv[arg_index+4]); 	 
		 int h=atoi(argv[arg_index+5]);  
		 log_info("xywh(%d %d %d %d)",x,y,w,h);
		 cv::Mat crop_raw_imag; //fetch raw will return the image	
			crop_raw_imag=process_fetch_raw_and_crop(argv[arg_index+1],x,y,w,h);		   
			show_raw_image(f_name,p_isp->raw_imag,p_isp->raw_dscr.bayer_format);		
			show_raw_image(f_name_1,crop_raw_imag,p_isp->raw_dscr.bayer_format);
		 }else
		 {
			log_err("too less pra for fetch raw");
		 }
	 }

}
void process_cmd_save_raw_dsc(int argc, char *argv[])
{
	isp_t *p_isp=NULL;
	char f_name[128];  
	char f_name_1[128];    
	p_isp=get_isp();
	int arg_index=0;
     arg_index=get_arg_index_by_name("--save_raw_dsc",argc,argv);
     if(arg_index>0)
     {
       int sub_arg_index=0;
       sub_arg_index=get_arg_index_by_name("-w",argc,argv);
       if(sub_arg_index>0){
          p_isp->raw_dscr.width=atoi(argv[sub_arg_index+1]);
       }
       sub_arg_index=get_arg_index_by_name("-h",argc,argv);
       if(sub_arg_index>0){
          p_isp->raw_dscr.hegiht=atoi(argv[sub_arg_index+1]);
       }
       sub_arg_index=get_arg_index_by_name("-stride",argc,argv);
       if(sub_arg_index>0){
          p_isp->raw_dscr.line_length_algin=atoi(argv[sub_arg_index+1]);
       }
       sub_arg_index=get_arg_index_by_name("-bayer",argc,argv);
       if(sub_arg_index>0){
          p_isp->raw_dscr.bayer_format=(bayer_format_t)atoi(argv[sub_arg_index+1]);
       }
       sub_arg_index=get_arg_index_by_name("-bit",argc,argv);
       if(sub_arg_index>0){
          p_isp->raw_dscr.bitwidth=atoi(argv[sub_arg_index+1]);
       }
       sub_arg_index=get_arg_index_by_name("-dng",argc,argv);
       if(sub_arg_index>0)
       {
          p_isp->raw_dscr.is_packed=0;
       }else
       {
          p_isp->raw_dscr.is_packed=1;
       }
       p_isp->raw_dscr.packed_type=PACKED_TYPE_128BIT;
       sub_arg_index=get_arg_index_by_name("-packed_type",argc,argv);
       if(sub_arg_index>0)
       {
          p_isp->raw_dscr.packed_type=atoi(argv[sub_arg_index+1]);;
       }
       save_bin((char *)"raw_dsc.bin",&p_isp->raw_dscr,sizeof(p_isp->raw_dscr)); 
    }
}
void process_cmd_fetch_raw_and_scaler(int argc, char *argv[])
{
	isp_t *p_isp=NULL;
	char f_name[128];  
	char f_name_1[128];    
	p_isp=get_isp();
	int arg_index=0;
    arg_index=get_arg_index_by_name("--fetch_raw_and_scaler",argc,argv);
    if(arg_index>0)
    {
        if((arg_index+1)<=(argc-1)){			
		sprintf(f_name,"%s_fetch",argv[arg_index+1]);		
		sprintf(f_name_1,"%s_fetch_scaler",argv[arg_index+1]);
		int w=atoi(argv[arg_index+2]);		
		int h=atoi(argv[arg_index+3]);	
		log_info("wh(%d %d)",w,h);
		cv::Mat raw_imag; //fetch raw will return the image   
           raw_imag=process_fetch_raw_and_scaler_to(argv[arg_index+1],w,h);           
           show_raw_image(f_name,p_isp->raw_imag,p_isp->raw_dscr.bayer_format);		   
           show_raw_image(f_name_1,raw_imag,p_isp->raw_dscr.bayer_format);
        }else
        {
           log_err("too less pra for fetch raw");
        }
    }

}
void process_cmd_get_blc_pra(int argc, char *argv[])
{
	isp_t *p_isp=NULL;
	char f_name[128];  
	char f_name_1[128];    
	p_isp=get_isp();
	int arg_index=0;
    arg_index=get_arg_index_by_name("--get_blc_pra",argc,argv);
    if(arg_index>0)
    {
        if((arg_index+1)<=(argc-1)){
           process_get_blc_pra(argv[arg_index+1]);
        }else
        {
           log_err("too less pra for get_blc_pra");
        }
    }

}
void process_cmd_get_raw_his(int argc, char *argv[])
{
	isp_t *p_isp=NULL;
	char f_name[128];  
	char f_name_1[128];    
	p_isp=get_isp();
	int arg_index=0;
    arg_index=get_arg_index_by_name("--get_blc_pra",argc,argv);
    if(arg_index>0)
    {
        if((arg_index+1)<=(argc-1)){
           process_get_blc_pra(argv[arg_index+1]);
        }else
        {
           log_err("too less pra for get_blc_pra");
        }
    }

}
void process_cmd_plot_raw_hist(int argc, char *argv[])
{
	isp_t *p_isp=NULL;
	char f_name[128];  
	char f_name_1[128];    
	p_isp=get_isp();
	int arg_index=0;
    arg_index=get_arg_index_by_name("--plot_raw_hist",argc,argv);
    if(arg_index>0)
    {
        if((arg_index+1)<=(argc-1)){
           process_plot_raw_hist(argv[arg_index+1]);
        }else
        {
           log_err("too less pra for plot_raw_hist");
        }
    }

}
void process_cmd_raw_denoise(int argc, char *argv[])
{
	isp_t *p_isp=NULL;
	char f_name[128];  
	char f_name_1[128];    
	p_isp=get_isp();
	int arg_index=0;
    arg_index=get_arg_index_by_name("--raw_denoise",argc,argv);
    if(arg_index>0)
    {
        int sub_pra_index=get_arg_index_by_name("--multi",argc,argv);
        if((arg_index+1)<=(argc-1)){
           if(sub_pra_index>0){
              process_raw_haar_denoise(argv[arg_index+1],1);
           }else
           {
              process_raw_haar_denoise(argv[arg_index+1],0);
           }
        }else
        {
           log_err("too less pra for raw_denoise");
        }
    }
}
void process_cmd_hdr_merge(int argc, char *argv[])
{	
    isp_t *p_isp=NULL;
	char f_name[128];  
	char f_name_1[128];    
	p_isp=get_isp();
	int arg_index=0;
    arg_index=get_arg_index_by_name("--hdr_merge",argc,argv);
    if(arg_index>0)
    {
        if((arg_index+1)<=(argc-1)){
           process_hdr_merge(argv[arg_index+1]);
        }else
        {
           log_err("too less pra for hdr_merge");
        }
    }
}
void process_cmd_hdr_dol_split(int argc, char *argv[])
{
    isp_t *p_isp=NULL;
	char f_name[128];  
	char f_name_1[128];    
	p_isp=get_isp();
	int arg_index=0;
    arg_index=get_arg_index_by_name("--hdr_dol_split",argc,argv);
    if(arg_index>0)
    {
        if((arg_index+1)<=(argc-1))
	   {
              process_hdr_dol_split(argv[arg_index+1]);
        }else
        {
              log_err("too less pra for hdr_merge");
        }
    }

}
void process_cmd_hdr_merge_kang2014(int argc, char *argv[])
{
    isp_t *p_isp=NULL;
	char f_name[128];  
	char f_name_1[128];    
	p_isp=get_isp();
	int arg_index=0;
    arg_index=get_arg_index_by_name("--hdr_merge_kang2014",argc,argv);
    if(arg_index>0)
    {
        if((arg_index+4)<=(argc-1)){
		int frame_num=2;			
           int index_sub=get_arg_index_by_name("--n",argc,argv);
		 if(index_sub>0)
		 {
		     frame_num=atoi(argv[index_sub+1]);
		 }
		 char *n1=NULL;
		 char *n2=NULL;
		 char *n3=NULL;
		 char *n4=NULL;
		 if(frame_num==2)
		 {
		    n1=argv[arg_index+3];
		    n2=argv[arg_index+4];
		    n3=argv[arg_index+4];
		    n4=argv[arg_index+4];
		 }else if(frame_num==3)
		 {
		      n1=argv[arg_index+2];
		      n2=argv[arg_index+3];
			 n3=argv[arg_index+4];
			 n4=argv[arg_index+4];
		 }else
		 {
		      n1=argv[arg_index+1];
		      n2=argv[arg_index+2];
			 n3=argv[arg_index+3];
			 n4=argv[arg_index+4];
		 }
            process_hdr_merge_kang2014(n1,n2,n3,n4,frame_num);
        }else
        {
           log_err("too less pra for hdr_merge_kang2014");
        }
    }

}
void process_cmd_add_noise(int argc, char *argv[])
{
      isp_t *p_isp=NULL;
	char f_name[128];  
	char f_name_1[128];    
	p_isp=get_isp();
	int arg_index=0;
	    arg_index=get_arg_index_by_name("--add_noise",argc,argv);
    if(arg_index>0)
    {      
        int sub_index=0;
        int avg=0;
        int std=0;
        sub_index=get_arg_index_by_name("--avg",argc,argv);
        if(sub_index>0){
            avg=atoi(argv[sub_index+1]);
        }
        sub_index=get_arg_index_by_name("--std",argc,argv);
        if(sub_index>0){
            std=atoi(argv[sub_index+1]);
        }        
        sub_index=get_arg_index_by_name("--name",argc,argv);
        if(sub_index>0){
           process_add_noise(argv[sub_index+1],avg,std);
        }
    }
}
void process_cmd_rgbtoyuv(int argc, char *argv[])
{
      isp_t *p_isp=NULL;
	char f_name[128];  
	char f_name_1[128];    
	p_isp=get_isp();
	int arg_index=0;
    arg_index=get_arg_index_by_name("--rgbtoyuv",argc,argv);
    if(arg_index>0)
    {      
        int sub_index=0;     
        sub_index=get_arg_index_by_name("--name",argc,argv);
        if(sub_index>0){
           process_rgb_to_yuv(argv[sub_index+1]);
        }
    }
}
void process_cmd_mesh_grid(int argc, char *argv[])
{
      isp_t *p_isp=NULL;
	char f_name[128];  
	char f_name_1[128];    
	p_isp=get_isp();
	int arg_index=0;
    arg_index=get_arg_index_by_name("--mesh_grid",argc,argv);
    if(arg_index>0)
    {      
        int sub_index=0; 
        int mesh_size=64;
        int th=200;
        int skip=64;
        char *name=NULL;
        vector<Mat> all_block;        
        vector<Mat> ref_match;
        sub_index=get_arg_index_by_name("--name",argc,argv);
        if(sub_index>0){
           name=argv[sub_index+1];
        }
        sub_index=get_arg_index_by_name("--size",argc,argv);
        if(sub_index>0){
           skip=mesh_size=atoi(argv[sub_index+1]);
        }
        sub_index=get_arg_index_by_name("--th",argc,argv);
        if(sub_index>0){
           th=atoi(argv[sub_index+1]);
        }
        sub_index=get_arg_index_by_name("--skip",argc,argv);
        if(sub_index>0){
           skip=atoi(argv[sub_index+1]);
        }
        if(skip==mesh_size){
            all_block=get_all_mesh_block(name,mesh_size);
        }else
        {
            all_block=get_all_mesh_block(name,mesh_size,skip);
        }
        ref_match=get_match_block(all_block[0],all_block,th);
        paint_match_block_to_orig(name,all_block[0], ref_match); 
    }
}
void process_cmd_dct(int argc, char *argv[])
{
      isp_t *p_isp=NULL;
	char f_name[128];  
	char f_name_1[128];    
	p_isp=get_isp();
	int arg_index=0;
    arg_index=get_arg_index_by_name("--dct",argc,argv);
    if(arg_index>0)
    {      
        int sub_index=0;
        char *name=NULL;
        sub_index=get_arg_index_by_name("--name",argc,argv);
        if(sub_index>0)
        {
            name=argv[sub_index+1];
        }        
        process_dct(name);
    }   

}
void process_cmd_haar(int argc, char *argv[])
{
      isp_t *p_isp=NULL;
	char f_name[128];  
	char f_name_1[128];    
	p_isp=get_isp();
	int arg_index=0;
    arg_index=get_arg_index_by_name("--haar",argc,argv);
    if(arg_index>0)
    {      
        int sub_index=0;
        int multi=1;
        char *name=NULL;
        sub_index=get_arg_index_by_name("--name",argc,argv);
        if(sub_index>0)
        {
            name=argv[sub_index+1];
        }   
        sub_index=get_arg_index_by_name("--multi",argc,argv);
        if(sub_index>0)
        {
            multi=atoi(argv[sub_index+1]);
        } 
        process_haar_denoise(name,multi);
    } 

}
void process_cmd_convert_to(int argc, char *argv[])
{
      isp_t *p_isp=NULL;
	char f_name[128];  
	char f_name_1[128];    
	p_isp=get_isp();
	int arg_index=0;

    arg_index=get_arg_index_by_name("--convert_to",argc,argv);
    if(arg_index>0)
    {      
        int sub_index=0;
        char *name=NULL;
        char *to=NULL;
        int save_to_bin=0;        
        int w=0,h=0;
        sub_index=get_arg_index_by_name("--name",argc,argv);
        if(sub_index>0)
        {
            name=argv[sub_index+1];
        }
        sub_index=get_arg_index_by_name("--to",argc,argv);
        if(sub_index>0)
        {
            to=argv[sub_index+1];
        } 
        sub_index=get_arg_index_by_name("--save_to_bin",argc,argv);
        if(sub_index>0)
        {
            save_to_bin=1;
        }
        sub_index=get_arg_index_by_name("--size",argc,argv);
        if(sub_index>0)
        {
             w=atoi(argv[sub_index+1]);
             h=atoi(argv[sub_index+2]);
        } 
        process_convert(name,to,save_to_bin,w,h);
    }

}
void process_cmd_bm3d_image_denoising(int argc, char *argv[])
{
      isp_t *p_isp=NULL;
	char f_name[128];  
	char f_name_1[128];    
	p_isp=get_isp();
	int arg_index=0;
    arg_index=get_arg_index_by_name("--bm3d_image_denoising",argc,argv);
    if(arg_index>0)
    {      
        int sub_index=0;
        float sigma=0;
        int block_size=8;
        int search_windows_size=64;
        char *name=NULL;
        sub_index=get_arg_index_by_name("--name",argc,argv);
        if(sub_index>0)
        {
             name=argv[sub_index+1];
        } 
        sub_index=get_arg_index_by_name("--sigma",argc,argv);
        if(sub_index>0)
        {
             sigma=atof(argv[sub_index+1]);
        }
        sub_index=get_arg_index_by_name("--block_size",argc,argv);
        if(sub_index>0)
        {
             block_size=atof(argv[sub_index+1]);
        } 
        sub_index=get_arg_index_by_name("--search_windows_size",argc,argv);
        if(sub_index>0)
        {
             search_windows_size=atof(argv[sub_index+1]);
        }
        Mat denoise_img=bm3d_test(name,sigma,block_size,search_windows_size);
        show_and_save_img(name,(char *)"bm3d_denoise",denoise_img); 
        
    }

}
void process_cmd_plot2d(int argc, char *argv[])
{
      isp_t *p_isp=NULL;
	char f_name[128];  
	char f_name_1[128];    
	p_isp=get_isp();
	int arg_index=0;
        
    arg_index=get_arg_index_by_name("--plot2d",argc,argv);
    if(arg_index>0)
    {      
        plot_test();
    } 

}

void process_cmd_to_yuv_1p2b(int argc, char *argv[])
{
      isp_t *p_isp=NULL;
	char f_name[128];  
	char f_name_1[128];    
	p_isp=get_isp();
	int arg_index=0;

    arg_index=get_arg_index_by_name("--to_yuv_1p2b",argc,argv);
    if(arg_index>0)
    {	 
    	char *buf=NULL;
    	char *name=NULL;
    	int w=3840;
    	int h=2160;
    	int save_8bit=1;
    	int simi=0;
    	int sub_index=get_arg_index_by_name("--name",argc,argv);
    	if(sub_index>0)
    	{
    		 name=argv[sub_index+1];
    	}
    	sub_index=get_arg_index_by_name("-w",argc,argv);
    	if(sub_index>0)
    	{
    		 w=atoi(argv[sub_index+1]);
    	}
    	sub_index=get_arg_index_by_name("-h",argc,argv);
    	if(sub_index>0)
    	{
    		  h=atoi(argv[sub_index+1]);
    	}
    	sub_index=get_arg_index_by_name("--save_8it",argc,argv);
    	if(sub_index>0)
    	{
    		  save_8bit=atoi(argv[sub_index+1]);
    	}
    	simi=0;
    	sub_index=get_arg_index_by_name("--simi",argc,argv);
    	if(sub_index>0)
    	{
    		  simi=1;
    	}
    	int continue_128bit_packed=0;
    	sub_index=get_arg_index_by_name("--128p",argc,argv);
    	if(sub_index>0)
    	{
    		  continue_128bit_packed=1;
    	}
    	char *format=(char *)"yuv_420";
    	sub_index=get_arg_index_by_name("--format",argc,argv);
    	if(sub_index>0)
    	{
    		  format=argv[sub_index+1];
    	}
    
    	
    	int stride_w=ALIGN_TO((w*4/3),512);
    	log_info("stride_w=%d",stride_w);
    	buf=load_yuv3p4b_10bit_img(name,w,h,stride_w,format);
    	vector<cv::Mat> yuv_image;
    	if(simi)
    	{
    		if(!strcmp("yuv_420",format)){
    		   yuv_image=convert_yuv3p4b_10bit_420simi_bin_to_yuv1p2b_420sp(buf,w,h,stride_w);
    		}else if(!strcmp("yuv_422",format))
    		{
    		   yuv_image=convert_yuv3p4b_10bit_422simi_bin_to_yuv1p2b_422sp(buf,w,h,stride_w);
    		}else
    		{
    		  log_info("err not supported format");
    		}
    	}else
    	{	 
    	   if(!strcmp("yuv_420",format))
    	   {
    		  if(continue_128bit_packed){
    			 yuv_image=convert_yuv3p4b_10bit_420sp_bin_to_yuv1p2b_420sp_2(buf,w,h,stride_w);
    		  }else{
    			 yuv_image=convert_yuv3p4b_10bit_420sp_bin_to_yuv1p2b_420sp(buf,w,h,stride_w); 
    		  }
    	   }else
    	   {
    		  log_info("err not supported format");
    	   }
    	}
    	if(!save_8bit)
    	{
    	   if(!strcmp("yuv_420",format)){
    		 save_yuv4201p2b_to_bin(name,yuv_image);  
    	   }else
    	   {
    		   log_info("err not supported format");
    	   }
    	}else{
    	   if(!strcmp("yuv_420",format)){
    		   save_yuv4201p2b_to_yuv420_8bit_bin(name,yuv_image); 
    	   }else if(!strcmp("yuv_422",format))
    	   {
    		  save_yuv4221p2b_to_yuv422_8bit_bin(name,yuv_image); 
    	   }else
    	   {
    		   log_info("err not supported format");
    	   }
    	}
    	free(buf);
    }	    
 }
void process_cmd_plotmat(int argc, char *argv[])
{
     isp_t *p_isp=NULL;
	char f_name[128];  
	char f_name_1[128];    
	p_isp=get_isp();
	int arg_index=0;

    arg_index=get_arg_index_by_name("--plotmat",argc,argv);
    if(arg_index>0)
    {       
	   matplot_test(); 
    }
}
void process_cmd_split_dol_hdr_encoder_frame(int argc, char *argv[])
{
     isp_t *p_isp=NULL;
	char f_name[128];  
	char f_name_1[128];    
	p_isp=get_isp();
	int arg_index=0;
    arg_index=get_arg_index_by_name("--split_dol_hdr_encoder_frame",argc,argv);
    if(arg_index>0)
    {
         //[name] [frame_num] [height] [offset1] [offset2] [offset2]
         char *name=argv[arg_index+1];
         int frame_num=atoi(argv[arg_index+2]);
	    int  height=atoi(argv[arg_index+3]);
         int offset1=atoi(argv[arg_index+4]);		  
         int offset2=atoi(argv[arg_index+5]);		  
         int offset3=atoi(argv[arg_index+6]);
	    log_info("name=%s frame_num=%d height=%d offset1=%d offset2=%d offset3=%d",name,frame_num,height,offset1,offset2,offset3);
	    std::vector<cv::Mat> ret=split_hdr_dol_encoder_raw(name,frame_num,height,offset1,offset2,offset3,&p_isp->raw_dscr);
	    sprintf(f_name,"%s_1",name);
	    dump_raw_bye(f_name, ret[0],p_isp->raw_dscr.bayer_format,&p_isp->raw_dscr);		
	    sprintf(f_name,"%s_2",name);
	    dump_raw_bye(f_name, ret[1],p_isp->raw_dscr.bayer_format,&p_isp->raw_dscr);
    }

}
void process_cmd_split_dol_dng_frame(int argc, char *argv[])
{
     isp_t *p_isp=NULL;
	char f_name[128];  
	char f_name_1[128];    
	p_isp=get_isp();
	int arg_index=0;

      arg_index=get_arg_index_by_name("--split_dol_dng_frame",argc,argv);
      if(arg_index>0)
      {
      	 //[name] [frame_num] [height] [offset1] [offset2] [offset2]
      	 char *name=argv[arg_index+1];
      	 int frame_num=atoi(argv[arg_index+2]);
      	int  height=atoi(argv[arg_index+3]);
      	 int offset1=atoi(argv[arg_index+4]);		  
      	 int offset2=atoi(argv[arg_index+5]);		  
      	 int offset3=atoi(argv[arg_index+6]);
      	log_info("name=%s frame_num=%d height=%d offset1=%d offset2=%d offset3=%d",name,frame_num,height,offset1,offset2,offset3);
      	std::vector<cv::Mat> ret=split_hdr_dol_dng_raw(name,frame_num,height,offset1,offset2,offset3,&p_isp->raw_dscr);
      	sprintf(f_name,"%s_1",name);
      	dump_raw_bye(f_name, ret[0],p_isp->raw_dscr.bayer_format,&p_isp->raw_dscr); 	
      	sprintf(f_name,"%s_2",name);
      	dump_raw_bye(f_name, ret[1],p_isp->raw_dscr.bayer_format,&p_isp->raw_dscr);
      }

}
void process_cmd_convert_raw_2_hdr_encoder_frame(int argc, char *argv[])
{
     isp_t *p_isp=NULL;
	char f_name[128];  
	char f_name_1[128];    
	p_isp=get_isp();
	int arg_index=0;
	arg_index=get_arg_index_by_name("--convert_raw_2_hdr_encoder_frame",argc,argv);
      if(arg_index>0)
      {
         cv::Mat raw_imag=fetch_raw(argv[arg_index+1],&p_isp->raw_dscr); 
         cv::Mat hdr_encoder_img=convert_raw_to_hdr_encoder_frame(raw_imag,&p_isp->raw_dscr);	   
	   dump_raw_bye(argv[arg_index+1], hdr_encoder_img,p_isp->raw_dscr.bayer_format,&p_isp->raw_dscr);
      }
}
int main( int argc, char *argv[])
{
    process_cmd_t cmd_list[]=
    {
    	     {
		      .cmd="--save_raw_dsc",
			 .help={ "--save_raw_dsc -w 1920 -h 1080 -stride 512 -bayer 2 -bit 12 -dng -packed_type 1\n",
                          "      | note: bayer format: isCV_BayerBG[0] CV_BayerGB[1] CV_BayerRG[2] CV_BayerGR[3]\n",
                          "      | packed_type:0->std_mipi 1->128bit mipi\n",
			          NULL
			        },
			 .process_cmd=process_cmd_save_raw_dsc,
		},
       	{
			 .cmd="--fetch_raw",
			 .help={ "--fetch_raw :<name>[raw picture name]\n",NULL},
			 .process_cmd=process_cmd_fetch_raw,
		},
		{
		      .cmd="--fetch_raw_and_crop",
			 .help={ "--fetch_raw_and_crop :<name>[raw picture name] x y w h \n",NULL},
			 .process_cmd=process_cmd_fetch_raw_and_crop,
		},
		{
		      .cmd="--fetch_raw_and_scaler",
			 .help={ "--fetch_raw_and_scaler :<name>[raw picture name] w h \n",NULL},
			 .process_cmd=process_cmd_fetch_raw_and_scaler,
		},
		{
		      .cmd="--get_blc_pra",
			 .help={ "--get_blc_pra :<name>[blc tuning picture] \n",NULL},
			 .process_cmd=process_cmd_get_blc_pra,
		},
		{
		      .cmd="--get_raw_his",
			 .help={ "--get_raw_his :<name>[raw picture] --log_en --dump_mem\n",NULL},
			 .process_cmd=process_cmd_get_raw_his,
		},
		{
		      .cmd="--plot_raw_hist",
			 .help={ "--plot_raw_hist :<name>[raw picture]\n",NULL},
			 .process_cmd=process_cmd_plot_raw_hist,
		},
		{
		      .cmd="--raw_denoise",
			 .help={ "--raw_denoise :<name>[raw picture name] --multi \n",NULL},
			 .process_cmd=process_cmd_raw_denoise,
		},
		{
		      .cmd="--hdr_merge",
			 .help={ "--hdr_merge :<name>[raw picture name] note: only for imx307 dol single frame",NULL},
			 .process_cmd=process_cmd_hdr_merge,
		},
		{
		      .cmd="--hdr_dol_split",
			 .help={ "--hdr_dol_split :<name>[raw picture name]\n",NULL},
			 .process_cmd=process_cmd_hdr_dol_split,
		},
		{
		      .cmd="--hdr_merge_kang2014",
			 .help={ "--hdr_merge_kang2014 [name1 name2 name3 name4] ,note: name1 is long exp\n",NULL},
			 .process_cmd=process_cmd_hdr_merge_kang2014,
		},
		{
		      .cmd="--add_noise",
			 .help={ "--add_noise --name [name] --avg [avg] --std [sigma]\n",NULL},
			 .process_cmd=process_cmd_add_noise,
		},
	     {
		      .cmd="--rgbtoyuv",
			 .help={ "--rgbtoyuv --name [name] \n",NULL},
			 .process_cmd=process_cmd_rgbtoyuv,
		},
	      {
		      .cmd="--mesh_grid",
			 .help={ "--mesh_grid --name [name] --size [n*n] --skip [skip] --th [th]: eg --mesh_grid 64 --th 100 \n",NULL},
			 .process_cmd=process_cmd_mesh_grid,
		},
	     {
		      .cmd="--dct",
			 .help={ "--dct --name [name] \n",NULL},
			 .process_cmd=process_cmd_dct,
		},
	     {
		      .cmd="--haar",
			 .help={ "----haar --name [name] --multi [b_multi]\n",NULL},
			 .process_cmd=process_cmd_haar,
		},
	     {
		      .cmd="--convert_to",
			 .help={ "--convert_to --name [name] --to [rgb565/rgb1555] --save_to_bin --size [w,h]\n",NULL},
			 .process_cmd=process_cmd_convert_to,
		},
	     {
		      .cmd="--bm3d_image_denoising",
			 .help={ "--bm3d_image_denoising --name <string> --sigma <double> --block_size <int> --search_windows_size <int>\n",NULL},
			 .process_cmd=process_cmd_bm3d_image_denoising,
		},
	     {
		      .cmd="--plot2d",
			 .help={ "--plot2d :run plot 2d demo\n",NULL},
			 .process_cmd=process_cmd_plot2d,
		},
	     {
		      .cmd="--to_yuv_1p2b",
			 .help={ 
			           "--to_yuv_1p2b  --name [name] -w[w] -h[h] --save_8it --simi --128p --format [yuv_420/yuv_422]:convert yuv image to 1p2b format\n",
				      "       |--simi: simi format --128p: artosyn 128 bit continue packed\n",
			           NULL
			         },
			 .process_cmd=process_cmd_to_yuv_1p2b,
		},
	     {
		      .cmd="--plotmat",
			 .help={ "--plotmat :run matplot demo\n",NULL},
			 .process_cmd=process_cmd_plotmat,
		},
	      {
		      .cmd="--split_dol_hdr_encoder_frame",
			 .help={ "--split_dol_hdr_encoder_frame [name] [frame_num] [height] [offset1] [offset2] [offset2]\n",NULL},
			 .process_cmd=process_cmd_split_dol_hdr_encoder_frame,
		},
	     {
		      .cmd="--split_dol_dng_frame",
			 .help={ "--split_dol_dng_frame [name] [frame_num] [height] [offset1] [offset2] [offset2]\n",NULL},
			 .process_cmd=process_cmd_split_dol_dng_frame,
		},
	     {
		      .cmd="--convert_raw_2_hdr_encoder_frame",
			 .help={ "--convert_raw_2_hdr_encoder_frame [name]\n",NULL},
			 .process_cmd=process_cmd_convert_raw_2_hdr_encoder_frame,
		},
    };

    int i=0;
    isp_t *p_isp=NULL;
    int arg_index=0;     

    for(i=0;i<argc;i++)
    {
       printf("%s ",argv[i]);
    }
    printf("\n"); 
    arg_index=get_arg_index_by_name("--help",argc,argv);
    if(arg_index>0 ||argc==1)
    {
        if(argc>=3)
        {
            char *help_cmd=argv[arg_index+1];			
		  for(int i=0;i<sizeof(cmd_list)/sizeof(process_cmd_t);i++)
		  {
		      if(!strcmp(help_cmd,cmd_list[i].cmd))
		      {
		            int j=0;
                       while(cmd_list[i].help[j])
			       {
                           printf("%s",cmd_list[i].help[j]);
               			j++;
                       }
			       break;
		      }
		  }
        }else{
            for(int i=0;i<sizeof(cmd_list)/sizeof(process_cmd_t);i++)
            {
                int j=0;
                while(cmd_list[i].help[j]){
                    printf("%s",cmd_list[i].help[j]);
    			    j++;
                }
            }
        }
        return 0;
    }
    p_isp_server=(isp_t *)new(isp_t);
    p_isp=get_isp();
    p_isp->isp_pra=(isp_pra_t *)malloc(sizeof(isp_pra_t));
    //load raw cfg  format etc
    if(load_bin((char *)"raw_dsc.bin",&p_isp->raw_dscr,sizeof(p_isp->raw_dscr))<0)
    {
       log_err("err load raw dsc");
       free(p_isp->isp_pra);
       delete(p_isp);
       return 0;
    }

    //process cmd
    for(int i=0;i<sizeof(cmd_list)/sizeof(process_cmd_t);i++)
    {
        char *cmd=(char *)cmd_list[i].cmd;
	   if(!strcmp(cmd,argv[1]))
	   {
	       cmd_list[i].process_cmd(argc,argv);
		  break;
	   }
    }
    if(i>=sizeof(cmd_list)/sizeof(process_cmd_t))
    {
        log_info("no cmd %s",argv[1]);
    }
	
    log_info("press a key to exit");
    cvWaitKey(0);
    free(p_isp->isp_pra);
    delete(p_isp);
    return 0;
}
int end_function()
{
  return 0;
}

