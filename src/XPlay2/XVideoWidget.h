#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QGLShaderProgram>
#include <mutex>
#include "IVideoCall.h"

struct AVFrame;
class XVideoWidget : public QOpenGLWidget, protected QOpenGLFunctions, public IVideoCall
{
	Q_OBJECT

public:
	virtual void Init(int width, int height);

	//不管成功与否，都释放frame空间
	virtual void Repaint(AVFrame *frame); //重绘制

	XVideoWidget(QWidget *parent);
	~XVideoWidget();

protected:
	void paintGL(); //刷新显示
	void initializeGL(); //初始化GL
	void resizeGL(int width, int height); //窗口尺寸变化

private:
	std::mutex mux;
	//shader程序
	QGLShaderProgram program;

	//显卡空间
	//shader中的yuv变量地址
	GLuint unis[3] = { 0 };
	//opengl的textture地址
	GLuint texs[3] = { 0 };

	//材质内存空间
	unsigned char *datas[3] = { 0 };

	int width = 240;
	int height = 128;
};
