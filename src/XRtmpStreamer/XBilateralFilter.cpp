#include "XBilateralFilter.h"
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp> //Í¼Ïñ´¦Àí

using namespace cv;
using namespace std;

bool XBilateralFilter::Filter(cv::Mat *src, cv::Mat *des) //¹ıÂË
{
	
	double d = paras["d"];

	bilateralFilter(*src, *des, d, d*2, d/2);
	return true;
}

XBilateralFilter::XBilateralFilter()
{
	paras.insert(make_pair("d", 5));
}


XBilateralFilter::~XBilateralFilter()
{
}
