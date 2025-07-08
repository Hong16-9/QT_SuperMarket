#ifndef LOGIN_DIALOG_H
#define LOGIN_DIALOG_H

#include <QDialog>
#include <QMessageBox>

QT_BEGIN_NAMESPACE
namespace Ui {
class LoginDialog;
}
QT_END_NAMESPACE

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();

    void setUsername(const QString &username);

private slots:
    void on_Login_clicked();
    void on_Register_clicked();  // 新增注册按钮槽函数

private:
    void Into(QString UserName,QString Role); //登陆成功后跳转的函数

signals:
    switch_to_productManage(QString UserName);//界面转换槽函数
    switch_to_cashier(QString UserName);
    switch_to_register();  // 新增跳转到注册界面的信号

private:
    Ui::LoginDialog *ui;
};
#endif // LOGIN_DIALOG_H
