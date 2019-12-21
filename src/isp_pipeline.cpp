#include "isp_pipeline.h"
#include "plot.h"

//#define DEBUG_MIPI_RAW

int test_plot();
int save_img(char *picname,char *func_name,Mat img);


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

static int  process_hdr_merge_kang2014(char *name1,char *name2,char *name3,char *name4)
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
   
   hdr_merge_pra_kang2014_t hdr_pra;
   hdr_pra.rou=2.0/3.0*0xffff;
   hdr_pra.beta=16<<8;
   hdr_pra.deta=0.25;
   //merg_img 32bit float
   cv::Mat merg_img=merge_hdr_img_kang2014(raw_imge1,raw_imge2,raw_imge3,raw_imge4,ration,&hdr_pra);   
   //cv::Mat merg_img_demosic=demosic_raw_image(merg_img,p_isp->raw_dscr.bayer_format);
   //only exr format supported float format or 32int
   dump_to_exr(name1,(char *)"kanghdr_demosic",merg_img);
   //cv::Mat hdr_img=drc_hdr_img_kang2014_gamma(merg_img,5.0); 
   cv::Mat hdr_img=drc_hdr_img_kang2014_linear(merg_img);
   show_and_save_img(name1,(char *)"kanghdr",hdr_img);   
   dump_raw_to_png(name1,hdr_img,p_isp->raw_dscr.bayer_format);
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

static cv::vector<Mat> get_all_mesh_block(char *name1,int mesh_size)
{
  int i=0;
  int j=0;
  cv::vector<Mat> all_block;
  cv::Mat sub_mesh_block;
  char name[256]="";
  cv::Mat src_img=imread(name1);
  int block_w=src_img.cols/mesh_size;
  int block_h=src_img.rows/mesh_size;
  for(i=0;i<block_h;i++)
  {
      for(j=0;j<block_w;j++)
      {         
         log_info("[%d][%d]",i,j);
         sub_mesh_block=src_img(Rect(j*mesh_size,i*mesh_size,mesh_size,mesh_size));
         all_block.push_back(sub_mesh_block); 
         //sprintf(name,"_%d_%d_",i,j);
         //save_img(name1,(char *)name,sub_mesh_block);   
         //show_and_save_img(name1,(char *)name,sub_mesh_block);  
      }
  }
  return all_block;
}

