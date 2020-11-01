#pragma once
#include "XDataThread.h"

enum AXUDIOTYPE
{
	X_AUDIO_QT
};

class XAudioRecord : public XDataThread
{
public:
	int channels = 2;			//ͨ����
	int sampleRate = 44100;		//������/������
	int sampleByte = 2;			//�����ֽڴ�С
	int nbSamples = 1024;		//һ֡��Ƶÿ��ͨ������������

	virtual bool Init() = 0;		//��ʼ¼��
	virtual void Stop() = 0;		//ֹͣ¼��

public:
	static XAudioRecord *Get(AXUDIOTYPE type = X_AUDIO_QT,unsigned char index = 0); //Ŀǰֻ֧��QT
	virtual ~XAudioRecord();

protected:
		XAudioRecord();

};

