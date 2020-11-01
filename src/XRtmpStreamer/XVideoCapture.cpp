#include <opencv2/highgui.hpp>
#include "XVideoCapture.h"
#include <iostream>
#pragma comment(lib,"opencv_world320.lib")
using namespace std;
using namespace cv;
class CXVideoCapture :public XVideoCapture
{
public:
	VideoCapture cam;

	void run()
	{
		Mat frame; //直接读取一帧，不做丢帧处理
		while (!isExit)
		{
			if (!cam.read(frame))
			{
				msleep(1);
				continue;
			}
			if (frame.empty())
			{
				msleep(1);
				continue;
			}

			//确保数据是连续的 
			fmutex.lock();
			for (int i = 0;i<filters.size(); i++)
			{
				Mat des;
				filters[i]->Filter(&frame, &des); //原图,目标
				frame = des; //将处理结果覆盖
			}
			fmutex.unlock();
			//读每一帧画面，经过滤器处理后再推送到列表中

			//elemSize()：每个元素大小，单位字节
			XData d((char*)frame.data, frame.cols*frame.rows*frame.elemSize(),GetCurTime());
			Push(d);

		}
	}

	bool Init(int camIndex = 0)
	{
		///1、使用opencv打开rtsp相机
		cam.open(camIndex); //index默认自带摄像头0，其他的外接摄像头一般是1.

		if (!cam.isOpened())
		{
			//没有打开，抛出异常
			cout<<"cam open failed!"<<endl;
			return false;
		}
		cout << camIndex << " cam open success" << endl;
		width= cam.get(CAP_PROP_FRAME_WIDTH); //视频流中帧的宽度
		height = cam.get(CAP_PROP_FRAME_HEIGHT); //视频流中帧的高度
		fps = cam.get(CAP_PROP_FPS); //帧率就是在1秒钟时间里传输的图片的帧数 CV_CAP_PROP_FPS 帧速率
		if (fps == 0) fps = 25; 
		return true;
	}

	bool Init(const char *url)
	{
		cam.open(url);

		if (!cam.isOpened())
		{
			//没有打开，抛出异常
			cout << "cam open failed!" << endl;
			return false;
		}
		cout << url << " cam open success" << endl;
		width = cam.get(CAP_PROP_FRAME_WIDTH);
		height = cam.get(CAP_PROP_FRAME_HEIGHT);
		fps = cam.get(CAP_PROP_FPS);
		if (fps == 0) fps = 25;
		return true;
	}
	
	void Stop()
	{
		XDataThread::Stop();

		if (cam.isOpened())
		{
			cam.release();
		}
	}

};

XVideoCapture *XVideoCapture::Get(unsigned char index)
{
	static CXVideoCapture xc[255];
	return &xc[index];
}

XVideoCapture::XVideoCapture()
{
}


XVideoCapture::~XVideoCapture()
{
}
