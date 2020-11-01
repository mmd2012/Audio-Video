#include "XDataThread.h"

void XDataThread::Clear()
{
	mutex.lock();
	while (!datas.empty())
	{
		datas.front().Drop();
		datas.pop_front();
	}
	mutex.unlock();
}

//���б��β����
void XDataThread::Push(XData d)
{
	mutex.lock();
	if (datas.size() > maxList)
	{
		datas.front().Drop();
		datas.pop_front();
	}
	datas.push_back(d); //β���壬ͷ��ȡ
	mutex.unlock();

}
//��ȡ�б������������
XData XDataThread::Pop()
{
	//�����ȡ��ʱ����һ���߳��еģ���Ҫ����
	mutex.lock();
	if (datas.empty())
	{
		mutex.unlock();
		return XData();
	}
	XData d = datas.front();
	datas.pop_front();
	mutex.unlock();
	return d;
}

//�����߳�
bool XDataThread::Start()
{
	isExit = false;
	QThread::start();	//����֮��ͻ����run()��
	return true;
}

//�˳��߳�,���ȴ��߳��˳���������
void XDataThread::Stop()
{
	isExit = true;
	wait();	//�ȴ��߳��˳�


}

XDataThread::XDataThread()
{
}


XDataThread::~XDataThread()
{
}
