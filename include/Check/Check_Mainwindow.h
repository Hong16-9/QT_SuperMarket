#ifndef CHECK_MAINWINDOW_H
#define CHECK_MAINWINDOW_H

#include <QMainWindow>
#include<QSqlDatabase>
#include<QSqlQuery>


QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
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

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


private slots:
    void on_chooselistWidget_itemDoubleClicked(QListWidgetItem *item);

private:
    Ui::MainWindow *ui;
    void setupDatabase();
    void updateProduct(const QString* category);
    void categoryProduct();
};

#endif // CHECK_MAINWINDOW_H
