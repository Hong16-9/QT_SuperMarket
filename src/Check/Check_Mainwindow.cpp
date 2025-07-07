#include "Check/Check_Mainwindow.h"
#include "ui_Check_Mainwindow.h"

Check_Mainwindow::Check_Mainwindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Check_Mainwindow)
{
    ui->setupUi(this);
    setWindowTitle("超市收银系统");
}

Check_Mainwindow::~Check_Mainwindow()
{
    delete ui;
}


void Check_Mainwindow::updateProduct(const QString* category){
    ui->chooselistWidget->clear();
    std::vector<Product> products = DatabaseManager::instance().getProductsByCategory(category);




}

void Check_Mainwindow::categoryProduct(){
    QPushButton* senderButton=qobject_cast<QPushButton*>(sender());
    if(!senderButton) return;
    updateProduct(senderButton->text());
}

void Check_Mainwindow::on_chooselistWidget_itemDoubleClicked(QListWidgetItem *item)
{
    if(!item) return;

}

