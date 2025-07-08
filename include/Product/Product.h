#ifndef PRODUCT_H
#define PRODUCT_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class Product;
}
QT_END_NAMESPACE

class Product : public QMainWindow
{
    Q_OBJECT

public:
    Product(QWidget *parent = nullptr);
    ~Product();

private:
    Ui::Product *ui;
};
#endif // PRODUCT_H
