#pragma once

enum XFilterType
{
	XBILATERAL //双边滤波
};

namespace cv
{
	class Mat;
}

#include <string>
#include <map>
class XFilter //过滤器
{
public:
	static XFilter*Get(XFilterType t = XBILATERAL);

	virtual bool Filter(cv::Mat *src, cv::Mat *des) = 0; //算法
	virtual bool Set(std::string key, double value);  //设置参数

	virtual ~XFilter();
protected:
	std::map<std::string, double> paras;
	XFilter();

};

