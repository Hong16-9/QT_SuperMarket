#include <QApplication>
#include "Check/Check_Mainwindow.h"
#include <QDebug>

int main(int argc, char *argv[])
{
    // 确保中文显示正常
    QApplication::setApplicationName("超市收银系统");
    QApplication::setOrganizationName("YourCompany");

    // 初始化Qt应用
    QApplication a(argc, argv);

    // 创建并显示主窗口
    Check_Mainwindow mainWindow;


    // 显示主窗口
    mainWindow.show();

    // 进入应用主循环
    return a.exec();
}
