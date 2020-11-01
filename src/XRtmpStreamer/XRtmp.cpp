#include "XRtmp.h"
#include <iostream>
#include <string>
using namespace std;

extern "C"
{
	#include <libavformat/avformat.h>
	#include <libavutil/time.h>
}
#pragma comment(lib,"avformat.lib")

class CXRtmp :public XRtmp
{
public:


	void Close()
	{
		if (ic)
		{
			avformat_close_input(&ic); //把ic置0
			vs = NULL;
		}
		vc = NULL;
		url = "";
	}

	//初始化封装器上下文
	bool Init(const char* url)
	{
		///5 输出封装器和视频流配置
		//a 创建输出封装器上下文
		int ret = avformat_alloc_output_context2(&ic, 0, "flv", url);
		this->url = url;
		if (ret != 0)
		{
			char buf[1024] = { 0 };
			av_strerror(ret, buf, sizeof(buf) - 1);
			cout << buf;
			return false;
		}
		return true;
	}

	//添加视频或者音频流 失败返回-1 成功返回流索引
	int AddStream(const AVCodecContext *c)
	{
		if (!c)return -1;
		
		//b 添加视频/音频流 ，此时流里没有任何信息
		AVStream *st = avformat_new_stream(ic, NULL);
		if (!st)
		{
			cout << "avformat_new_stream failed" << endl;
			return -1;
		}
		//视频流参数设置
		st->codecpar->codec_tag = 0; //编码格式设置0
									 //参数拷贝，从编码器复制参数到视频流信息里面
		avcodec_parameters_from_context(st->codecpar, c);
		av_dump_format(ic, 0, url.c_str(), 1);//打印一下

		if (c->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			vc = c;
			vs = st;
		}
		else if(c->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			ac = c;
			as = st;
		}
		return st->index;
	}

	//打开RTMP网络IO，发送封装头
	bool SendHeaad()
	{
		//6、打开rtmp的网络输出 打开网络的io
		int ret = avio_open(&ic->pb, url.c_str(), AVIO_FLAG_WRITE);
		if (ret != 0)
		{
			char buf[1024] = { 0 };
			av_strerror(ret, buf, sizeof(buf) - 1);
			cout<< buf<<endl;
			return false;
		}

		//写入封装头
		ret = avformat_write_header(ic, NULL);
		if (ret != 0)
		{
			char buf[1024] = { 0 };
			av_strerror(ret, buf, sizeof(buf) - 1);
			cout<< buf <<endl;
			return false;
		}
		return true;
	}

	bool SendFrame(XData d, int streamIndex)
	{
		if (!d.data || d.size <= 0) return false;
		AVPacket *pack = (AVPacket *)d.data;
		pack->stream_index = streamIndex;

		AVRational stime;
		AVRational dtime;

		//判断时音频，视频
		if (vs && vc && pack->stream_index == vs->index)
		{
			stime = vc->time_base;
			dtime = vs->time_base;
		}
		else if(as && ac && pack->stream_index == as->index)
		{
			stime = ac->time_base;
			dtime = as->time_base;
		}
		else
		{
			return false;
		}

		///推流 毫秒 //av_rescale_q（）把时间戳从一个时基调整到另外一个时基时候用的函数
		//之前采集时间基准时1/1000000;微妙级别； 推流时采用毫秒级别
		pack->pts = av_rescale_q(pack->pts, stime, dtime); //原始的pts是基于vc的timebase，改为基于stream vs的
		pack->dts = av_rescale_q(pack->dts, stime, dtime); //原始的pts是基于vc的timebase，改为基于stream vs的
		pack->duration = av_rescale_q(pack->duration, stime, dtime);

		int ret = av_interleaved_write_frame(ic, pack); //不管成功与否，都会把pack中的内存释放掉
		if (ret == 0)
		{
			cout << "#" << flush;
			return true;
		}

		return false;
	}

private:
	//rtmp flv 封装器
	AVFormatContext *ic = NULL;
	//视频编码器流
	const AVCodecContext *vc = NULL;

	//音频编码器流
	const AVCodecContext *ac = NULL;

	//视频流
	AVStream *vs = NULL;
	//音频流 
	AVStream *as = NULL;

	string url = "";

};

//工厂生产方法
XRtmp * XRtmp::Get(unsigned char index)
{
	static CXRtmp cxr[255];

	static bool isFirst = true;
	if (isFirst)
	{
		//注册所有的封装器
		av_register_all();

		//注册所有网络协议
		avformat_network_init();
		isFirst = false;
	}

	return &cxr[index];
}

XRtmp::XRtmp()
{
}


XRtmp::~XRtmp()
{
}
