#include "Product/SalesStatisticsDialog.h"
#include "LogIn/dbmanager.h"
#include <QVBoxLayout>

SalesStatisticsDialog::SalesStatisticsDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("当月商品销量统计");
    setMinimumWidth(600);

    salesModel = new QStandardItemModel(0, 2, this);
    salesModel->setHorizontalHeaderLabels({"商品名称", "当月销量"});

    tableView = new QTableView(this);
    tableView->setModel(salesModel);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(tableView);

    loadMonthlySales();
}

SalesStatisticsDialog::~SalesStatisticsDialog()
{
}

void SalesStatisticsDialog::loadMonthlySales()
{
    salesModel->removeRows(0, salesModel->rowCount());

    QList<QMap<QString, QVariant>> salesData = DBManager::instance().getMonthlyProductSales();

    for (const auto& sale : salesData) {
        QList<QStandardItem*> items;
        items.append(new QStandardItem(sale["name"].toString()));
        items.append(new QStandardItem(sale["total_sales"].toString()));

        // 设置单元格不可编辑
        foreach (QStandardItem* item, items) {
            item->setEditable(false);
        }

        salesModel->appendRow(items);
    }
}
