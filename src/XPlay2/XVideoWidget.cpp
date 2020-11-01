#include "XVideoWidget.h"
#include <QDebug>
#include <QTimer>

extern "C" {
#include <libavutil/frame.h>
}

//自动加双引号
#define GET_STR(x) #x
#define A_VER	3
#define T_VER	4

FILE *fp = NULL;

//顶点shader
const char *vString = GET_STR(
	attribute vec4 vertexIn;
attribute vec2 textureIn;
varying vec2 textureOut;
void main(void)
{
	gl_Position = vertexIn;
	textureOut = textureIn;
}
);

//片元shader
const char *tString = GET_STR(
	varying vec2 textureOut;
uniform sampler2D tex_y;
uniform sampler2D tex_u;
uniform sampler2D tex_v;
void main(void)
{
	vec3 yuv;
	vec3 rgb;
	yuv.x = texture2D(tex_y, textureOut).r;
	yuv.y = texture2D(tex_u, textureOut).r - 0.5;
	yuv.z = texture2D(tex_v, textureOut).r - 0.5;
	rgb = mat3(1.0, 1.0, 1.0,
		0.0, -0.39465, 2.03211,
		1.13983, -0.58060, 0.0) * yuv;
	gl_FragColor = vec4(rgb, 1.0);
}

);


//准备数据
//ffmpeg -i v1080.mp4 -t 10 -s 240*128 -pix_fmt yuv420p out240*128.yuv
XVideoWidget::XVideoWidget(QWidget *parent)
	: QOpenGLWidget(parent)
{
}

XVideoWidget::~XVideoWidget()
{
}
void XVideoWidget::Repaint(AVFrame *frame) //重绘制
{
	//传入的是视频帧
	if (!frame) return;
	mux.lock();
	//容错保证尺寸正确，保证是视频帧，因为音频帧的宽高比是0
	if ( !datas[0] || width * height == 0 || frame->width != this->width || frame->height != this->height)
	{
		av_frame_free(&frame);
		mux.unlock();
		return;
	}

	if (width == frame->linesize[0]) //无需对齐
	{
		//将YUV数据拷贝到datas内存中，在paint()函数中会将内存datas的yuv数据与显存中的shader进行关联绑定，从而在显卡中显示出来
		memcpy(datas[0], frame->data[0], width*height);
		memcpy(datas[1], frame->data[1], width*height/4);
		memcpy(datas[2], frame->data[2], width*height/4);
	}
	else //行对齐问题
	{
		for (int i = 0; i < height; i++) //Y
			memcpy(datas[0] + width*i, frame->data[0] + frame->linesize[0] * i, width);
		for (int i = 0; i < height/2; i++) //U
			memcpy(datas[1] + width/2*i, frame->data[1] + frame->linesize[0] * i, width);
		for (int i = 0; i < height/2; i++) //V
			memcpy(datas[2] + width/2*i, frame->data[2] + frame->linesize[0] * i, width);
	}


	mux.unlock();
	av_frame_free(&frame);

	//刷新显示
	update();
}

void XVideoWidget::Init(int width, int height)
{
	mux.lock();
	this->width = width;
	this->height = height;
	delete datas[0];
	delete datas[1];
	delete datas[2];

	//分配材质内存空间
	datas[0] = new unsigned char[width*height];		//Y
	datas[1] = new unsigned char[width*height / 4];	//U
	datas[2] = new unsigned char[width*height / 4];	//V

	if (texs[0])
	{
		glDeleteTextures(3, texs);
	}

	//
	//创建材质，3个材质，地址
	glGenTextures(3, texs);

	//Y
	glBindTexture(GL_TEXTURE_2D, texs[0]); //材质类型的绑定，2D图像
	//放大过滤，线性插值 GL_NEAREST(效率高，但马赛克严重)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //设置材质属性 放大 ，GL_LINEAR线性缩放
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); //设置材质属性 缩小
	//创建材质显卡空间
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, 0);

	//U
	glBindTexture(GL_TEXTURE_2D, texs[1]);
	//放大过滤，线性插值
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//创建材质显卡空间
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width / 2, height / 2, 0, GL_RED, GL_UNSIGNED_BYTE, 0);

	//V
	glBindTexture(GL_TEXTURE_2D, texs[2]);
	//放大过滤，线性插值
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//创建材质显卡空间
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width / 2, height / 2, 0, GL_RED, GL_UNSIGNED_BYTE, 0);

	mux.unlock();
}

