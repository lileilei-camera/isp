#include "opencv2/xphoto.hpp"
#include "opencv2/highgui.hpp"

cv::Mat bm3d_test(char *infile,float sigma,int templateWindowSize,int searchWindowSize)
{
    std::string inFilename(infile);
    cv::Mat src = cv::imread(inFilename, cv::IMREAD_GRAYSCALE);
    if (sigma == 0.0)
        sigma = 15.0;

    if (templateWindowSize == 0)
        templateWindowSize = 4;

    if (searchWindowSize == 0)
        searchWindowSize = 16;

    cv::Mat res(src.size(), src.type());
    cv::xphoto::bm3dDenoising(src, res, sigma, templateWindowSize, searchWindowSize);

    return res;
}
