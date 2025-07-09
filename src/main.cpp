
#include "LogIn/dbmanager.h"

#include "Check/Check_Mainwindow.h"


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


    Check_Mainwindow m;
    m.show();
    return a.exec();
}
