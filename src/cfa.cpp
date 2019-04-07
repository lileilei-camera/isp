#include "isp_pipeline.h"
//out is a bgr picture
Mat isp_cfa(Mat in_img,bayer_format_t format,int algo=0)
{

  IplImage imge;
  cv::Mat out;
  int code=CV_BayerBG2RGB;
  switch(format)
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
  cv::cvtColor(in_img,out,code);
  return out;
}
