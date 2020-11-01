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

//停止线程,清理资源
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
		//没有数据
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
		//有可能是一次send,多次recv
		while (!isExit)
		{
			AVFrame *frame = decode->Recv(); //在decode中定义个pts到时候传出来，避免AVFrame读pts要加载头文件
			if (!frame) break; //没读到

			//在解码前获取音频的播放时间，因为QAudioPlay播放也有缓冲，所以要减去这个缓冲的时间
			//减去缓冲中未播放的时间，毫秒
			pts = decode->pts - ap->GetNoPlayMs(); 

			//cout << "audio pts = " << pts << endl;

			//读到了，进行重采样
			int size = res->Resample(frame,pcm);
			//播放音频
			while (!isExit)
			{
				if (size <= 0) break;
				//缓冲未播完，空间不够
				if (ap->GetFree() < size || isPause) //暂停了就不要往里写
				{
					msleep(1);
					continue;
				}
				ap->Write(pcm,size); //空间够就直接数据写进去
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
	//等待线程退出
	isExit = true;
	wait();
}
