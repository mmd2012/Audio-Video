#include "XDemuxThread.h"
#include "XDemux.h"
#include "XVideoThread.h"
#include "XAudioThread.h"
#include <iostream>
#include "XDecode.h"
extern "C"
{
	#include <libavformat/avformat.h>
}
using namespace std;

void XDemuxThread::Clear()
{
	mux.lock();
	if (demux)demux->Clear();
	if (vt) vt->Clear();
	if (at) at->Clear();

	mux.unlock();
}

void XDemuxThread::Seek(double pos)
{
	//������
	Clear();

	mux.lock();
	bool status = this->isPause;
	mux.unlock();

	//��ͣ
	SetPause(true);

	mux.lock();
	if (demux)
		demux->Seek(pos);
	//ʵ��Ҫ��ʾ��λ��pts
	long long seekPts = pos * demux->totalMs;
	while (!isExit)
	{
		AVPacket *pkt = demux->ReadVideo(); //��ȡһ֡
		if (!pkt) break;

		//������뵽seekpts
		if (vt->RepaintPts(pkt, seekPts))
		{
			this->pts = seekPts;
			break;
		}
		//bool re = vt->decode->Send(pkt); //���ͳ�ȥ�����н���
		//if (!re) break;
		//AVFrame *frame = vt->decode->Recv(); //����
		//if (!frame)  continue;//����ʧ��
		////����λ�ã���ʾ
		//if (frame->pts >= seekPts)
		//{
		//	this->pts = frame->pts;
		//	vt->call->Repaint(frame);
		//	break;
		//}
		//av_frame_free(&frame);
	}

	mux.unlock();

	//seek�Ƿ���ͣ״̬
	if (!status)
		SetPause(false); //�ָ����ż�
}

void XDemuxThread::SetPause(bool isPause)
{
	mux.lock();
	this->isPause = isPause;
	if (at) at->SetPause(isPause);
	if (vt) vt->SetPause(isPause);

	mux.unlock();
}

void XDemuxThread::run() //����Ĵ����߳�
{
	while (!isExit)
	{
		mux.lock(); //�ж�δ򿪵����⣬����ȷ���̴߳򿪵Ĵ���
		if (isPause)
		{
			mux.unlock();
			msleep(5);
			continue;
		}

		if (!demux) //����û�л�ȡ����ý���ļ��������Ϣ
		{
			mux.unlock();
			msleep(5);
			continue;
		}

		//����Ƶͬ��
		if (vt && at)
		{
			pts = at->pts; 
			vt->synpts = at->pts; //ͬ����Ƶʱ�䵽��Ƶʱ����
		}

		AVPacket *pkt = demux->Read();
		if (!pkt) //��ʾ������β��
		{
			mux.unlock();
			msleep(5);
			continue;
		}
		//�ж���������Ƶ
		if (demux->IsAudio(pkt))
		{
			if(at)
				at->Push(pkt);
		}
		else //��Ƶ
		{
			if (vt)
				vt->Push(pkt);
		}

		mux.unlock();
		msleep(1); //1����
	}
}

bool XDemuxThread::Open(const char *url, IVideoCall *call)
{
	if (url == 0 || url[0] == '\0') //��ʾurlΪ�մ�
		return false;

	mux.lock();

	//�򿪽��װ
	bool re = demux->Open(url);
	if (!re)
	{
		cout << "demux->Open(url) failed!" << endl;
		mux.unlock();
		return false;
	}

	//������Ƶ���� ����Ƶ�������ʹ����߳�
	if (!vt->Open(demux->CopyVPara(), call, demux->width, demux->height))
	{
		re = false;
		cout << "vt->Open failed!" << endl;
	}

	//����Ƶ�������ʹ����߳�
	if (!at->Open(demux->CopyAPara(), demux->sampleRate, demux->channels))
	{
		re = false;
		cout << "at->Open failed!" << endl;
	}
	totalMs = demux->totalMs;
	mux.unlock();
	cout << "XDemuxThread::Open��" << re << endl;
	return re;
}

void XDemuxThread::Close() //�ر��̣߳�������Դ
{
	isExit = true;
	wait(); //�ȴ���ǰ�߳��˳�

	if (vt) vt->Close();
	if (at) at->Close();
	
	mux.lock();
	delete vt;
	delete at;
	vt = NULL;
	at = NULL;
	mux.unlock();
}

void XDemuxThread::Start() //���������̣߳�vt,at���߳�
{
	mux.lock();
	if (!demux)	demux = new XDemux();
	if (!vt)	vt = new XVideoThread();
	if (!at)	at = new XAudioThread();

	//������ǰ�Լ����߳�
	QThread::start();
	if (vt) vt->start();
	if (at) at->start();
	mux.unlock();
}

XDemuxThread::XDemuxThread()
{
}


XDemuxThread::~XDemuxThread()
{
	//��ֹ�߳�down��
	isExit = true;
	wait();
}
