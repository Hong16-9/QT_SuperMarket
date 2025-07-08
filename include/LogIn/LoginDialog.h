#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QMessageBox>
#include <QString>  // 确保包含QString头文件

QT_BEGIN_NAMESPACE
namespace Ui { class LoginDialog; }
QT_END_NAMESPACE

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();

private slots:
    void on_Login_clicked();

private:
    void Into(QString UserName, QString Role); // 登陆成功后跳转的函数

signals:
    void switch_to_productManage(QString UserName); // 显式声明为void
    void switch_to_cashier(QString UserName);       // 显式声明为void

private:
    Ui::LoginDialog *ui;
};
#endif // LOGINDIALOG_H
