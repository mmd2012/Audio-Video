#include "XAudioThread.h"
#include "XDecode.h"
#include "XAudioPlay.h"
#include "XResample.h"
#include <iostream>
using namespace std;

void XAudioThread::Clear()
{
	XDecodeThread::Clear();

	amux.lock();
	if (ap) ap->Clear();
	amux.unlock();
}

//ֹͣ�߳�,������Դ
void XAudioThread::Close()
{
	XDecodeThread::Close();
	if (res)
	{
		res->Close();
		amux.lock();
		delete res;
		res = NULL;
		amux.unlock();
	}
	if (ap)
	{
		ap->Close();
		amux.lock();
		ap = NULL;
		amux.unlock();
	}
}

bool XAudioThread::Open(AVCodecParameters *para,int sampleRate, int channels)
{
	if (!para)return false;

	Clear();

	amux.lock();
	pts = 0;
	if (!decode) decode = new XDecode();
	if (!res) res = new XResample();
	if (!ap) ap = XAudioPlay::Get();
	bool re = true;
	if (!res->Open(para, false))
	{
		cout << "XResample open failed!" << endl;
		re = false;
	}
	ap->sampleRate = sampleRate;
	ap->channels = channels;
	if (!ap->Open())
	{
		re = false;
		cout << "XAudioPlay open failed!" << endl;
	}
	if (!decode->Open(para))
	{
		cout << "audio XDecode open failed!" << endl;
		re = false;
	}
	amux.unlock();
	cout << "XAudioThread::Open :" << re << endl;
	return re;
}

void XAudioThread::SetPause(bool isPause)
{
	//amux.lock();
	this->isPause = isPause;
	if (ap)
		ap->SetPause(isPause);
	//amux.unlock();
}

void XAudioThread::run()
{
	unsigned char *pcm = new unsigned char[1024*1024*10];
	while (!isExit)
	{
		amux.lock();
		if (this->isPause)
		{
			amux.unlock();
			msleep(5);
			continue;
		}
		//û������
		//if (packs.empty() || !decode || !res || !ap)
		//{
		//	amux.unlock();
		//	msleep(1);
		//	continue;
		//}

		//AVPacket *pkt = packs.front();
		//packs.pop_front();
		AVPacket *pkt = Pop();
		bool re = decode->Send(pkt);
		if (!re)
		{
			amux.unlock();
			msleep(1);
			continue;
		}
		//�п�����һ��send,���recv
		while (!isExit)
		{
			AVFrame *frame = decode->Recv(); //��decode�ж����pts��ʱ�򴫳���������AVFrame��ptsҪ����ͷ�ļ�
			if (!frame) break; //û����

			//�ڽ���ǰ��ȡ��Ƶ�Ĳ���ʱ�䣬��ΪQAudioPlay����Ҳ�л��壬����Ҫ��ȥ��������ʱ��
			//��ȥ������δ���ŵ�ʱ�䣬����
			pts = decode->pts - ap->GetNoPlayMs(); 

			//cout << "audio pts = " << pts << endl;

			//�����ˣ������ز���
			int size = res->Resample(frame,pcm);
			//������Ƶ
			while (!isExit)
			{
				if (size <= 0) break;
				//����δ���꣬�ռ䲻��
				if (ap->GetFree() < size || isPause) //��ͣ�˾Ͳ�Ҫ����д
				{
					msleep(1);
					continue;
				}
				ap->Write(pcm,size); //�ռ乻��ֱ������д��ȥ
				break;
			}
		}
		amux.unlock();
	}
	delete pcm;
}

XAudioThread::XAudioThread()
{
}


XAudioThread::~XAudioThread()
{
	//�ȴ��߳��˳�
	isExit = true;
	wait();
}
