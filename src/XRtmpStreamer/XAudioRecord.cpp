#include "XAudioRecord.h"
#include <QAudioInput>
#include <iostream>
#include <QMutex>
#include <list>  //�����ɾ�����ܿ�

using namespace std;


class CXAudioRecord :public XAudioRecord
{
public:
	
	void run()
	{
		cout << "������Ƶ¼���߳�" << endl;

		//һ�ζ�ȡһ֡��Ƶ���ֽ���
		int readSize = nbSamples * channels * sampleByte; //һ֡��ȡ���ٴ� * ͨ���� * ��Ÿ�ʽ�ֽ���
		char *buf = new char[readSize];

		while (!isExit) //��������Ƶ�´ν������˳���
		{
			//��ȡ��¼�Ƶ���Ƶ
			//һ�ζ�ȡһ֡��Ƶ
			if (input->bytesReady() < readSize) //bytesReady()���ڲ鿴������˷��пɶ���������
			{
				QThread::msleep(1);
				continue;
			}

			int size = 0; //�Ѿ����˶���
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

			//�Ѿ���ȡһ֡��Ƶ,�洢
			long long pts = GetCurTime(); //ÿ����һ֡�ͼ�ʱ����
			Push(XData(buf, readSize,pts));
		}
		delete buf;
		cout << "�˳���Ƶ¼���߳�" << endl;

	}

	bool Init()		//��ʼ¼��
	{
		Stop(); 

		//1��qt��Ƶ��ʼ¼��
		QAudioFormat fmt;
		fmt.setSampleRate(sampleRate); //����������
		fmt.setChannelCount(channels); //����ͨ����
		fmt.setSampleSize(sampleByte * 8); //������С��2�ֽ�
		fmt.setCodec("audio/pcm");//������ʽ
		fmt.setByteOrder(QAudioFormat::LittleEndian); //�����ֽ���
		fmt.setSampleType(QAudioFormat::UnSignedInt);//����������ʽ

		//�ж��豸�Ƿ�֧��
		QAudioDeviceInfo info = QAudioDeviceInfo::defaultInputDevice(); //��ȡ��Ƶ�豸
		if (!info.isFormatSupported(fmt)) //�Ƿ�֧��fmt
		{
			cout << "Audio Format not support!" << endl;
			fmt = info.nearestFormat(fmt); //ȡ����������÷���
		}
		input = new QAudioInput(fmt);
		//��ʼ¼����Ƶ��
		io = input->start(); //��ʼ�ɼ����������д���
		if (!io)
			return false;

		return true;
	}

	void Stop()		//ֹͣ¼��
	{
		XDataThread::Stop();

		if (input)
			input->stop();
		if (io)
			io->close();
		input = NULL;
		io = NULL;
	}
	QAudioInput *input = NULL; //QT¼����Ƶ
	//��ʼ¼����Ƶ��
	QIODevice *io = NULL; //��ʼ�ɼ����������д���

};

XAudioRecord *XAudioRecord::Get(AXUDIOTYPE type, unsigned char index) //Ŀǰֻ֧��QT
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
