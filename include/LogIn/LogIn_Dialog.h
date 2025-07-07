#ifndef LOGIN_DIALOG_H
#define LOGIN_DIALOG_H

#include <QDialog>
#include<QMessageBox>

QT_BEGIN_NAMESPACE
namespace Ui {
class LogIn_Dialog;
}
QT_END_NAMESPACE

class LogIn_Dialog : public QDialog
{
    Q_OBJECT

public:
    LogIn_Dialog(QWidget *parent = nullptr);
    ~LogIn_Dialog();

private:
    Ui::LogIn_Dialog *ui;
};
#endif // LOGIN_DIALOG_H
