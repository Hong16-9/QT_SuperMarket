#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include <QDialog>
#include "LogIn/dbmanager.h"

namespace Ui {
class RegisterDialog;
}

class RegisterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RegisterDialog(QWidget *parent = nullptr);
    ~RegisterDialog();

signals:
    void registrationSuccess(QString username);
    void switch_back();

private slots:
    void on_registerButton_clicked();
    void on_backButton_clicked();

    void on_codeshow_pressed();

    void on_codeshow_released();

    void on_codeshow_2_pressed();

    void on_codeshow_2_released();

    void on_codeshow_3_pressed();

    void on_codeshow_3_released();

private:
    Ui::RegisterDialog *ui;
};

#endif // REGISTERDIALOG_H
