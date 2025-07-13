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

    QList<QMap<QString, QVariant>> salesData = DBManager::instance().getMonthlyProductSales();

    // 使用列表存储所有销售数据用于排序
    QList<QPair<int, double>> salesList; // <rowIndex, salesValue>

    // 添加以下两行声明总销量和总销售额变量
    double totalSales = 0.0;
    double totalRevenue = 0.0;

    // 第一部分：处理商品数据
    for (int i = 0; i < salesData.size(); ++i) {
        const auto& sale = salesData[i];
        int id = sale["id"].toInt();
        QString name = sale["name"].toString();
        int quantity = sale["total_sales"].toInt();
        double revenue = quantity * DBManager::instance().getProductById(id)["price"].toDouble();

        // 累加总销量和总销售额
        totalSales += quantity;
        totalRevenue += revenue;

        QList<QStandardItem*> items;

        // 商品ID - 存储为数值
        QStandardItem* idItem = new QStandardItem();
        idItem->setData(id, Qt::DisplayRole);
        items.append(idItem);

        // 商品名称 - 字符串
        items.append(new QStandardItem(name));

        // 销量 - 存储为数值
        QStandardItem* quantityItem = new QStandardItem();
        quantityItem->setData(quantity, Qt::DisplayRole);
        items.append(quantityItem);

        // 销售额 - 存储为数值
        QStandardItem* revenueItem = new QStandardItem();
        revenueItem->setData(revenue, Qt::DisplayRole);
        items.append(revenueItem);

        // 设置单元格属性
        foreach (QStandardItem* item, items) {
            item->setEditable(false);
            item->setTextAlignment(Qt::AlignCenter);
        }

        salesModel->appendRow(items);

        // 保存销量值用于后续高亮处理
        salesList.append(qMakePair(i, static_cast<double>(quantity)));
    }

    // 按销量排序（降序）
    std::sort(salesList.begin(), salesList.end(),
        [](const QPair<int, double>& a, const QPair<int, double>& b) {
            return a.second > b.second;
        });
    // 高亮销量前三的商品
    for (int rank = 0; rank < qMin(3, salesList.size()); ++rank) {
        int row = salesList[rank].first;
        for (int col = 0; col < salesModel->columnCount(); ++col) {
            QStandardItem* item = salesModel->item(row, col);
            item->setBackground(QColor(255, 255, 200)); // 浅黄色背景
        }
    }

    // 第二部分：添加汇总行（数值存储）
    QList<QStandardItem*> totalItems;

    // 第一列：文本
    QStandardItem* totalLabel = new QStandardItem("汇总");
    totalLabel->setEditable(false);
    totalLabel->setTextAlignment(Qt::AlignCenter);
    totalLabel->setBackground(QColor(230, 230, 250));
    QFont font = totalLabel->font();
    font.setBold(true);
    totalLabel->setFont(font);
    totalItems.append(totalLabel);

    // 第二列：空
    QStandardItem* emptyItem = new QStandardItem();
    emptyItem->setEditable(false);
    emptyItem->setBackground(QColor(230, 230, 250));
    totalItems.append(emptyItem);

    // 第三列：总销量（数值）
    QStandardItem* totalSalesItem = new QStandardItem();
    totalSalesItem->setData(totalSales, Qt::DisplayRole);  // 现在totalSales已定义
    totalSalesItem->setEditable(false);
    totalSalesItem->setTextAlignment(Qt::AlignCenter);
    totalSalesItem->setBackground(QColor(230, 230, 250));
    totalSalesItem->setFont(font);
    totalItems.append(totalSalesItem);

    // 第四列：总销售额（数值）
    QStandardItem* totalRevenueItem = new QStandardItem();
    totalRevenueItem->setData(totalRevenue, Qt::DisplayRole);  // 现在totalRevenue已定义
    totalRevenueItem->setEditable(false);
    totalRevenueItem->setTextAlignment(Qt::AlignCenter);
    totalRevenueItem->setBackground(QColor(230, 230, 250));
    totalRevenueItem->setFont(font);
    totalItems.append(totalRevenueItem);

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
            QStandardItem* item = salesModel->item(row, col);

            // 特殊处理数值列
            if (col == 0 || col == 2 || col == 3) { // ID, 销量, 销售额列
                QVariant data = item->data(Qt::DisplayRole);

                // 检查是否是数值类型
                if (data.type() == QVariant::Int || data.type() == QVariant::Double) {
                    rowData << data.toString();
                } else {
                    rowData << item->text();
                }
            } else {
                rowData << item->text();
            }
        }
        out << rowData.join(",") << "\n";
    }

    file.close();
    QMessageBox::information(this, "导出完成", "销售数据已成功导出");
}
