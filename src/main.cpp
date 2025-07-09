#include "LogIn/LoginDialog.h"
#include "LogIn/dbmanager.h"
#include"LogIn/registerdialog.h"
#include "Product/Product.h"

#include <QApplication>
#include <QMessageBox>

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

    // 连接登录成功信号到商品管理页的显示
    QObject::connect(&loginDialog, &LoginDialog::switch_to_productManage,
                     [&](const QString &userName) {
        productPage=new Product(userName);
        loginDialog.hide();
        productPage->show();
    });

    // 连接商品管理页的返回信号到登录页的显示
    QObject::connect(productPage, &Product::backToLogin, [&]() {
        productPage->hide();
        loginDialog.show();
    });


    // 连接登录对话框的信号
    QObject::connect(&loginDialog, &LoginDialog::switch_to_register, [&]() {
        loginDialog.hide();
        registerDialog.show();
    });

    QObject::connect(&loginDialog, &LoginDialog::switch_to_cashier, [&](QString username) {
        // 这里可以打开收银界面
        qDebug() << "收银员登录成功:" << username;
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

    // 显示登录对话框
    loginDialog.show();
    return a.exec();
}
