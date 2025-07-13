#include "Product/Product.h"
#include "qtimer.h"
#include "ui_Product.h"
#include "Product/AddProductDialog.h"
#include "Product/StockDialog.h"
#include "Product/SalesStatisticsDialog.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QHeaderView>
#include <QStandardItem>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>

Product::Product(QString name, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Product),
    dbManager(&DBManager::instance()),
    name(name)
{
    ui->setupUi(this);
    initUI();
    setupConnections();
    loadProducts();
    setWindowTitle(QString("商品管理 - 当前用户: %1").arg(name));

    // 修正后的样式表（确保语法正确，无多余空格或缺失符号）
    setStyleSheet(R"(
        QMainWindow {
            background-color: #f0f0f0;
        }
        QPushButton {
            background-color: #4CAF50;
            color: white;
            padding: 5px 10px;
            border: none;
            border-radius: 3px;
        }
        QPushButton:hover {
            background-color: #45a049;
        }
        QTableView {
            background-color: white;
            border: 1px solid #ccc;
        }
        QHeaderView::section {
            background-color: #e0e0e0;
            padding: 5px;
            border: 1px solid #ccc;
        }
    )");
}

Product::~Product()
{
    delete ui;
}

void Product::initUI()
{
    setWindowTitle("商品管理");

    // 创建中央部件和主布局
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // 主垂直布局
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);

    // 搜索区域布局
    QHBoxLayout *searchLayout = new QHBoxLayout();
    searchLayout->setSpacing(5);

    // 关联UI控件
    searchLineEdit = ui->searchLineEdit;
    categoryComboBox = ui->categoryComboBox;

    // 设置搜索框和下拉框的最小宽度
    searchLineEdit->setMinimumWidth(200);
    categoryComboBox->setMinimumWidth(120);

    // 添加搜索区域控件到布局
    searchLayout->addWidget(new QLabel("分类:", this));
    searchLayout->addWidget(categoryComboBox);
    searchLayout->addWidget(new QLabel("搜索:", this));
    searchLayout->addWidget(searchLineEdit);
    searchLayout->addWidget(ui->Search_Product);
    searchLayout->addStretch(); // 让搜索区域控件靠左排列，右侧填充空白

    // 初始化表格模型
    productModel = new QStandardItemModel(0, 6, this);
    productModel->setHorizontalHeaderLabels({"ID", "商品名称", "条码", "价格", "库存", "分类"});

    // 表格视图设置
    ui->productTableView->setModel(productModel);
    ui->productTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->productTableView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->productTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->productTableView->setSortingEnabled(true);

    // 关键表格布局设置
    ui->productTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->productTableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->productTableView->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui->productTableView->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    ui->productTableView->setAlternatingRowColors(true);

    // 设置表格的拉伸策略
    ui->productTableView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // 将搜索区域和表格添加到主布局
    mainLayout->addLayout(searchLayout);
    mainLayout->addWidget(ui->productTableView, 1); // 表格占据主要空间

    // 状态栏设置
    QLabel* adminLabel = new QLabel(QString("当前管理员：%1").arg(name), this);
    adminLabel->setMargin(5);

    QLabel* dateLabel = new QLabel(this);
    QDate currentDate = QDate::currentDate();
    const QStringList weekDays = {"", "一", "二", "三", "四", "五", "六", "日"};
    int weekNum = currentDate.dayOfWeek();
    QString weekStr = weekDays[weekNum];

    QString dateText = QString("当前日期：%1 星期%2")
                           .arg(currentDate.toString("yyyy-MM-dd"))
                           .arg(weekStr);
    dateLabel->setText(dateText);
    dateLabel->setMargin(5);

    statusBar()->addWidget(adminLabel);
    statusBar()->addPermanentWidget(dateLabel);
}

