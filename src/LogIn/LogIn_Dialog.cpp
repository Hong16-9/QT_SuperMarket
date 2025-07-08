#include "LogIn/LogIn_Dialog.h"
#include "ui_LogIn_Dialog.h"
#include "LogIn/dbmanager.h"

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

void LoginDialog::on_Login_clicked()
{
    QString username = ui->usernameEdit->text().trimmed();
    QString password = ui->passwordEdit->text();

    // 使用DBManager进行身份验证
    QString role = DBManager::instance().authenticateUser(username, password);

    if (!role.isEmpty()) {
        Into(username,role); // 登录成功
    } else {
        ui->errorLabel->setText("用户名或密码错误");
    }
}

void LoginDialog::Into(QString UserName, QString Role){

    if(Role=="admin") emit switch_to_productManage(UserName);
    else if(Role=="cashier") emit switch_to_cashier(UserName);
}
