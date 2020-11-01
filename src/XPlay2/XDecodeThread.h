#pragma once
//公共解码模块
struct AVPacket;
class XDecode;

#include <list>
#include <mutex>
#include <QThread>

class XDecodeThread:public QThread
{
public:
	XDecodeThread();
	virtual ~XDecodeThread();
	virtual void Push(AVPacket *pkt);
	virtual void Clear(); //清理队列
	virtual void Close(); //清理资源，停止线程

	//取出一帧数据，并出栈，若没有返回NULL
	virtual AVPacket *Pop();

	//最大队列，避免内存溢出，只写不读
	int maxList = 100;
	bool isExit = false;
protected:
	XDecode *decode = 0;
	std::list<AVPacket *> packs;
	std::mutex mux;

};

