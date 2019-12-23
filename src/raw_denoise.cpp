#include"isp_pipeline.h"

Mat single_haar_wavelet_decompose(Mat input_img,int layers)
{
   int i=0;
   int j=0;
   int jj=0;
   int wave_rows;
   int wave_cols;
   
   if(!(input_img.rows%2))
      wave_rows=input_img.rows;
   else
      wave_rows=input_img.rows+1;

   
   if(!(input_img.cols%2))
      wave_cols=input_img.cols;
   else
      wave_cols=input_img.cols+1;
   
   Mat temp_img(wave_rows,wave_cols,CV_32FC1);   
   Mat wave_img(wave_rows,wave_cols,CV_32FC1);
   if(layers>=1)
   {
      log_info("decompose 1 layer");
      //h decompose
      if(!(input_img.cols%2))
      {
          for(i=0;i<input_img.rows;i++)
          {
              jj=0;
              for(j=0;j<input_img.cols;j+=2)
              {                 
                 temp_img.at<float>(i,jj)=((float)input_img.at<float>(i,j)+(float)input_img.at<float>(i,j+1))/2;                 
                 temp_img.at<float>(i,wave_cols/2+jj)=((float)input_img.at<float>(i,j)-(float)input_img.at<float>(i,j+1))/2;
                 jj++;
              }
          }
      }else
      {
          for(i=0;i<input_img.rows;i++)
          {
              jj=0;
              for(j=0;j<input_img.cols-1;j+=2)
              {                 
                 temp_img.at<float>(i,jj)=((float)input_img.at<float>(i,j)+(float)input_img.at<float>(i,j+1))/2;                 
                 temp_img.at<float>(i,wave_cols/2+jj)=((float)input_img.at<float>(i,j)-(float)input_img.at<float>(i,j+1))/2;
                 jj++;
              }               
              temp_img.at<float>(i,jj)=((float)input_img.at<float>(i,j)+(float)input_img.at<float>(i,j))/2;                 
              temp_img.at<float>(i,wave_cols/2+jj)=((float)input_img.at<float>(i,j)-(float)input_img.at<float>(i,j))/2;
          }
      }
      //v decompose
      if(!(input_img.rows%2))
      {
          for(i=0;i<input_img.cols;i++)
          {
              jj=0;
              for(j=0;j<input_img.rows;j+=2)
              {
                 wave_img.at<float>(jj,i)=(temp_img.at<float>(j,i)+temp_img.at<float>(j+1,i))/2;                 
                 wave_img.at<float>(wave_rows/2+jj,i)=(temp_img.at<float>(j,i)-temp_img.at<float>(j+1,i))/2;
                 jj++;
              }              
          }
      }else
      {
          for(i=0;i<input_img.cols;i++)
          {
              jj=0;
              for(j=0;j<input_img.rows-1;j+=2)
              {
                 wave_img.at<float>(jj,i)=(temp_img.at<float>(j,i)+temp_img.at<float>(j+1,i))/2;                 
                 wave_img.at<float>(wave_rows/2+jj,i)=(temp_img.at<float>(j,i)-temp_img.at<float>(j+1,i))/2;
                 jj++;
              }              
              wave_img.at<float>(jj,i)=(temp_img.at<float>(j,i)+temp_img.at<float>(j,i))/2;                 
              wave_img.at<float>(wave_rows/2+jj,i)=(temp_img.at<float>(j,i)-temp_img.at<float>(j,i))/2;
          }
      }
   }
   return wave_img;
}

Mat multi_haar_wavelet_decompose(Mat input_img,int layers)
{
   int i=0;
   int j=0;
   int wave_rows;
   int wave_cols;
   
   wave_rows=input_img.rows*2;
   wave_cols=input_img.cols*2;
   
   Mat temp_img(wave_rows,wave_cols,CV_32FC1);       
   Mat wave_img(wave_rows,wave_cols,CV_32FC1);
   if(layers>=1)
   {
      log_info("decompose 1 layer");

      for(i=0;i<input_img.rows;i++)
      {
          for(j=0;j<input_img.cols-1;j++)
          {                 
             temp_img.at<float>(i,j)=((float)input_img.at<float>(i,j)+(float)input_img.at<float>(i,j+1))/2;                 
             temp_img.at<float>(i,wave_cols/2+j)=((float)input_img.at<float>(i,j)-(float)input_img.at<float>(i,j+1))/2;
          }
          //process last rows pix
          temp_img.at<float>(i,j)=((float)input_img.at<float>(i,j)+(float)input_img.at<float>(i,j))/2;                 
          temp_img.at<float>(i,wave_cols/2+j)=((float)input_img.at<float>(i,j)-(float)input_img.at<float>(i,j))/2;
      }
      //v decompose
      for(i=0;i<wave_cols;i++)
      {
          for(j=0;j<input_img.rows-1;j++)
          {
             wave_img.at<float>(j,i)=(temp_img.at<float>(j,i)+temp_img.at<float>(j+1,i))/2;                 
             wave_img.at<float>(wave_rows/2+j,i)=(temp_img.at<float>(j,i)-temp_img.at<float>(j+1,i))/2;
          }              
          wave_img.at<float>(j,i)=(temp_img.at<float>(j,i)+temp_img.at<float>(j,i))/2;                 
          wave_img.at<float>(wave_rows/2+j,i)=(temp_img.at<float>(j,i)-temp_img.at<float>(j,i))/2;
      }
   }
   return wave_img;
}

Mat haar_wavelet_decompose(Mat input_img,int layers,int is_multi)
{
   if(is_multi)
    return multi_haar_wavelet_decompose(input_img,layers);
   else
    return single_haar_wavelet_decompose(input_img,layers);
}

int haar_wavelet_denoise(Mat *wave_imag,int layers,wave_denoise_pra_t *pra)
{
   
}

int haar_wavelet_compose(Mat out_img,Mat *wave_imag,int layers)
{

}