// 连接信号与槽（关联Action和控件事件）
void Product::setupConnections()
{
    // 工具栏中的Action（UI中Action对象名需与以下一致）
    connect(ui->Add_Product, &QAction::triggered, this, &Product::onAddProductClicked);
    connect(ui->Delete_Product, &QAction::triggered, this, &Product::onDeleteProductClicked);
    connect(ui->Put_Product, &QAction::triggered, this, &Product::onPutProductClicked);
    connect(ui->Out_Product, &QAction::triggered, this, &Product::onTakeProductClicked);
    connect(ui->importAction, &QAction::triggered, this, &Product::onImportDataClicked);
    connect(ui->exportAction, &QAction::triggered, this, &Product::onExportDataClicked);

    // 外部搜索按钮（非Action，UI中按钮对象名为Search_Product）
    connect(ui->Search_Product, &QPushButton::clicked, this, &Product::onSearchProductClicked);
    // 文本框回车触发搜索
    connect(searchLineEdit, &QLineEdit::returnPressed, this, &Product::onSearchProductClicked);

    connect(ui->backToLogin, &QAction::triggered, this, &Product::onBackToLoginTriggered);

    // 新增：连接查看销售统计的Action
    connect(ui->Sale_Action, &QAction::triggered, this, &Product::onViewSalesStatisticsClicked);
}

// 加载商品数据
void Product::loadProducts()
{
    productModel->removeRows(0, productModel->rowCount());

    // 从数据库获取所有商品
    QList<QMap<QString, QVariant>> products = dbManager->getAllProducts();

    QList<QMap<QString, QVariant>> lowStockProducts; // 存储库存不足的商品

    for (const auto& product : products) {
        QList<QStandardItem*> items;
        items.append(new QStandardItem(product["id"].toString()));
        items.append(new QStandardItem(product["name"].toString()));
        items.append(new QStandardItem(product["barcode"].toString()));
        items.append(new QStandardItem(QString::number(product["price"].toDouble(), 'f', 2)));

        // 库存不足时标红
        QStandardItem* stockItem = new QStandardItem(product["stock"].toString());
        if (product["stock"].toInt() < 20) {
            stockItem->setForeground(Qt::red);
            lowStockProducts.append(product); // 添加到库存不足的商品列表
        }
        items.append(stockItem);

        items.append(new QStandardItem(product["category"].toString()));

        // 设置单元格不可编辑
        foreach (QStandardItem* item, items) {
            item->setEditable(false);
        }

        productModel->appendRow(items);
    }

    updateCategoryComboBox();

    // 关键修改：使用QTimer延迟提示框，确保页面先显示
    // 0毫秒延迟表示"当前事件循环结束后执行"，此时页面已完成绘制
    QTimer::singleShot(0, this, [this, lowStockProducts]() {
        showLowStockProducts(lowStockProducts);
    });
}

// 更新分类下拉框
void Product::updateCategoryComboBox()
{
    QString currentCategory = categoryComboBox->currentText();
    categoryComboBox->clear();
    categoryComboBox->addItem("全部");

    QSet<QString> categories;
    for (int i = 0; i < productModel->rowCount(); ++i) {
        categories.insert(productModel->item(i, 5)->text());
    }

    categoryComboBox->addItems(categories.values());

    // 恢复之前的选择
    int index = categoryComboBox->findText(currentCategory);
    if (index != -1) {
        categoryComboBox->setCurrentIndex(index);
    }
}

// 添加商品
void Product::onAddProductClicked()
{
    AddProductDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        QString name = dialog.getName();
        QString barcode = dialog.getBarcode();
        double price = dialog.getPrice();
        int stock = dialog.getStock();
        QString category = dialog.getCategory();

        if (dbManager->addProduct(name, barcode, price, stock, category)) {
            QMessageBox::information(this, "成功", "商品添加成功！");
            loadProducts();
        } else {
            QMessageBox::critical(this, "失败", "商品添加失败！可能是条码已存在。");
        }
    }
}

