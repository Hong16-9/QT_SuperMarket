#include "LogIn/LoginDialog.h"
#include "LogIn/dbmanager.h"
#include "Product/Product.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 初始化数据库
    if (!DBManager::instance().initialize()) {
        QMessageBox::critical(nullptr, "数据库错误",
                              "无法初始化数据库:\n" + DBManager::instance().lastError());
        return 1;
    }

    Product c;
    c.show();
    LoginDialog w;
    w.show();
    return a.exec();
}
