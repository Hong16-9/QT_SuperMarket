#include "Check_Mainwindow.h"
#include "./ui_Check_Mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("超市收银系统");
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::updateProduct(const QString* category){
    ui->chooselistWidget->clear();



}

void MainWindow::categoryProduct(){
    QPushButton* senderButton=qobject_cast<QPushButton*>(sender());
    if(!senderButton) return;
    updateProduct(senderButton->text());
}

void MainWindow::on_chooselistWidget_itemDoubleClicked(QListWidgetItem *item)
{
    if(!item) return;

}

