#include "Product/SalesStatisticsDialog.h"
#include "LogIn/dbmanager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QTextStream>
#include <QHeaderView>
#include<QMessageBox>

SalesStatisticsDialog::SalesStatisticsDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("商品销量统计");
    setMinimumSize(800, 600);

    setupUI();
    loadSalesData();
}

SalesStatisticsDialog::~SalesStatisticsDialog()
{
}

void SalesStatisticsDialog::setupUI()
{
    // 创建模型和视图
    salesModel = new QStandardItemModel(0, 4, this);
    salesModel->setHorizontalHeaderLabels({"商品ID", "商品名称", "销量", "销售额"});

    tableView = new QTableView(this);
    tableView->setModel(salesModel);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    tableView->setSortingEnabled(true);
    tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    tableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    tableView->verticalHeader()->setVisible(false);

    // 过滤控件
//    QGroupBox *filterGroup = new QGroupBox("统计时段", this);
//    QHBoxLayout *filterLayout = new QHBoxLayout(filterGroup);

//    periodCombo = new QComboBox(this);
//    periodCombo->addItem("当月", "month");
//    periodCombo->addItem("当季", "quarter");
//    periodCombo->addItem("当年", "year");
//    periodCombo->addItem("自定义", "custom");

//    startDateEdit = new QDateEdit(this);
//    startDateEdit->setDate(QDate::currentDate().addMonths(-1));
//    startDateEdit->setCalendarPopup(true);
//    startDateEdit->setDisplayFormat("yyyy-MM-dd");

//    endDateEdit = new QDateEdit(this);
//    endDateEdit->setDate(QDate::currentDate());
//    endDateEdit->setCalendarPopup(true);
//    endDateEdit->setDisplayFormat("yyyy-MM-dd");

//    QLabel *toLabel = new QLabel("至", this);

//    filterLayout->addWidget(new QLabel("时段:", this));
//    filterLayout->addWidget(periodCombo);
//    filterLayout->addSpacing(20);
//    filterLayout->addWidget(new QLabel("开始日期:", this));
//    filterLayout->addWidget(startDateEdit);
//    filterLayout->addWidget(toLabel);
//    filterLayout->addWidget(endDateEdit);
//    filterLayout->addStretch();

    // 按钮
    QPushButton *refreshBtn = new QPushButton("刷新", this);
    QPushButton *exportBtn = new QPushButton("导出CSV", this);

    connect(refreshBtn, &QPushButton::clicked, this, &SalesStatisticsDialog::onRefreshClicked);
    connect(exportBtn, &QPushButton::clicked, this, &SalesStatisticsDialog::onExportClicked);

    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
//    mainLayout->addWidget(filterGroup);
    mainLayout->addWidget(tableView);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();
    buttonLayout->addWidget(refreshBtn);
    buttonLayout->addWidget(exportBtn);

    mainLayout->addLayout(buttonLayout);
}

void SalesStatisticsDialog::loadSalesData()
{
    salesModel->removeRows(0, salesModel->rowCount());

    // 获取销量数据（这里简化实现，实际应根据过滤条件查询）
    QList<QMap<QString, QVariant>> salesData = DBManager::instance().getMonthlyProductSales();

    double totalSales = 0;
    double totalRevenue = 0;

    for (const auto& sale : salesData) {
        int id = sale["id"].toInt();
        QString name = sale["name"].toString();
        int quantity = sale["total_sales"].toInt();
        double revenue = quantity * DBManager::instance().getProductById(id)["price"].toDouble();

        QList<QStandardItem*> items;
        items.append(new QStandardItem(QString::number(id)));
        items.append(new QStandardItem(name));
        items.append(new QStandardItem(QString::number(quantity)));
        items.append(new QStandardItem(QString::number(revenue, 'f', 2)));

        // 设置单元格不可编辑
        foreach (QStandardItem* item, items) {
            item->setEditable(false);
            item->setTextAlignment(Qt::AlignCenter);
        }

        // 高亮显示销量前3的商品
        if (salesModel->rowCount() < 3) {
            foreach (QStandardItem* item, items) {
                item->setBackground(QColor(255, 255, 200)); // 浅黄色背景
            }
        }

        salesModel->appendRow(items);

        totalSales += quantity;
        totalRevenue += revenue;
    }

    // 添加汇总行
    QList<QStandardItem*> totalItems;
    totalItems.append(new QStandardItem("汇总"));
    totalItems.append(new QStandardItem(""));
    totalItems.append(new QStandardItem(QString::number(totalSales)));
    totalItems.append(new QStandardItem(QString::number(totalRevenue, 'f', 2)));

    foreach (QStandardItem* item, totalItems) {
        item->setEditable(false);
        item->setBackground(QColor(230, 230, 250)); // 浅紫色背景
        item->setTextAlignment(Qt::AlignCenter);
        QFont font = item->font();
        font.setBold(true);
        item->setFont(font);
    }

    salesModel->appendRow(totalItems);
}

void SalesStatisticsDialog::onRefreshClicked()
{
    loadSalesData();
}

void SalesStatisticsDialog::onExportClicked()
{
    QString filePath = QFileDialog::getSaveFileName(
        this, "导出销售数据", "", "CSV文件 (*.csv);;所有文件 (*)"
    );

    if (filePath.isEmpty()) return;

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "错误", "无法创建文件");
        return;
    }

    QTextStream out(&file);
    out << "商品ID,商品名称,销量,销售额\n";

    for (int row = 0; row < salesModel->rowCount(); ++row) {
        QStringList rowData;
        for (int col = 0; col < salesModel->columnCount(); ++col) {
            rowData << salesModel->item(row, col)->text();
        }
        out << rowData.join(",") << "\n";
    }

    file.close();
    QMessageBox::information(this, "导出完成", "销售数据已成功导出");
}
