#include "LogIn/LoginDialog.h"
#include "LogIn/dbmanager.h"
#include "LogIn/registerdialog.h"
#include "Product/Product.h"
#include "Check/Check_Mainwindow.h"

#include <QApplication>
#include <QMessageBox>
#include <QTimer>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    // 初始化数据库
    if (!DBManager::instance().initialize()) {
        QMessageBox::critical(nullptr, "错误", "数据库初始化失败！");
        return -1;
    }

    // 创建登录和注册对话框
    LoginDialog loginDialog;
    RegisterDialog registerDialog;

    // 创建商品管理页
    Product *productPage=nullptr;

    //创建收银界面
    Check_Mainwindow *checkwindow=nullptr;

    // 连接登录成功信号
    QObject::connect(&loginDialog, &LoginDialog::switch_to_productManage,
                     [&](const QString &userName) {
                         if (productPage) {
                             productPage->deleteLater(); // 清理旧窗口
                         }
                         productPage = new Product(userName);

                         // 将返回信号连接移到窗口创建后
                         QObject::connect(productPage, &Product::backToLogin, [&]() {
                             productPage->hide();
                             loginDialog.show();
                         });

                         loginDialog.hide();
                         productPage->show();
                     });

    // 连接登录成功信号
    QObject::connect(&loginDialog, &LoginDialog::switch_to_cashier,
                     [&](const QString &userName) {
                         if (checkwindow) {
                             checkwindow->deleteLater(); // 清理旧窗口
                         }
                         checkwindow = new Check_Mainwindow(userName);

                         // 将返回信号连接移到窗口创建后
                         QObject::connect(checkwindow, &Check_Mainwindow::backToLogin, [&]() {
                             checkwindow->hide();       //隐藏收银窗口
                             loginDialog.show();        //显示原登录窗口
                         });

                         loginDialog.hide();
                         checkwindow->show();
                     });


    // 连接登录对话框的信号
    QObject::connect(&loginDialog, &LoginDialog::switch_to_register, [&]() {
        loginDialog.hide();
        registerDialog.show();
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
    return a.exec();
}
