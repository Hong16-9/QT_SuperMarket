#ifndef LOGIN_DIALOG_H
#define LOGIN_DIALOG_H

#include <QDialog>
#include<QMessageBox>

QT_BEGIN_NAMESPACE
namespace Ui {
class LoginDialog;
}
QT_END_NAMESPACE

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
<<<<<<< HEAD
    LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();

private slots:
    void on_Login_clicked();

private:
    void Into(QString UserName,QString Role); //登陆成功后跳转的函数，传参UserName是唯一标识

signals:
    switch_to_productManage(QString UserName);//界面转换槽函数
    switch_to_cashier(QString UserName);


private:
    Ui::LoginDialog *ui;
};
#endif // LOGIN_DIALOG_H
