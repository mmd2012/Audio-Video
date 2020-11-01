#pragma once

struct AVCodecParameters;
struct AVCodecContext;
struct AVFrame;
struct AVPacket;
#include <mutex>

extern void XFreePacket(AVPacket **pkt);
extern void XFreeFrame(AVFrame **frame);
class XDecode
{
public:
	bool isAudio = false;
	
	//��ǰ���뵽��pts
	long long pts = 0;

	//�򿪽����������ܳɹ�����ͷ�para�ռ�
	virtual bool Open(AVCodecParameters *para);

	//���͵������߳��У�����pkt�ռ䣬���ܳɹ�����ͷ�pkt�ռ䣨�����ý�����ݣ�
	virtual bool Send(AVPacket *pkt);
	//��ȡ�������ݣ�һ��send������Ҫ���recv,��ȡ�����е����� Send NULL�� Recv���
	//ÿ�θ���һ�ݣ��ɵ������ͷ� av_frame_free
	virtual AVFrame* Recv();

	virtual void Close();
	virtual void Clear();

	XDecode();
	virtual ~XDecode();
private:
	AVCodecContext *codec = 0;
	std::mutex mux;
};
