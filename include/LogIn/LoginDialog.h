#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QMessageBox>
#include <QString>  // 确保包含QString头文件
#include <QObject>
#include <QEvent>

QT_BEGIN_NAMESPACE
namespace Ui { class LoginDialog; }
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
    void on_Register_clicked();

    void on_codeshow_pressed();

    void on_codeshow_released();
\
private:
    void Into(QString UserName, QString Role); // 登陆成功后跳转的函数
protected:

    bool eventFilter(QObject *obj, QEvent *event) override;

signals:
    void switch_to_productManage(QString UserName);
    void switch_to_cashier(QString UserName);
    void switch_to_register();

private:
    Ui::LoginDialog *ui;


};
#endif // LOGINDIALOG_H