// 删除商品
void Product::onDeleteProductClicked()
{
    QModelIndexList selectedRows = ui->productTableView->selectionModel()->selectedRows();
    if (selectedRows.isEmpty()) {
        QMessageBox::warning(this, "警告", "请先选择要删除的商品！");
        return;
    }

    if (QMessageBox::question(this, "确认删除",
                              QString("确定要删除选中的 %1 个商品吗？").arg(selectedRows.size()),
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) {
        return;
    }

    // 按行号降序删除，避免索引错乱
    QList<int> rowsToDelete;
    foreach (const QModelIndex &index, selectedRows) {
        rowsToDelete.append(index.row());
    }
    std::sort(rowsToDelete.begin(), rowsToDelete.end(), std::greater<int>());

    // 执行删除
    foreach (int row, rowsToDelete) {
        int productId = productModel->item(row, 0)->text().toInt();
        if (dbManager->deleteProduct(productId)) {
            productModel->removeRow(row);
        } else {
            QMessageBox::critical(this, "失败", "删除商品失败！");
        }
    }

    updateCategoryComboBox();
}

// 商品入库
void Product::onPutProductClicked()
{
    // 获取表格中选中的行
    QModelIndexList selectedRows = ui->productTableView->selectionModel()->selectedRows();

    // 校验选中状态
    if (selectedRows.isEmpty()) {
        QMessageBox::warning(this, "警告", "请先选择要入库的商品！");
        return;
    }
    if (selectedRows.size() > 1) {
        QMessageBox::warning(this, "警告", "一次只能对一个商品进行入库操作！");
        return;
    }

    // 获取选中商品的信息（从表格中提取）
    QModelIndex index = selectedRows.first(); // 获取第一行选中项
    int row = index.row();                    // 选中行的行号

    // 从表格模型中提取商品ID、名称、当前库存
    int productId = productModel->item(row, 0)->text().toInt();       // ID在第0列
    QString productName = productModel->item(row, 1)->text();          // 名称在第1列
    int currentStock = productModel->item(row, 4)->text().toInt();     // 库存在第4列

    StockDialog dialog(StockOperation::In, productName, currentStock, this);
    if (dialog.exec() == QDialog::Accepted) {
        int amount = dialog.getChangeAmount();
        // 修正：调用正确的函数名
        if (dbManager->updateProductStock(productId, amount)) {
            QMessageBox::information(this, "成功", "商品入库成功！");
            loadProducts();
        } else {
            QMessageBox::critical(this, "失败", "商品入库失败！");
        }
    }
}

// 商品出库
void Product::onTakeProductClicked()
{
    QModelIndexList selectedRows = ui->productTableView->selectionModel()->selectedRows();
    if (selectedRows.isEmpty()) {
        QMessageBox::warning(this, "警告", "请先选择要出库的商品！");
        return;
    }
    if (selectedRows.size() > 1) {
        QMessageBox::warning(this, "警告", "一次只能对一个商品进行出库操作！");
        return;
    }

    QModelIndex index = selectedRows.first();
    int row = index.row();
    int productId = productModel->item(row, 0)->text().toInt();
    QString productName = productModel->item(row, 1)->text();
    int currentStock = productModel->item(row, 4)->text().toInt();

    StockDialog dialog(StockOperation::Out, productName, currentStock, this);
    if (dialog.exec() == QDialog::Accepted) {
        int amount = dialog.getChangeAmount();
        if (dbManager->updateProductStock(productId, -amount)) {
            QMessageBox::information(this, "成功", "商品出库成功！");
            loadProducts();
        } else {
            QMessageBox::critical(this, "失败", "商品出库失败！");
        }
    }
}

// 在 Product.cpp 中实现数据导出函数
void Product::onExportDataClicked()
{
    QString filePath = QFileDialog::getSaveFileName(this, "导出商品数据", "", "CSV文件 (*.csv);;所有文件 (*)");
    if (filePath.isEmpty()) return;

    if (exportDataToCSV(filePath)) {
        QMessageBox::information(this, "导出成功", "商品数据已成功导出");
    } else {
        QMessageBox::critical(this, "导出失败", "商品数据导出失败");
    }
}

bool Product::exportDataToCSV(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream out(&file);
    // 写入表头
    for (int i = 0; i < productModel->columnCount(); ++i) {
        out << productModel->headerData(i, Qt::Horizontal).toString();
        if (i < productModel->columnCount() - 1) {
            out << ",";
        }
    }
    out << "\n";

    // 写入数据
    for (int row = 0; row < productModel->rowCount(); ++row) {
        for (int col = 0; col < productModel->columnCount(); ++col) {
            QStandardItem* item = productModel->item(row, col);
            if (item) {
                out << item->text();
            }
            if (col < productModel->columnCount() - 1) {
                out << ",";
            }
        }
        out << "\n";
    }

    file.close();
    return true;
}

// 在 Product.cpp 中实现数据导入函数
void Product::onImportDataClicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, "导入商品数据", "", "CSV文件 (*.csv);;所有文件 (*)");
    if (filePath.isEmpty()) return;

    if (importDataFromCSV(filePath)) {
        QMessageBox::information(this, "导入成功", "商品数据已成功导入");
        loadProducts(); // 重新加载商品数据
    } else {
        QMessageBox::critical(this, "导入失败", "商品数据导入失败");
    }
}

