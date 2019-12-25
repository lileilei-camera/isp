#ifndef __ISP_LOG_H__
#define __ISP_LOG_H__
#define log_info(fmt,...)  printf("[INFO]%s" fmt "\n",__FUNCTION__,##__VA_ARGS__)
#define log_func_enter()  printf("[FUN]%s enter %d\n",__FUNCTION__,__LINE__)
#define log_func_exit()  printf("[FUN]%s exit\n",__FUNCTION__)
#define log_err(fmt,...)  printf("[ERR]%s" fmt "\n",__FUNCTION__,##__VA_ARGS__)
#define log_debug(fmt,...) do{}while(0)
#endif
