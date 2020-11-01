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
	//清理缓存
	Clear();

	mux.lock();
	bool status = this->isPause;
	mux.unlock();

	//暂停
	SetPause(true);

	mux.lock();
	if (demux)
		demux->Seek(pos);
	//实际要显示的位置pts
	long long seekPts = pos * demux->totalMs;
	while (!isExit)
	{
		AVPacket *pkt = demux->ReadVideo(); //读取一帧
		if (!pkt) break;

		//如果解码到seekpts
		if (vt->RepaintPts(pkt, seekPts))
		{
			this->pts = seekPts;
			break;
		}
		//bool re = vt->decode->Send(pkt); //发送出去，进行解码
		//if (!re) break;
		//AVFrame *frame = vt->decode->Recv(); //接收
		//if (!frame)  continue;//接收失败
		////到达位置，显示
		//if (frame->pts >= seekPts)
		//{
		//	this->pts = frame->pts;
		//	vt->call->Repaint(frame);
		//	break;
		//}
		//av_frame_free(&frame);
	}

	mux.unlock();

	//seek是非暂停状态
	if (!status)
		SetPause(false); //恢复播放键
}

void XDemuxThread::SetPause(bool isPause)
{
	mux.lock();
	this->isPause = isPause;
	if (at) at->SetPause(isPause);
	if (vt) vt->SetPause(isPause);

	mux.unlock();
}

void XDemuxThread::run() //具体的处理线程
{
	while (!isExit)
	{
		mux.lock(); //有多次打开的问题，不能确保线程打开的次序
		if (isPause)
		{
			mux.unlock();
			msleep(5);
			continue;
		}

		if (!demux) //代表还没有获取到多媒体文件的相关信息
		{
			mux.unlock();
			msleep(5);
			continue;
		}

		//音视频同步
		if (vt && at)
		{
			pts = at->pts; 
			vt->synpts = at->pts; //同步视频时间到音频时间上
		}

		AVPacket *pkt = demux->Read();
		if (!pkt) //表示读到结尾了
		{
			mux.unlock();
			msleep(5);
			continue;
		}
		//判断数据是音频
		if (demux->IsAudio(pkt))
		{
			if(at)
				at->Push(pkt);
		}
		else //视频
		{
			if (vt)
				vt->Push(pkt);
		}

		mux.unlock();
		msleep(1); //1毫秒
	}
}

bool XDemuxThread::Open(const char *url, IVideoCall *call)
{
	if (url == 0 || url[0] == '\0') //表示url为空串
		return false;

	mux.lock();

	//打开解封装
	bool re = demux->Open(url);
	if (!re)
	{
		cout << "demux->Open(url) failed!" << endl;
		mux.unlock();
		return false;
	}

	//启动视频部分 打开视频解码器和处理线程
	if (!vt->Open(demux->CopyVPara(), call, demux->width, demux->height))
	{
		re = false;
		cout << "vt->Open failed!" << endl;
	}

	//打开音频解码器和处理线程
	if (!at->Open(demux->CopyAPara(), demux->sampleRate, demux->channels))
	{
		re = false;
		cout << "at->Open failed!" << endl;
	}
	totalMs = demux->totalMs;
	mux.unlock();
	cout << "XDemuxThread::Open：" << re << endl;
	return re;
}

void XDemuxThread::Close() //关闭线程，清理资源
{
	isExit = true;
	wait(); //等待当前线程退出

	if (vt) vt->Close();
	if (at) at->Close();
	
	mux.lock();
	delete vt;
	delete at;
	vt = NULL;
	at = NULL;
	mux.unlock();
}

void XDemuxThread::Start() //启动所有线程，vt,at的线程
{
	mux.lock();
	if (!demux)	demux = new XDemux();
	if (!vt)	vt = new XVideoThread();
	if (!at)	at = new XAudioThread();

	//启动当前自己的线程
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
	//防止线程down掉
	isExit = true;
	wait();
}
