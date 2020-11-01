#pragma once
#include <list>
#include <mutex>
#include <QThread>
#include "IVideoCall.h"
#include "XDecodeThread.h"

struct AVPacket;
struct AVCodecParameters;
class XDecode;


//�������ʾ��Ƶ
class XVideoThread : public XDecodeThread
{
public:
	
	//����pts��������յ��Ľ�������pts >= seekpts return true ������ʾ����
	virtual bool RepaintPts(AVPacket *pkt, long long seekpts);
	//�򿪣����ܳɹ��������
	virtual bool Open(AVCodecParameters *para, IVideoCall *call, int width, int height);
	//virtual void Stop();
	void run();

	XVideoThread();
	virtual ~XVideoThread();

	//ͬ��ʱ�䣬���ⲿ����,��ʵ������Ƶʱ��
	long long synpts = 0;

	void SetPause(bool isPause);
	bool isPause = false;

protected:
	IVideoCall *call = 0;
	std::mutex vmux;
};

