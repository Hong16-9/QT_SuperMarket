#include "LogIn/LogIn_Dialog.h"
#include "ui_LogIn_Dialog.h"
#include "LogIn/dbmanager.h"
#include "LogIn/RegisterDialog.h"  // 新增注册对话框头文件

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoginDialog)
{
    ui->setupUi(this);

    // 隐藏错误标签直到需要显示
    ui->errorLabel->hide();

    // 设置密码输入框为密码模式
    ui->passwordEdit->setEchoMode(QLineEdit::Password);
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

void LoginDialog::on_Login_clicked()
{
    QString username = ui->usernameEdit->text().trimmed();
    QString password = ui->passwordEdit->text();

    if (username.isEmpty() || password.isEmpty()) {
        ui->errorLabel->setText("用户名和密码不能为空");
        ui->errorLabel->show();
        return;
    }

    // 使用DBManager进行身份验证
    QString role = DBManager::instance().authenticateUser(username, password);

    if (!role.isEmpty()) {
        ui->errorLabel->hide();
        Into(username, role); // 登录成功
    } else {
        ui->errorLabel->setText("用户名或密码错误");
        ui->errorLabel->show();
    }
}

void LoginDialog::on_Register_clicked()
{
    // 发出跳转到注册界面的信号
    emit switch_to_register();
}

void LoginDialog::Into(QString UserName, QString Role){
    if(Role=="admin") emit switch_to_productManage(UserName);
    else if(Role=="cashier") emit switch_to_cashier(UserName);
}

void LoginDialog::setUsername(const QString &username)
{
    ui->usernameEdit->setText(username);
    // 可选：将焦点设置到密码输入框
    ui->passwordEdit->setFocus();
}
