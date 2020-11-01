#pragma once
#include"XData.h"

class AVCodecContext;

enum XSampleFmt
{
	X_S16 = 1,
	X_FLATP = 8
};

///音视频编码接口类
class XMediaEncode
{
public:
	//输入参数
	int inWidth = 1280;
	int inHeight = 720;
	int inPixSize = 3; //像素大小
	int channels = 2;
	int sampleRate = 44100; //样本率
	XSampleFmt inSampleFmt = X_S16; ////输入的样本格式

	//输出参数
	int outWidth = 1280;
	int outHeight = 720;
	int bitrate = 4000000; //压缩后每秒是的bit位大小 50kB
	int fps = 25;
	int nbSamples = 1024;
	XSampleFmt outSampleFmt = X_FLATP; //输出的样本格式


	//工厂模式，对象的创建不由用户来创建（限定用户创建对象）
	//工厂生产方法来获取对象 ，一定要是static
	static XMediaEncode * Get(unsigned char index = 0); //对象由内部来创建

	//对象当中的实现可以将其都封装掉，由外部看不出来
	//如何看不出来？全部用接口，然后内部实现
	//为什么要看不出来？将其封装后，外部在调用时就不用依赖多个项了。

	//初始化像素格式转换的上下文
	virtual bool InitScale() = 0;

	virtual bool InitResample() = 0; //音频重采样，上下文初始化

	// 返回值无需调用者清理
	virtual XData Resample(XData d) = 0;

	// 返回值无需调用者清理
	virtual XData RGBToYUV(XData d) = 0;

	//视频编码器初始化
	virtual bool InitVideoCodec() = 0;
	
	//音频编码器初始化
	virtual bool InitAudioCodec() = 0;

	//视频编码 返回值无需调用者清理
	virtual XData EncodeVideo(XData frame) = 0;

	//音频编码 返回值无需调用者清理
	virtual XData EncodeAudio(XData frame) = 0;

	virtual void Close() = 0;

	virtual ~XMediaEncode();

	AVCodecContext *vc = 0; //视频编码器上下文
	AVCodecContext *ac = 0; //音频编码器上下文

protected:
	XMediaEncode();

};

