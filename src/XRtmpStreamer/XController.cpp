#include "XController.h"
#include "XVideoCapture.h"
#include "XAudioRecord.h"
#include "XMediaEncode.h"
#include "XRtmp.h"
#include <iostream>

using namespace std;

void XController::run()
{
	long long beginTime = GetCurTime();

	while (!isExit)
	{
		//һ�ζ�ȡһ֡��Ƶ
		XData ad = XAudioRecord::Get()->Pop();
		XData vd = XVideoCapture::Get()->Pop();
		if (ad.size<=0 && vd.size<=0)
		{
			msleep(1);
			continue;
		}
		//������Ƶ
		if (ad.size > 0)
		{
			ad.pts = ad.pts - beginTime; //��ʱ���ܲ��Ǿ�ȷ�ģ�

			//�ز���Դ����
			XData pcm = XMediaEncode::Get()->Resample(ad);
			ad.Drop();
			XData pkt = XMediaEncode::Get()->EncodeAudio(pcm);
			if (pkt.size > 0)
			{
				////����
				if (XRtmp::Get()->SendFrame(pkt,aindex))
				{
					cout << "#" << flush;
				}
			}
		}
		//������Ƶ
		if (vd.size > 0)
		{
			vd.pts = vd.pts - beginTime; //���Ϳ��ܲ��Ǿ�ȷ��
			XData yuv = XMediaEncode::Get()->RGBToYUV(vd);
			vd.Drop();
			XData pkt = XMediaEncode::Get()->EncodeVideo(yuv);
			if (pkt.size> 0)
			{
				////����
				if (XRtmp::Get()->SendFrame(pkt,vindex))
				{
					cout << "@" << flush;
				}
			}
		}
	}
}


//�趨���ղ���
bool XController::Set(std::string key, double val)
{
	XFilter::Get()->Set(key, val);
	return true;
}

bool XController::Start() //�����߳� usb�����
{
	//1������ĥƤ������
	XVideoCapture::Get()->AddFilter(XFilter::Get()); //��ӹ�����
	cout << "1 ����ĥƤ������" << endl;
	//2�������
	if (camIndex >= 0)
	{
		if (!XVideoCapture::Get()->Init(camIndex))
		{
			err = "2��ϵͳ���ʧ��";
			cout << err << endl;
			return false;
		}
	}
	else if(!inUrl.empty())
	{
		if (!XVideoCapture::Get()->Init(inUrl.c_str()))
		{
			err = "2��";
			err += inUrl;
			err += "���ʧ��";
			cout << err << endl;
			return false;
		}
	}
	else
	{
		err = "2�������������";
		cout << err << endl;
		return false;
	}
	cout << "2����򿪳ɹ�" << endl;
	
	///3 qt��Ƶ��ʼ¼��
	if (!XAudioRecord::Get()->Init())
	{
		err = "3¼���豸��ʧ��";
		cout << err << endl;
		return false;
	}
	cout << "3¼���豸�򿪳ɹ�" << endl;

	//11����������Ƶ¼���߳�
	// ��Ƶ��initʱ�Ϳ�ʼ¼����Ƶ�ˣ��ȵ������߳�ȥ��ʱ�������Ѿ��кܶ������ˣ�
	XAudioRecord::Get()->Start();  //����֮���������,clear
	XVideoCapture::Get()->Start(); //�ɼ����ݣ������߳�

	//����Ƶ������
	//4����ʼ����ʽת��������
	XMediaEncode::Get()->inWidth = XVideoCapture::Get()->width; //���������ȡ�Ŀ��
	XMediaEncode::Get()->inHeight = XVideoCapture::Get()->height; //���������ȡ�Ŀ��
	XMediaEncode::Get()->outWidth = XVideoCapture::Get()->width; //���������ȡ�Ŀ��
	XMediaEncode::Get()->outHeight = XVideoCapture::Get()->height; //���������ȡ�Ŀ��
	if (!XMediaEncode::Get()->InitScale()) //��ʼ��
	{
		err = "4��Ƶ���ظ�ʽת����ʧ��!";
		cout << err << endl;
		return false;
	}
	cout << "4��Ƶ���ظ�ʽת���򿪳ɹ�!" << endl;
	///5 ��Ƶ�ز��� �����ĳ�ʼ��
	XMediaEncode::Get()->channels = XAudioRecord::Get()->channels;
	XMediaEncode::Get()->nbSamples = XAudioRecord::Get()->nbSamples;
	XMediaEncode::Get()->sampleRate = XAudioRecord::Get()->sampleRate;
	if (!XMediaEncode::Get()->InitResample()) //�ز���
	{
		err = "5��Ƶ�ز��������ĳ�ʼ��ʧ��!";
		cout << err << endl;
		return false;
	}
	cout << "5��Ƶ�ز��������ĳ�ʼ���ɹ�!" << endl;
	///6��ʼ����Ƶ������
	if (!XMediaEncode::Get()->InitAudioCodec())
	{
		err = "6��ʼ����Ƶ������ʧ��!";
		cout << err << endl;
		return false;
	}
	cout << "��ʼ����Ƶ�������ɹ�!" << endl;

	//7����ʼ����Ƶ������
	if (!XMediaEncode::Get()->InitVideoCodec()) 
	{
		err = "7��ʼ����ƵƵ������ʧ��!";
		cout << err << endl;
		return false;
	}
	cout << "7����Ƶ��������ʼ���ɹ�!" << endl;
	/// 8���������װ��������
	if (!XRtmp::Get()->Init(outUrl.c_str()))
	{
		err = "8���������װ��������ʧ��!";
		cout << err << endl;
		return false;
	}
	cout << "8���������װ�������ĳɹ�!" << endl;

	//9�������Ƶ������Ƶ��
	vindex = XRtmp::Get()->AddStream(XMediaEncode::Get()->vc); //��������Ƶ���ı�����
	aindex = XRtmp::Get()->AddStream(XMediaEncode::Get()->ac); //��������Ƶ���ı�����
	if (vindex < 0 || aindex < 0)
	{
		err = "9�������Ƶ��ʧ��!";
		cout << err << endl;
		return false;
	}
	cout << "9�������Ƶ���ɹ�!" << endl;

	//10����rtmp����io,���ͷ�װͷ
	if (!XRtmp::Get()->SendHeaad())
	{
		err = "10�����ͷ�װͷʧ��!";
		cout << err << endl;
		return false;
	}

	XAudioRecord::Get()->Clear();
	XVideoCapture::Get()->Clear();
	XDataThread::Start(); //�ڲ����߳�ʵ�ֺܷ���

	return true;
}

void XController::Stop()
{
	XDataThread::Stop();
	XAudioRecord::Get()->Stop();
	XVideoCapture::Get()->Stop();
	XMediaEncode::Get()->Close();
	XRtmp::Get()->Close();

	camIndex = -1;
	inUrl = "";
	return;
}

XController::XController()
{
}


XController::~XController()
{
}
