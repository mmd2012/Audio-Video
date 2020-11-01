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

	//���ܳɹ���񣬶��ͷ�frame�ռ�
	virtual void Repaint(AVFrame *frame); //�ػ���

	XVideoWidget(QWidget *parent);
	~XVideoWidget();

protected:
	void paintGL(); //ˢ����ʾ
	void initializeGL(); //��ʼ��GL
	void resizeGL(int width, int height); //���ڳߴ�仯

private:
	std::mutex mux;
	//shader����
	QGLShaderProgram program;

	//�Կ��ռ�
	//shader�е�yuv������ַ
	GLuint unis[3] = { 0 };
	//opengl��textture��ַ
	GLuint texs[3] = { 0 };

	//�����ڴ�ռ�
	unsigned char *datas[3] = { 0 };

	int width = 240;
	int height = 128;
};
