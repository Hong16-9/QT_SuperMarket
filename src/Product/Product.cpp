#include "Product/Product.h"
#include "ui_Product.h"

Product::Product(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Product)
{
    ui->setupUi(this);
}

Product::~Product()
{
    delete ui;
}
