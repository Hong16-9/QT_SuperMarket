#ifndef PRODUCT_H
#define PRODUCT_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class Mainwindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::Mainwindow *ui;
};
#endif // PRODUCT_H