void XVideoWidget::initializeGL() //初始化GL
{
	qDebug() << "initializeGL";
	mux.lock();

	//初始化opengl(QOpenGLFunctions继承)函数
	initializeOpenGLFunctions();

	//program 加载shader（顶点和片元）脚本
	//片元（像素）
	qDebug() << program.addShaderFromSourceCode(QGLShader::Fragment, tString);

	//顶点shader
	qDebug() << program.addShaderFromSourceCode(QGLShader::Vertex, vString);

	//设置顶点坐标的变量
	program.bindAttributeLocation("vertexIn", A_VER);

	//设置材质坐标
	program.bindAttributeLocation("textureIn", T_VER);

	//编译shader()
	qDebug() << "program.link() = " << program.link();
	qDebug() << "program.bind() = " << program.bind();

	//传递顶点和材质坐标
	//顶点
	static const GLfloat ver[] = {
		-1.0f,-1.0f,
		1.0f,-1.0f,
		-1.0f,1.0f,
		1.0f,1.0f
	};

	//材质坐标
	static const GLfloat tex[] = {
		0.0f,1.0f,
		1.0f,1.0f,
		0.0f,0.0f,
		1.0f,0.0f
	};

	//顶坐标写入openGL中
	glVertexAttribPointer(A_VER, 2, GL_FLOAT, 0, 0, ver);
	glEnableVertexAttribArray(A_VER);

	//材质
	glVertexAttribPointer(T_VER, 2, GL_FLOAT, 0, 0, tex);
	glEnableVertexAttribArray(T_VER);

	//把材质具体处理，材质创建，关联到shader中
	//从shader中获取材质
	unis[0] = program.uniformLocation("tex_y");
	unis[1] = program.uniformLocation("tex_u");
	unis[2] = program.uniformLocation("tex_v");

	mux.unlock();



													//读取材质
	//fp = fopen("out240x128.yuv", "rb");
	//if (!fp)
	//{
	//	qDebug() << "out240x128.yuv file open failed!";
	//}

	////启动定时器
	//QTimer *ti = new QTimer(this);
	//connect(ti, SIGNAL(timeout()), this, SLOT(update()));
	//ti->start(40);


}

void XVideoWidget::paintGL() //刷新显示
{
	//if (feof(fp))
	//{
	//	fseek(fp, 0, SEEK_SET);
	//}

	//fread(datas[0], 1, width*height, fp);
	//fread(datas[1], 1, width*height / 4, fp);
	//fread(datas[2], 1, width*height / 4, fp);

	mux.lock();

	//写入和绘制材质
	glActiveTexture(GL_TEXTURE0); //激活第0层 激活材质
	glBindTexture(GL_TEXTURE_2D, texs[0]);//0层绑定到Y材质
	//修改材质内容（复制内存内容），与应用程序交互，通过opengl的材质传入到内存中，实际就是显存到内存
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RED, GL_UNSIGNED_BYTE, datas[0]);
	//与shader变量关联
	glUniform1i(unis[0], 0); //与shader相关联，绑定到0层

	glActiveTexture(GL_TEXTURE0 + 1); //激活第1层
	glBindTexture(GL_TEXTURE_2D, texs[1]);//1层绑定到U材质
										  //修改材质内容（复制内存内容）
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width / 2, height / 2, GL_RED, GL_UNSIGNED_BYTE, datas[1]);
	//与shader变量关联
	glUniform1i(unis[1], 1);

	glActiveTexture(GL_TEXTURE0 + 2); //激活第2层
	glBindTexture(GL_TEXTURE_2D, texs[2]);//2层绑定到V材质
										  //修改材质内容（复制内存内容）
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width / 2, height / 2, GL_RED, GL_UNSIGNED_BYTE, datas[2]);
	//与shader变量关联
	glUniform1i(unis[2], 2);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); //绘制，从0开始绘制4个顶点，将材质贴图都绘制出来

	qDebug() << "paintGL";
	mux.unlock();

}

void XVideoWidget::resizeGL(int width, int height) //窗口尺寸变化
{
	mux.lock();
	qDebug() << "resizeGL" << width << ";" << height;
	mux.unlock();
}
