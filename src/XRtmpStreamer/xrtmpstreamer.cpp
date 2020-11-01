#include "xrtmpstreamer.h"
#include <iostream>
#include "XController.h"
using namespace std;
static bool isStream = false; //�Ƿ�����
XRtmpStreamer::XRtmpStreamer(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
}

void XRtmpStreamer::Stream()
{
	cout << "Stream";
	if (isStream)
	{
		isStream = false;
		ui.startButton->setText(QString::fromLocal8Bit("��ʼ"));
		XController::Get()->Stop(); //ֹͣ¼�ƣ�ֹͣ����

	}
	else
	{
		isStream = true;
		ui.startButton->setText(QString::fromLocal8Bit("ֹͣ"));
		QString url = ui.inUrl->text();
		bool ok = false;
		int camIndex = url.toInt(&ok);
		if (!ok)
		{
			XController::Get()->inUrl = url.toStdString(); //�����rtsp��
		}
		else
		{
			XController::Get()->camIndex = camIndex; //usb���
		}
		XController::Get()->outUrl = ui.outUrl->text().toStdString();
		XController::Get()->Set("b", (ui.face->currentIndex()+1)*3);
		XController::Get()->Start();
	}
}