bool Product::importDataFromCSV(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream in(&file);
    // 跳过表头
    in.readLine();

    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList fields = line.split(",");
        if (fields.size() == 6) {
            QString name = fields[1];
            QString barcode = fields[2];
            double price = fields[3].toDouble();
            int stock = fields[4].toInt();
            QString category = fields[5];

            if (!dbManager->addProduct(name, barcode, price, stock, category)) {
                file.close();
                return false;
            }
        }
    }

    file.close();
    return true;
}

// 搜索商品
void Product::onSearchProductClicked()
{
    QString keyword = searchLineEdit->text().trimmed();
    QString category = categoryComboBox->currentText();

    productModel->removeRows(0, productModel->rowCount());

    QList<QMap<QString, QVariant>> products;
    if (category == "全部") {
        products = dbManager->searchProducts(keyword);
    } else {
        QList<QMap<QString, QVariant>> allProducts = dbManager->searchProducts(keyword);
        for (const auto& product : allProducts) {
            if (product["category"].toString() == category) {
                products.append(product);
            }
        }
    }

    for (const auto& product : products) {
        QList<QStandardItem*> items;
        items.append(new QStandardItem(product["id"].toString()));
        items.append(new QStandardItem(product["name"].toString()));
        items.append(new QStandardItem(product["barcode"].toString()));
        items.append(new QStandardItem(QString::number(product["price"].toDouble(), 'f', 2)));

        QStandardItem* stockItem = new QStandardItem(product["stock"].toString());
        if (product["stock"].toInt() < 20) {
            stockItem->setForeground(Qt::red);
        }
        items.append(stockItem);

        items.append(new QStandardItem(product["category"].toString()));

        foreach (QStandardItem* item, items) {
            item->setEditable(false);
        }

        productModel->appendRow(items);
    }
}

// 返回登录界面
void Product::onBackToLoginTriggered()
{
    emit backToLogin();
}

// 查看销售统计
void Product::onViewSalesStatisticsClicked()
{
    SalesStatisticsDialog dialog(this);
    dialog.exec();
}

// 显示库存不足的商品提示框
void Product::showLowStockProducts(const QList<QMap<QString, QVariant>>& lowStockProducts)
{
    if (!lowStockProducts.isEmpty()) {
        QString message = "以下商品库存不足（低于20件）：\n";
        for (const auto& product : lowStockProducts) {
            message += QString("ID: %1, 名称: %2, 库存: %3\n")
                           .arg(product["id"].toString())
                           .arg(product["name"].toString())
                           .arg(product["stock"].toString());
        }
        QMessageBox::warning(this, "库存不足警告", message);
    }
}
