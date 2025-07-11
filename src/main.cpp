#include <QApplication>
<<<<<<< HEAD
#include "Check/Check_Mainwindow.h"
#include <QDebug>
=======
#include <QMessageBox>
#include <QTimer>
>>>>>>> e0b06f548074cbd0aa3868300a6aa55c0e8b8873

int main(int argc, char *argv[])
{
    // 确保中文显示正常
    QApplication::setApplicationName("超市收银系统");
    QApplication::setOrganizationName("YourCompany");

    // 初始化Qt应用
    QApplication a(argc, argv);

    // 创建并显示主窗口
    Check_Mainwindow w("admin",nullptr);


    // 显示主窗口
    w.show();

<<<<<<< HEAD
    // 进入应用主循环
=======
    QObject::connect(&loginDialog, &LoginDialog::switch_to_cashier, [&](QString username) {
        // 这里可以打开收银界面
        qDebug() << "收银员登录成功:" << username;
        checkwindow=new Check_Mainwindow(username);
        loginDialog.hide();
        checkwindow->show();
        // CashierWindow cashierWindow(username);
        // cashierWindow.show();
    });

    // 连接注册对话框的信号
    QObject::connect(&registerDialog, &RegisterDialog::switch_back, [&]() {
        registerDialog.hide();
        loginDialog.show();
    });

    QObject::connect(&registerDialog, &RegisterDialog::registrationSuccess, [&](QString username) {
        // 注册成功后自动填充登录界面的用户名
        loginDialog.setUsername(username);
        registerDialog.hide();
        loginDialog.show();
    });



    // 创建月销量重置定时器
    QTimer *monthlyResetTimer = new QTimer(&a);
    QObject::connect(monthlyResetTimer, &QTimer::timeout, []() {
        QDate currentDate = QDate::currentDate();
        if (currentDate.day() == 1) { // 每月1号重置
            DBManager::instance().resetmonthlysale();
        }
    });

    // 每天检查一次
    monthlyResetTimer->start(24 * 60 * 60 * 1000);


    // 显示登录对话框
    loginDialog.show();
>>>>>>> e0b06f548074cbd0aa3868300a6aa55c0e8b8873
    return a.exec();
}
