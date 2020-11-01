#include "XAudioRecord.h"
#include <QAudioInput>
#include <iostream>
#include <QMutex>
#include <list>  //插入和删除都很快

using namespace std;


class CXAudioRecord :public XAudioRecord
{
public:
	
	void run()
	{
		cout << "进入音频录制线程" << endl;

		//一次读取一帧音频的字节数
		int readSize = nbSamples * channels * sampleByte; //一帧采取多少次 * 通道数 * 存放格式字节数
		char *buf = new char[readSize];

		while (!isExit) //处理完音频下次进来就退出了
		{
			//读取已录制的音频
			//一次读取一帧音频
			if (input->bytesReady() < readSize) //bytesReady()用于查看可以麦克风中可读的数据量
			{
				QThread::msleep(1);
				continue;
			}

			int size = 0; //已经读了多少
			while (size != readSize)
			{
				int len = io->read(buf + size, readSize - size);
				if (len < 0) break;
				size += len;
			}
			if (size != readSize)
			{
				continue;
			}

			//已经读取一帧音频,存储
			long long pts = GetCurTime(); //每读到一帧就计时下来
			Push(XData(buf, readSize,pts));
		}
		delete buf;
		cout << "退出音频录制线程" << endl;

	}

	bool Init()		//开始录制
	{
		Stop(); 

		//1、qt音频开始录制
		QAudioFormat fmt;
		fmt.setSampleRate(sampleRate); //设置样本率
		fmt.setChannelCount(channels); //设置通道数
		fmt.setSampleSize(sampleByte * 8); //样本大小，2字节
		fmt.setCodec("audio/pcm");//采样格式
		fmt.setByteOrder(QAudioFormat::LittleEndian); //设置字节序
		fmt.setSampleType(QAudioFormat::UnSignedInt);//设置样本格式

		//判断设备是否支持
		QAudioDeviceInfo info = QAudioDeviceInfo::defaultInputDevice(); //获取音频设备
		if (!info.isFormatSupported(fmt)) //是否支持fmt
		{
			cout << "Audio Format not support!" << endl;
			fmt = info.nearestFormat(fmt); //取到最靠近的配置方法
		}
		input = new QAudioInput(fmt);
		//开始录制音频了
		io = input->start(); //开始采集，往缓冲中存了
		if (!io)
			return false;

		return true;
	}

	void Stop()		//停止录制
	{
		XDataThread::Stop();

		if (input)
			input->stop();
		if (io)
			io->close();
		input = NULL;
		io = NULL;
	}
	QAudioInput *input = NULL; //QT录制音频
	//开始录制音频了
	QIODevice *io = NULL; //开始采集，往缓冲中存了

};

XAudioRecord *XAudioRecord::Get(AXUDIOTYPE type, unsigned char index) //目前只支持QT
{
	static CXAudioRecord record[255];
	return &record[index];
}

XAudioRecord::XAudioRecord()
{
}


XAudioRecord::~XAudioRecord()
{
}
