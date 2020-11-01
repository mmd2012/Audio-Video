/********************************************************************************
** Form generated from reading UI file 'XPlay2.ui'
**
** Created by: Qt User Interface Compiler version 5.9.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_XPLAY2_H
#define UI_XPLAY2_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>
#include "XSlider.h"
#include "XVideoWidget.h"

QT_BEGIN_NAMESPACE

class Ui_XPlay2Class
{
public:
    XVideoWidget *video;
    QPushButton *openFile;
    XSlider *playPos;
    QPushButton *isPlay;

    void setupUi(QWidget *XPlay2Class)
    {
        if (XPlay2Class->objectName().isEmpty())
            XPlay2Class->setObjectName(QStringLiteral("XPlay2Class"));
        XPlay2Class->resize(1280, 720);
        video = new XVideoWidget(XPlay2Class);
        video->setObjectName(QStringLiteral("video"));
        video->setGeometry(QRect(0, 0, 1280, 720));
        openFile = new QPushButton(XPlay2Class);
        openFile->setObjectName(QStringLiteral("openFile"));
        openFile->setGeometry(QRect(140, 620, 150, 46));
        playPos = new XSlider(XPlay2Class);
        playPos->setObjectName(QStringLiteral("playPos"));
        playPos->setGeometry(QRect(10, 670, 1261, 16));
        playPos->setMaximum(999);
        playPos->setOrientation(Qt::Horizontal);
        isPlay = new QPushButton(XPlay2Class);
        isPlay->setObjectName(QStringLiteral("isPlay"));
        isPlay->setGeometry(QRect(380, 620, 150, 46));

        retranslateUi(XPlay2Class);
        QObject::connect(openFile, SIGNAL(clicked()), XPlay2Class, SLOT(OpenFile()));
        QObject::connect(isPlay, SIGNAL(clicked()), XPlay2Class, SLOT(PlayOrPause()));
        QObject::connect(playPos, SIGNAL(sliderPressed()), XPlay2Class, SLOT(SliderPress()));
        QObject::connect(playPos, SIGNAL(sliderReleased()), XPlay2Class, SLOT(SliderRelease()));

        QMetaObject::connectSlotsByName(XPlay2Class);
    } // setupUi

    void retranslateUi(QWidget *XPlay2Class)
    {
        XPlay2Class->setWindowTitle(QApplication::translate("XPlay2Class", "XPlay2", Q_NULLPTR));
        openFile->setText(QApplication::translate("XPlay2Class", "\346\211\223\345\274\200\346\226\207\344\273\266", Q_NULLPTR));
        isPlay->setText(QApplication::translate("XPlay2Class", "\346\222\255\346\224\276", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class XPlay2Class: public Ui_XPlay2Class {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_XPLAY2_H
