#include "LogIn/LoginDialog.h"
#include"LogIn/registerdialog.h"
#include "Product/Product.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 创建登录对话框和商品管理页
    LoginDialog loginDialog;
    Product productPage;

    // 连接登录成功信号到商品管理页的显示
    QObject::connect(&loginDialog, &LoginDialog::switch_to_productManage,
                     [&](const QString &userName) {
                         loginDialog.hide();
                         productPage.show();
                         // 可以在这里传递用户名，例如设置窗口标题
                         productPage.setWindowTitle(QString("商品管理 - 当前用户: %1").arg(userName));
                     });

    // 连接商品管理页的返回信号到登录页的显示
    QObject::connect(&productPage, &Product::backToLogin, [&]() {
        productPage.hide();
        loginDialog.show();
    });

    // 显示登录对话框
    loginDialog.show();

    // 创建登录和注册对话框
    LoginDialog loginDialog;
    RegisterDialog registerDialog;


    // 连接登录对话框的信号
    QObject::connect(&loginDialog, &LoginDialog::switch_to_register, [&]() {
        loginDialog.hide();
        registerDialog.show();
    });

    QObject::connect(&loginDialog, &LoginDialog::switch_to_productManage, [&](QString username) {
        // 这里可以打开主管理界面
        Product *productWindow = new Product(username); // 使用指针，避免局部变量被销毁
        productWindow->setAttribute(Qt::WA_DeleteOnClose); // 关闭时自动删除
        loginDialog.hide();
        productWindow->show();
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