static cv::vector<Mat> get_match_block(Mat ref_block,cv::vector<Mat> all_block,int th)
{
   char name[256]="";
   cv::vector<Mat> ref_match;
   double odis=0;
   int i=0;
   CvMat ref=ref_block;
   CvMat all;
   for(i=0;i<all_block.size();i++)
   {
       all=all_block[i];       
       odis=cvNorm(&ref, &all, CV_L2);
       //log_info("odis=%f",odis);
       if(odis<th)
       {
          log_info("i=%d odis=%d",i,(int)odis);
          ref_match.push_back(all_block[i]);
          sprintf(name,"_%d_dis=%d_",i,(int)odis);
          save_img((char *)"../file/pic/mesh/ref_match",(char *)name,all_block[i]);    
       }
   }
   return ref_match;
}
static int paint_match_block_to_orig(char *name1,Mat ref_block,cv::vector<Mat> match_block)
{  
     cv::Mat src_img=imread(name1);
     
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
static int show_help()
{
   printf("--help :show this help\n");
   printf("--fetch_raw :<name>[raw picture name]\n");
   printf("--save_raw_dsc -w 1920 -h 1080 -stride 512 -byaer 2 --bit 12 :note: byaer_farmat isCV_BayerBG[0] CV_BayerGB[1] CV_BayerRG[2] CV_BayerGR[3]");
   printf("--get_blc_pra :<name>[blc tuning picture] \n");
   printf("--process :<raw_name> <name1> <name2> ... --end \n");
   printf("--test_plot :test the plat function \n");   
   printf("--get_raw_his :<name>[raw picture] --log_en --dump_mem\n");   
   printf("--plot_raw_hist :<name>[raw picture]\n");
   printf("--raw_denoise :<name>[raw picture name] --multi \n");   
   printf("--hdr_merge :<name>[raw picture name]\n");   
   printf("--hdr_merge_kang2014 [name1 name2 name3 name4]\n");
   printf("--test_cvui\n");
   printf("--add_noise --name [name] --avg [avg] --std [sigma] \n");
   printf("--rgbtoyuv --name [name] \n");
   printf("--mesh_grid --name [name] --size [n*n] --th [th]: eg --mesh_grid 64 --th 100 \n");
   return 0;
}

int main( int argc, char *argv[])
{
    int i=0;
    isp_t *p_isp=NULL;
    int arg_index=0;     
    int sub_arg_index=0;
    char f_name[128];

    for(i=0;i<argc;i++)
    {
       printf("%s ",argv[i]);
    }
    printf("\n");
    
    if(argc==1)
    {
       show_help();
       return 0;
    }
    arg_index=get_arg_index_by_name("--help",argc,argv);
    if(arg_index>0)
    {
        show_help();
        return 0;
    }
    
    p_isp_server=(isp_t *)new(isp_t);
    p_isp=get_isp();
    p_isp->isp_pra=(isp_pra_t *)malloc(sizeof(isp_pra_t));
 
    #ifdef DEBUG_MIPI_RAW
    p_isp->raw_dscr.width=4208;
    p_isp->raw_dscr.hegiht=3120;
    p_isp->raw_dscr.bitwidth=10;
    p_isp->raw_dscr.is_packed=1;
    p_isp->raw_dscr.height_algin=1;
    p_isp->raw_dscr.width_algin=4;
    p_isp->raw_dscr.line_length_algin=8;
    p_isp->raw_dscr.bayer_format=CV_BayerBG; 
    #else
    #if 1
       #if 0
       p_isp->raw_dscr.width=1952;
       p_isp->raw_dscr.hegiht=1080;    
       p_isp->raw_dscr.bitwidth=14;
       #endif
       #if 0
       p_isp->raw_dscr.width=1952;
       p_isp->raw_dscr.hegiht=2288;
       p_isp->raw_dscr.bitwidth=12;
       #endif
       #if 1
       p_isp->raw_dscr.width=1920;
       p_isp->raw_dscr.hegiht=1080;
       p_isp->raw_dscr.bitwidth=12;
       #endif   
    p_isp->raw_dscr.is_packed=0;
    p_isp->raw_dscr.height_algin=1;
    p_isp->raw_dscr.width_algin=1;
    p_isp->raw_dscr.line_length_algin=512;
    p_isp->raw_dscr.bayer_format=CV_BayerRG; 
    #else  //dvp raw
    p_isp->raw_dscr.width=1278;
    p_isp->raw_dscr.hegiht=800;
    p_isp->raw_dscr.bitwidth=8;
    p_isp->raw_dscr.is_packed=0;
    p_isp->raw_dscr.height_algin=1;
    p_isp->raw_dscr.width_algin=1;
    p_isp->raw_dscr.line_length_algin=1;
    p_isp->raw_dscr.bayer_format=CV_BayerRG; 
    #endif
    #endif
       
    arg_index=get_arg_index_by_name("--save_raw_dsc",argc,argv);
    if(arg_index>0)
    {
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
       save_bin((char *)"raw_dsc.bin",&p_isp->raw_dscr,sizeof(p_isp->raw_dscr));       
    }
    if(load_bin((char *)"raw_dsc.bin",&p_isp->raw_dscr,sizeof(p_isp->raw_dscr))<0)
    {
       log_err("err load raw dsc");
       free(p_isp->isp_pra);
       delete(p_isp);
       return 0;
    }
    
    
    cv::Mat blc_imag(p_isp->raw_dscr.hegiht,p_isp->raw_dscr.width,CV_16UC1);
    p_isp->blc_imag=blc_imag;
    
    arg_index=get_arg_index_by_name("--test_plot",argc,argv);
    if(arg_index>0)
    {       
       int plot_drc_gtm1(float k1,float k2,float k3,float gain);
       plot_drc_gtm1(1,8,0.4,1);
    }

    arg_index=get_arg_index_by_name("--test_cvui",argc,argv);
    if(arg_index>0)
    {       
       //int cvui_test();
       //cvui_test();
    }
    
    arg_index=get_arg_index_by_name("--fetch_raw",argc,argv);
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

    arg_index=get_arg_index_by_name("--get_raw_his",argc,argv);
    if(arg_index>0)
    {
        if((arg_index+1)<=(argc-1)){
           process_get_hist(argv[arg_index+1]);
           sub_arg_index=get_arg_index_by_name("--log_en",argc,argv);
           if(sub_arg_index>0)
           {
              log_raw_hist(&p_isp->raw_hsit);
           }
           release_raw_hist(&p_isp->raw_hsit);
        }else
        {
           log_err("too less pra for get_raw_his");
        }
    }

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
    
    arg_index=get_arg_index_by_name("--hdr_merge_kang2014",argc,argv);
    if(arg_index>0)
    {
        if((arg_index+4)<=(argc-1)){
           process_hdr_merge_kang2014(argv[arg_index+1],argv[arg_index+2],argv[arg_index+3],argv[arg_index+4]);
        }else
        {
           log_err("too less pra for hdr_merge_kang2014");
        }
    }
    
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
    arg_index=get_arg_index_by_name("--rgbtoyuv",argc,argv);
    if(arg_index>0)
    {      
        int sub_index=0;     
        sub_index=get_arg_index_by_name("--name",argc,argv);
        if(sub_index>0){
           process_rgb_to_yuv(argv[sub_index+1]);
        }
    }
   
    arg_index=get_arg_index_by_name("--mesh_grid",argc,argv);
    if(arg_index>0)
    {      
        int sub_index=0; 
        int mesh_size=64;
        int th=200;
        char *name=NULL;
        cv::vector<Mat> all_block;        
        cv::vector<Mat> ref_match;
        sub_index=get_arg_index_by_name("--name",argc,argv);
        if(sub_index>0){
           name=argv[sub_index+1];
        }
        sub_index=get_arg_index_by_name("--size",argc,argv);
        if(sub_index>0){
           mesh_size=atoi(argv[sub_index+1]);
        }
        sub_index=get_arg_index_by_name("--th",argc,argv);
        if(sub_index>0){
           th=atoi(argv[sub_index+1]);
        }
        all_block=get_all_mesh_block(name,mesh_size);
        ref_match=get_match_block(all_block[0],all_block,th);
        
    }
    
    arg_index=get_arg_index_by_name("--process",argc,argv);
    if(arg_index>0)
    {
       load_isp_pra(p_isp->isp_pra,sizeof(isp_pra_t));
       int arg_end=get_arg_index_by_name("--end",argc,argv);
       if(arg_end>0)
       {
          int pro_index=0;
          process_fetch_raw(argv[arg_index+1]);          
          sprintf(f_name,"%s_fetch",argv[arg_index+1]);
          show_raw_image(argv[arg_index+1],p_isp->raw_imag,p_isp->raw_dscr.bayer_format);
          log_info("arg_index=%d,arg_end=%d",arg_index,arg_end);
          for(pro_index=arg_index+2;pro_index<arg_end;pro_index++)
          {
              if(!strcmp("blc",argv[pro_index]))
              {
                 log_info("%s",argv[pro_index]);
                 process_proc_blc(argv[arg_index+1]);                 
                 sprintf(f_name,"%s_blc",argv[arg_index+1]);
                 show_raw_image(f_name,p_isp->blc_imag,p_isp->raw_dscr.bayer_format);
              }
          }
       }
    }

    log_info("press a key to exit");
    cvWaitKey(0);
    free(p_isp->isp_pra);
    delete(p_isp);
    return 0;
}


