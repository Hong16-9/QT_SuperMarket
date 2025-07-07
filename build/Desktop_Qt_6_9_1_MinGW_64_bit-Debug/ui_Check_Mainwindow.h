/********************************************************************************
** Form generated from reading UI file 'Check_Mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.9.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CHECK_MAINWINDOW_H
#define UI_CHECK_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QListView>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Check_Mainwindow
{
public:
    QWidget *centralwidget;
    QListView *settlelistView;
    QGroupBox *groupBox;
    QListWidget *chooselistWidget;
    QPushButton *pushButton_5;
    QPushButton *pushButton_6;
    QPushButton *pushButton_7;
    QPushButton *pushButton_8;
    QGroupBox *groupBox_2;
    QPushButton *pushButton_2;
    QPushButton *pushButton;
    QPushButton *pushButton_3;
    QPushButton *pushButton_4;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *Check_Mainwindow)
    {
        if (Check_Mainwindow->objectName().isEmpty())
            Check_Mainwindow->setObjectName("Check_Mainwindow");
        Check_Mainwindow->resize(900, 600);
        centralwidget = new QWidget(Check_Mainwindow);
        centralwidget->setObjectName("centralwidget");
        settlelistView = new QListView(centralwidget);
        settlelistView->setObjectName("settlelistView");
        settlelistView->setGeometry(QRect(100, 20, 300, 500));
        groupBox = new QGroupBox(centralwidget);
        groupBox->setObjectName("groupBox");
        groupBox->setGeometry(QRect(400, 20, 501, 501));
        chooselistWidget = new QListWidget(groupBox);
        chooselistWidget->setObjectName("chooselistWidget");
        chooselistWidget->setGeometry(QRect(0, 30, 500, 471));
        chooselistWidget->setViewMode(QListView::ViewMode::IconMode);
        pushButton_5 = new QPushButton(groupBox);
        pushButton_5->setObjectName("pushButton_5");
        pushButton_5->setGeometry(QRect(20, 0, 75, 24));
        pushButton_6 = new QPushButton(groupBox);
        pushButton_6->setObjectName("pushButton_6");
        pushButton_6->setGeometry(QRect(110, 0, 75, 24));
        pushButton_7 = new QPushButton(groupBox);
        pushButton_7->setObjectName("pushButton_7");
        pushButton_7->setGeometry(QRect(220, 0, 75, 24));
        pushButton_8 = new QPushButton(groupBox);
        pushButton_8->setObjectName("pushButton_8");
        pushButton_8->setGeometry(QRect(340, 0, 75, 24));
        groupBox_2 = new QGroupBox(centralwidget);
        groupBox_2->setObjectName("groupBox_2");
        groupBox_2->setGeometry(QRect(0, 20, 100, 500));
        pushButton_2 = new QPushButton(groupBox_2);
        pushButton_2->setObjectName("pushButton_2");
        pushButton_2->setGeometry(QRect(10, 50, 81, 51));
        pushButton = new QPushButton(groupBox_2);
        pushButton->setObjectName("pushButton");
        pushButton->setGeometry(QRect(10, 170, 81, 51));
        pushButton_3 = new QPushButton(groupBox_2);
        pushButton_3->setObjectName("pushButton_3");
        pushButton_3->setGeometry(QRect(10, 280, 81, 51));
        pushButton_4 = new QPushButton(groupBox_2);
        pushButton_4->setObjectName("pushButton_4");
        pushButton_4->setGeometry(QRect(10, 400, 81, 51));
        Check_Mainwindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(Check_Mainwindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 900, 22));
        Check_Mainwindow->setMenuBar(menubar);
        statusbar = new QStatusBar(Check_Mainwindow);
        statusbar->setObjectName("statusbar");
        Check_Mainwindow->setStatusBar(statusbar);

        retranslateUi(Check_Mainwindow);

        QMetaObject::connectSlotsByName(Check_Mainwindow);
    } // setupUi

    void retranslateUi(QMainWindow *Check_Mainwindow)
    {
        Check_Mainwindow->setWindowTitle(QCoreApplication::translate("Check_Mainwindow", "MainWindow", nullptr));
        groupBox->setTitle(QString());
        pushButton_5->setText(QCoreApplication::translate("Check_Mainwindow", "PushButton", nullptr));
        pushButton_6->setText(QCoreApplication::translate("Check_Mainwindow", "PushButton", nullptr));
        pushButton_7->setText(QCoreApplication::translate("Check_Mainwindow", "PushButton", nullptr));
        pushButton_8->setText(QCoreApplication::translate("Check_Mainwindow", "PushButton", nullptr));
        groupBox_2->setTitle(QString());
        pushButton_2->setText(QCoreApplication::translate("Check_Mainwindow", "\346\224\271\346\225\260", nullptr));
        pushButton->setText(QCoreApplication::translate("Check_Mainwindow", "\345\210\240\351\231\244", nullptr));
        pushButton_3->setText(QCoreApplication::translate("Check_Mainwindow", "\346\270\205\347\251\272", nullptr));
        pushButton_4->setText(QCoreApplication::translate("Check_Mainwindow", "\346\224\257\344\273\230", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Check_Mainwindow: public Ui_Check_Mainwindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CHECK_MAINWINDOW_H
