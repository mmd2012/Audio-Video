#pragma once
#include"XData.h"

class AVCodecContext;

enum XSampleFmt
{
	X_S16 = 1,
	X_FLATP = 8
};

///����Ƶ����ӿ���
class XMediaEncode
{
public:
	//�������
	int inWidth = 1280;
	int inHeight = 720;
	int inPixSize = 3; //���ش�С
	int channels = 2;
	int sampleRate = 44100; //������
	XSampleFmt inSampleFmt = X_S16; ////�����������ʽ

	//�������
	int outWidth = 1280;
	int outHeight = 720;
	int bitrate = 4000000; //ѹ����ÿ���ǵ�bitλ��С 50kB
	int fps = 25;
	int nbSamples = 1024;
	XSampleFmt outSampleFmt = X_FLATP; //�����������ʽ


	//����ģʽ������Ĵ��������û����������޶��û���������
	//����������������ȡ���� ��һ��Ҫ��static
	static XMediaEncode * Get(unsigned char index = 0); //�������ڲ�������

	//�����е�ʵ�ֿ��Խ��䶼��װ�������ⲿ��������
	//��ο���������ȫ���ýӿڣ�Ȼ���ڲ�ʵ��
	//ΪʲôҪ���������������װ���ⲿ�ڵ���ʱ�Ͳ�������������ˡ�

	//��ʼ�����ظ�ʽת����������
	virtual bool InitScale() = 0;

	virtual bool InitResample() = 0; //��Ƶ�ز����������ĳ�ʼ��

	// ����ֵ�������������
	virtual XData Resample(XData d) = 0;

	// ����ֵ�������������
	virtual XData RGBToYUV(XData d) = 0;

	//��Ƶ��������ʼ��
	virtual bool InitVideoCodec() = 0;
	
	//��Ƶ��������ʼ��
	virtual bool InitAudioCodec() = 0;

	//��Ƶ���� ����ֵ�������������
	virtual XData EncodeVideo(XData frame) = 0;

	//��Ƶ���� ����ֵ�������������
	virtual XData EncodeAudio(XData frame) = 0;

	virtual void Close() = 0;

	virtual ~XMediaEncode();

	AVCodecContext *vc = 0; //��Ƶ������������
	AVCodecContext *ac = 0; //��Ƶ������������

protected:
	XMediaEncode();

};

