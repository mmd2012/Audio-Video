#include "xrtmpstreamer.h"
#include <iostream>
#include "XController.h"
using namespace std;
static bool isStream = false; //是否推流
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
		ui.startButton->setText(QString::fromLocal8Bit("开始"));
		XController::Get()->Stop(); //停止录制，停止推流

	}
	else
	{
		isStream = true;
		ui.startButton->setText(QString::fromLocal8Bit("停止"));
		QString url = ui.inUrl->text();
		bool ok = false;
		int camIndex = url.toInt(&ok);
		if (!ok)
		{
			XController::Get()->inUrl = url.toStdString(); //摄像机rtsp流
		}
		else
		{
			XController::Get()->camIndex = camIndex; //usb相机
		}
		XController::Get()->outUrl = ui.outUrl->text().toStdString();
		XController::Get()->Set("b", (ui.face->currentIndex()+1)*3);
		XController::Get()->Start();
	}
}
