#include "cv.h"
#include "highgui.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include "isp_pipeline.h"
#include "isp_log.h"
#include "string.h"
#include "isp_util.h"

#define MAX_CMDS 128


typedef struct
{
  int num;
  char args[8][265];
}args_t;

typedef struct
{
  char *cmdline;
  char *f_name;
  raw_type_file_dscr_t raw_dscr;
  isp_pra_t *isp_pra;
  raw_ch_t ch;  
  cv::Mat raw_imag; //fetch raw will return the image    
  cv::Mat blc_imag;
}isp_t;

typedef struct
{
 char *cmd;
 char *help_string;
 int (* process)(char *cmd_lines); 
}cmds_t;

static cmds_t *g_cmds[MAX_CMDS]={0};
static isp_t *p_isp_server=NULL;

static get_isp()
{
  return p_isp_server;
}

args_t *get_args(char *cmdline)
{
   args_t *p_arg=(args_t *)malloc(sizeof(args_t));
   char *cmdline_new=(char *)malloc(1024);
   char *delim=(char *)" "; 
   memset(p_arg,0,sizeof(args_t));
   strcpy(cmdline_new,cmdline);
   char* p=strtok(cmdline_new,delim); 
   while(p!=NULL){ 
         strcpy(p_arg->args[p_arg->num],p);
         p_arg->num++;
         p=strtok(NULL,delim); 
   }
  free(cmdline_new);
  return p_arg; 
}
int register_cmds(cmds_t *cmd)
{
   int i=0;
   log_func_enter();
   for(i=0;i<MAX_CMDS;i++)
   {
      if(g_cmds[i]&&g_cmds[i]->cmd&&!strcmp(g_cmds[i]->cmd,cmd->cmd))
      {
        log_info("have register");
        return 0;
      }
   }
   //find a null slot
   for(i=0;i<MAX_CMDS;i++)
   {
      if(!g_cmds[i])
      {
         g_cmds[i]=cmd;         
         log_info("%s register success",g_cmds[i]->cmd);
         return 0;
      }
   }
   log_err("register failed full");
   return 0;
}
static int process_cmd_line(char *cmd_line)
{
   args_t *arg=get_args(cmd_line);
   int i=0;
   for(i=0;i<MAX_CMDS;i++)
   {
     if(g_cmds[i]&&!strcmp(arg->args[0],g_cmds[i]->cmd))
     {
        g_cmds[i]->process(cmd_line);
        break;
     }
   }
   return 0;
}

static int process_help(char *cmd_line)
{
   int i=0;
   args_t *arg=get_args(cmd_line);
   for(i=0;i<MAX_CMDS;i++)
   {
      if(g_cmds[i]&&g_cmds[i]->cmd)
      {
          printf("%s:%s \n",g_cmds[i]->cmd,g_cmds[i]->help_string);
      }
   }
   free(arg);
}
static cmds_t cmd_help=
{ 
   .cmd=(char *)"help",
   .help_string=(char *)"list all the cmds",
   .process=process_help,
};
static int process_fetch_raw(char *cmd_line)
{
   int i=0;
   isp_t *p_isp=get_isp();
   args_t *arg=get_args(cmd_line);
   if(arg->num!=2)
   {
     log_err("par num err,must be cmdline=%s",cmd_line);     
     free(arg);
     return -1;
   }   
   p_isp->raw_imag=fetch_raw(arg->args[1],&p_isp->raw_dscr); 
   sprintf(p_isp->f_name,"%s_fetch",arg->args[1]);
   dump_raw_to_png(p_isp->f_name,p_isp->raw_imag,p_isp->raw_dscr.bayer_format);
   show_raw_image(p_isp->f_name,p_isp->raw_imag,p_isp->raw_dscr.bayer_format);
   free(arg);
}
static cmds_t cmd_fetch_raw=
{ 
   .cmd=(char *)"fetch_raw",
   .help_string=(char *)"fetch_raw",
   .process=process_fetch_raw,
};
static int process_get_blc_pra(char *cmd_line)
{
   int i=0;   
   cv::Mat raw_imag;  
   isp_t *p_isp=get_isp();
   args_t *arg=get_args(cmd_line);
   if(arg->num!=2)
   {
     log_err("par num err,must be 2");     
     free(arg);
     return -1;
   }   
   raw_imag=fetch_raw(arg->args[1],&p_isp->raw_dscr); 
   p_isp->ch=split_raw_ch(raw_imag,p_isp->raw_dscr.bayer_format);
   p_isp->isp_pra->blc_pra=get_blc_value(p_isp->ch,p_isp->raw_dscr.bayer_format);
   save_isp_pra(p_isp->isp_pra,sizeof(isp_pra_t));
   free(arg);
}
static cmds_t cmd_get_blc_pra=
{ 
   .cmd=(char *)"get_blc_pra",
   .help_string=(char *)"get_blc_pra",
   .process=process_get_blc_pra,
};

