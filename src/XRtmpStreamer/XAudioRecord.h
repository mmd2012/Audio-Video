#pragma once
#include "XDataThread.h"

enum AXUDIOTYPE
{
	X_AUDIO_QT
};

class XAudioRecord : public XDataThread
{
public:
	int channels = 2;			//通道数
	int sampleRate = 44100;		//样本率/采样率
	int sampleByte = 2;			//样本字节大小
	int nbSamples = 1024;		//一帧音频每个通道的样本数量

	virtual bool Init() = 0;		//开始录制
	virtual void Stop() = 0;		//停止录制

public:
	static XAudioRecord *Get(AXUDIOTYPE type = X_AUDIO_QT,unsigned char index = 0); //目前只支持QT
	virtual ~XAudioRecord();

protected:
		XAudioRecord();

};

