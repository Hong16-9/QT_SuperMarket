#include "LogIn/LoginDialog.h"
#include "LogIn/dbmanager.h"
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

    return a.exec();
}
