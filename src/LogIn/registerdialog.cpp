#include "LogIn/registerdialog.h"
#include "ui_registerdialog.h"
#include <QMessageBox>

RegisterDialog::RegisterDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RegisterDialog)
{
    ui->setupUi(this);

    setWindowTitle("注册");

    // 设置密码输入框为密码模式
    ui->passwordEdit->setEchoMode(QLineEdit::Password);
    ui->confirmPasswordEdit->setEchoMode(QLineEdit::Password);
    ui->adminPasswordEdit->setEchoMode(QLineEdit::Password);

}

RegisterDialog::~RegisterDialog()
{
    delete ui;
}

void RegisterDialog::on_registerButton_clicked()
{
    QString adminUsername = ui->adminUsernameEdit->text().trimmed();
    QString adminPassword = ui->adminPasswordEdit->text();
    QString username = ui->usernameEdit->text().trimmed();
    QString password = ui->passwordEdit->text();
    QString confirmPassword = ui->confirmPasswordEdit->text();
    QString gender = ui->genderCombo->currentText();
    int age = ui->ageSpinBox->value();

    // 验证输入
    if (adminUsername.isEmpty() || adminPassword.isEmpty()) {
        QMessageBox::warning(this, "注册失败", "管理员账号和密码不能为空");
        return;
    }

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "注册失败", "用户名和密码不能为空");
        return;
    }

    if (password != confirmPassword) {
        QMessageBox::warning(this, "注册失败", "两次输入的密码不一致");
        return;
    }

    if (password.length() < 6) {
        QMessageBox::warning(this, "注册失败", "密码长度至少需要6位");
        return;
    }

    // 验证管理员身份
    if (!DBManager::instance().authenticateAdmin(adminUsername, adminPassword)) {
        QMessageBox::warning(this, "注册失败", "管理员认证失败，账号或密码错误");
        return;
    }

    // 尝试注册用户
    if (DBManager::instance().registerUser(username, password,gender,age)) {
        QMessageBox::information(this, "注册成功", "账户创建成功，请登录");
        emit registrationSuccess(username);
        emit switch_back();
    } else {
        QMessageBox::warning(this, "注册失败", "用户名已被使用");
    }
}

void RegisterDialog::on_backButton_clicked()
{
    emit switch_back();
}

void RegisterDialog::on_codeshow_pressed()
{
    ui->adminPasswordEdit->setEchoMode(QLineEdit::Normal);
}

void RegisterDialog::on_codeshow_released()
{
    ui->adminPasswordEdit->setEchoMode(QLineEdit::Password);
}

void RegisterDialog::on_codeshow_2_pressed()
{
    ui->passwordEdit->setEchoMode(QLineEdit::Normal);
}

void RegisterDialog::on_codeshow_2_released()
{
    ui->passwordEdit->setEchoMode(QLineEdit::Password);
}

void RegisterDialog::on_codeshow_3_pressed()
{
    ui->confirmPasswordEdit->setEchoMode(QLineEdit::Normal);
}

void RegisterDialog::on_codeshow_3_released()
{
    ui->confirmPasswordEdit->setEchoMode(QLineEdit::Password);
}
