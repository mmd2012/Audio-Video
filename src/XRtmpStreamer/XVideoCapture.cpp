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
		Mat frame; //ֱ�Ӷ�ȡһ֡��������֡����
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

			//ȷ�������������� 
			fmutex.lock();
			for (int i = 0;i<filters.size(); i++)
			{
				Mat des;
				filters[i]->Filter(&frame, &des); //ԭͼ,Ŀ��
				frame = des; //������������
			}
			fmutex.unlock();
			//��ÿһ֡���棬������������������͵��б���

			//elemSize()��ÿ��Ԫ�ش�С����λ�ֽ�
			XData d((char*)frame.data, frame.cols*frame.rows*frame.elemSize(),GetCurTime());
			Push(d);

		}
	}

	bool Init(int camIndex = 0)
	{
		///1��ʹ��opencv��rtsp���
		cam.open(camIndex); //indexĬ���Դ�����ͷ0���������������ͷһ����1.

		if (!cam.isOpened())
		{
			//û�д򿪣��׳��쳣
			cout<<"cam open failed!"<<endl;
			return false;
		}
		cout << camIndex << " cam open success" << endl;
		width= cam.get(CAP_PROP_FRAME_WIDTH); //��Ƶ����֡�Ŀ��
		height = cam.get(CAP_PROP_FRAME_HEIGHT); //��Ƶ����֡�ĸ߶�
		fps = cam.get(CAP_PROP_FPS); //֡�ʾ�����1����ʱ���ﴫ���ͼƬ��֡�� CV_CAP_PROP_FPS ֡����
		if (fps == 0) fps = 25; 
		return true;
	}

	bool Init(const char *url)
	{
		cam.open(url);

		if (!cam.isOpened())
		{
			//û�д򿪣��׳��쳣
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
