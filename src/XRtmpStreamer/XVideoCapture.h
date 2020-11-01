#pragma once
#include "XDataThread.h"
#include "XFilter.h"
#include <vector>
class XVideoCapture:public XDataThread
{
public:
	int width = 0;
	int height = 0;
	int fps = 0;

	static XVideoCapture *Get(unsigned char index = 0);
	virtual bool Init(int camIndex = 0) = 0;
	virtual bool Init(const char *url) = 0;
	virtual void Stop() = 0;
	virtual ~XVideoCapture();
	
	//插入过滤器
	void AddFilter(XFilter *f)
	{
		fmutex.lock();
		filters.push_back(f);
		fmutex.unlock();
	}
	//清空过滤器

protected:
	QMutex fmutex; //filters会频繁操作，故专门创建个单独的锁
	std::vector<XFilter*> filters;//多个过滤器
	XVideoCapture();

};