static int process_proc_blc(char *cmd_line)
{
   int i=0;   
   cv::Mat raw_imag;   
   isp_t *p_isp=get_isp();
   args_t *arg=get_args(cmd_line);
   if(arg->num!=2)
   {
     log_err("par num err,must be 2");     
     free(arg);
     return -1;
   }   
   p_isp->ch=split_raw_ch(p_isp->raw_imag,p_isp->raw_dscr.bayer_format);
   p_isp->ch=process_blc(p_isp->ch,p_isp->isp_pra->blc_pra,p_isp->raw_dscr.bayer_format);
   merge_raw_ch(p_isp->ch,p_isp->raw_dscr.bayer_format,p_isp->blc_imag); 
   sprintf(p_isp->f_name,"%s_blc",arg->args[1]);
   dump_raw_to_png(p_isp->f_name,p_isp->blc_imag,p_isp->raw_dscr.bayer_format);
   show_raw_image(p_isp->f_name,p_isp->blc_imag,p_isp->raw_dscr.bayer_format);
   free(arg);
}
static cmds_t cmd_proc_blc=
{ 
   .cmd=(char *)"proc_blc",
   .help_string=(char *)"proc_blc",
   .process=process_proc_blc,
};
cmds_t *isp_cmd[]=
{
   &cmd_help,
   &cmd_fetch_raw,
   &cmd_get_blc_pra,
   &cmd_proc_blc,
   NULL,
};
int main( int argc, char** argv)
{
    int i=0;
    isp_t *p_isp=NULL;
    p_isp_server=(isp_t *)new(isp_t);
    p_isp=get_isp();
    p_isp->cmdline=(char *)malloc(1024);
    p_isp->f_name=(char *)malloc(1024);
    p_isp->isp_pra=(isp_pra_t *)malloc(sizeof(isp_pra_t));
    p_isp->raw_dscr.width=4208;
    p_isp->raw_dscr.hegiht=3120;
    p_isp->raw_dscr.bitwidth=10;
    p_isp->raw_dscr.is_packed=1;
    p_isp->raw_dscr.height_algin=1;
    p_isp->raw_dscr.width_algin=4;
    p_isp->raw_dscr.line_length_algin=8;
    p_isp->raw_dscr.bayer_format=CV_BayerBG;    
    cv::Mat blc_imag(p_isp->raw_dscr.hegiht,p_isp->raw_dscr.width,CV_16UC1);
    p_isp->blc_imag=blc_imag;
    while(isp_cmd[i])
    {
       register_cmds(isp_cmd[i]);
       i++;
    }
    load_isp_pra(p_isp->isp_pra,sizeof(isp_pra_t));
    while(1)
    {
       printf("isp@lll>>>");
       gets(p_isp->cmdline);
       if(!strcmp(p_isp->cmdline,"exit")){
         break;
       }else
       {
          process_cmd_line(p_isp->cmdline);
       }
    }
    free(p_isp->cmdline);
    free(p_isp->f_name);    
    free(p_isp->isp_pra);
    delete(p_isp);
    return 0;
}


