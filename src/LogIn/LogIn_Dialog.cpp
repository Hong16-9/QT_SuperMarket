#include "LogIn/LogIn_Dialog.h"
#include "ui_LogIn_Dialog.h"

LogIn_Dialog::LogIn_Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LogIn_Dialog)
{
    ui->setupUi(this);
}

LogIn_Dialog::~LogIn_Dialog()
{
    delete ui;
}
