#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include<unistd.h>
#include "isp_log.h"
#include "isp_util.h"

int save_isp_pra(void *buf,int size)
{
  int fd=open("isp_pra.bin",O_RDWR|O_CREAT);
  if(fd<0)
  {
    log_err("open file failed");
	return fd;
  }
  write(fd,buf,size);
  log_info("save success");
  close(fd);
  return 0;
}

int load_isp_pra(void *buf,int size)
{
  int fd=open("isp_pra.bin",O_RDONLY);
  if(fd<0)
  {
    log_err("open file failed,please firset gen the data");
	return fd;
  }
  read(fd,buf,size);
  close(fd);
  return 0;
}
