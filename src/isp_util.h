#ifndef __ISP_UTIL_H__
#define __ISP_UTIL_H__
int save_isp_pra(void *buf,int size);
int load_isp_pra(void *buf,int size);
int save_bin(char *name,void *buf,int size);
int load_bin(char *name,void *buf,int size);
#endif
