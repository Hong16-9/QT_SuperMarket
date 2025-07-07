#ifndef CHECK_MAINWINDOW_H
#define CHECK_MAINWINDOW_H

#include <QMainWindow>
#include<QSqlDatabase>
#include<QSqlQuery>
#include<QListWidgetItem>



QT_BEGIN_NAMESPACE
namespace Ui {
class Check_Mainwindow;
}
QT_END_NAMESPACE

//商品类定义
class Product {
public:
    Product() : id(0), price(0), stock(0) {}
    Product(int id, const QString& name, const QString& barcode,
            double price, int stock, const QString& category)
        : id(id), name(name), barcode(barcode), price(price),
        stock(stock), category(category) {}

    int id;
    QString name;
    QString barcode;
    double price;
    int stock;
    QString category;
};

class Check_Mainwindow : public QMainWindow
{
    Q_OBJECT

public:
    Check_Mainwindow(QWidget *parent = nullptr);
    ~Check_Mainwindow();


private slots:
    void on_chooselistWidget_itemDoubleClicked();

private:
    Ui::Check_Mainwindow *ui;
    void setupDatabase();
    void updateProduct(const QString* category);
    void categoryProduct();
};

#endif // CHECK_MAINWINDOW_H
