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

//再列表结尾插入
void XDataThread::Push(XData d)
{
	mutex.lock();
	if (datas.size() > maxList)
	{
		datas.front().Drop();
		datas.pop_front();
	}
	datas.push_back(d); //尾部插，头部取
	mutex.unlock();

}
//读取列表中最早的数据
XData XDataThread::Pop()
{
	//插入和取出时不在一个线程中的，故要加锁
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

//启动线程
bool XDataThread::Start()
{
	isExit = false;
	QThread::start();	//启动之后就会进入run()中
	return true;
}

//退出线程,并等待线程退出（阻塞）
void XDataThread::Stop()
{
	isExit = true;
	wait();	//等待线程退出


}

XDataThread::XDataThread()
{
}


XDataThread::~XDataThread()
{
}
