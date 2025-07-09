#include "Product/Product.h"
#include "ui_Product.h"
#include "Product/AddProductDialog.h"
#include "Product/StockDialog.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QHeaderView>
#include <QStandardItem>
#include <QFile>
#include <QTextStream>
#include <QDebug>

Product::Product(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Product),
    dbManager(&DBManager::instance())

{
    ui->setupUi(this);  // 加载UI Designer设计的界面
    initUI();           // 关联UI控件
    setupConnections(); // 连接信号与槽
    loadProducts();     // 初始加载商品数据
}

Product::~Product()
{
    delete ui;
}

// 初始化UI：关联UI Designer中的控件
void Product::initUI()
{
    setWindowTitle("商品管理");
    setMinimumSize(1024, 600);

    // 初始化表格模型
    productModel = new QStandardItemModel(0, 6, this);
    productModel->setHorizontalHeaderLabels({"ID", "商品名称", "条码", "价格", "库存", "分类"});

    // 关联表格视图（UI中表格对象名为productTableView）
    ui->productTableView->setModel(productModel);
    ui->productTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->productTableView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->productTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->productTableView->setSortingEnabled(true);
    ui->productTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->productTableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->productTableView->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui->productTableView->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    ui->productTableView->setAlternatingRowColors(true);

    // 关联外部的文本框和组合框（UI中对应对象名）
    searchLineEdit = ui->searchLineEdit;       // 文本框对象名保持searchLineEdit
    categoryComboBox = ui->categoryComboBox;   // 组合框对象名保持categoryComboBox
    searchLineEdit->setPlaceholderText("输入商品名称搜索...");
    categoryComboBox->setMinimumWidth(120);
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

}

// 加载商品数据
void Product::loadProducts()
{
    productModel->removeRows(0, productModel->rowCount());

    // 从数据库获取所有商品
    QList<QMap<QString, QVariant>> products = dbManager->getAllProducts();

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

    // 显示入库对话框（指定操作类型为“入库”）
    StockDialog dialog(StockOperation::In, productName, currentStock, this);
    if (dialog.exec() == QDialog::Accepted) {
        // 获取入库数量（通过对话框接口）
        auto stockChanges = dialog.getStockChanges();
        if (stockChanges.isEmpty()) {
            QMessageBox::warning(this, "错误", "未获取到入库信息！");
            return;
        }

        // 调用数据库接口更新库存（入库为正数）
        bool updateSuccess = dbManager->updateProductStock(
            stockChanges.first().first,  // 商品ID
            stockChanges.first().second   // 入库数量（正数）
            );

        if (updateSuccess) {
            QMessageBox::information(this, "成功",
                                     QString("商品入库成功！\n入库数量：%1\n更新后库存：%2")
                                         .arg(stockChanges.first().second)
                                         .arg(currentStock + stockChanges.first().second));
            loadProducts(); // 刷新表格数据
        } else {
            QMessageBox::critical(this, "失败", "商品入库失败，请检查数据库连接！");
        }
    }
}

// 商品出库
void Product::onTakeProductClicked()
{
    // 获取表格中选中的行
    QModelIndexList selectedRows = ui->productTableView->selectionModel()->selectedRows();

    // 校验选中状态
    if (selectedRows.isEmpty()) {
        QMessageBox::warning(this, "警告", "请先选择要出库的商品！");
        return;
    }
    if (selectedRows.size() > 1) {
        QMessageBox::warning(this, "警告", "一次只能对一个商品进行出库操作！");
        return;
    }

    // 获取选中商品的信息（从表格中提取）
    QModelIndex index = selectedRows.first(); // 获取第一行选中项
    int row = index.row();                    // 选中行的行号

    // 从表格模型中提取商品ID、名称、当前库存
    int productId = productModel->item(row, 0)->text().toInt();       // ID在第0列
    QString productName = productModel->item(row, 1)->text();          // 名称在第1列
    int currentStock = productModel->item(row, 4)->text().toInt();     // 库存在第4列

    // 显示出库对话框（指定操作类型为“出库”）
    StockDialog dialog(StockOperation::Out, productName, currentStock, this);
    if (dialog.exec() == QDialog::Accepted) {
        // 获取出库数量（通过对话框接口）
        auto stockChanges = dialog.getStockChanges();
        if (stockChanges.isEmpty()) {
            QMessageBox::warning(this, "错误", "未获取到出库信息！");
            return;
        }

        // 调用数据库接口更新库存（出库为负数，直接使用对话框返回的负值）
        bool updateSuccess = dbManager->updateProductStock(
            stockChanges.first().first,  // 商品ID
            stockChanges.first().second   // 出库数量（负数，由对话框返回）
            );

        if (updateSuccess) {
            QMessageBox::information(this, "成功",
                                     QString("商品出库成功！\n出库数量：%1\n更新后库存：%2")
                                         .arg(-stockChanges.first().second)  // 负数转正数显示
                                         .arg(currentStock + stockChanges.first().second)); // 库存减少
            loadProducts(); // 刷新表格数据
        } else {
            QMessageBox::critical(this, "失败", "商品出库失败，请检查数据库连接！");
        }
    }
}

// 搜索商品
void Product::onSearchProductClicked()
{
    QString keyword = searchLineEdit->text().trimmed();
    QString category = categoryComboBox->currentText();

    if (keyword.isEmpty() && category == "全部") {
        loadProducts(); // 无搜索条件时加载全部
        return;
    }

    searchProducts(keyword, category);
}

// 搜索商品实现
void Product::searchProducts(const QString &keyword, const QString &category)
{
    productModel->removeRows(0, productModel->rowCount());
    QList<QMap<QString, QVariant>> products;

    if (category == "全部") {
        // 按名称搜索（无关键词则加载全部）
        products = keyword.isEmpty() ?
                       dbManager->getAllProducts() :
                       dbManager->getProductsByName(keyword);
    } else {
        // 先按分类筛选，再按名称筛选
        QList<QMap<QString, QVariant>> categoryProducts = dbManager->getProductsByCategory(category);
        if (keyword.isEmpty()) {
            products = categoryProducts;
        } else {
            foreach (const auto& product, categoryProducts) {
                if (product["name"].toString().contains(keyword, Qt::CaseInsensitive)) {
                    products.append(product);
                }
            }
        }
    }

    // 填充表格
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

// 导入数据
void Product::onImportDataClicked()
{
    QString filePath = QFileDialog::getOpenFileName(
        this, "选择导入文件", "", "CSV文件 (*.csv);;所有文件 (*)"
        );

    if (filePath.isEmpty()) return;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "错误", "无法打开文件！");
        return;
    }

    QTextStream in(&file);
    QString header = in.readLine(); // 跳过标题行

    int successCount = 0;
    int failCount = 0;

    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList fields = line.split(',');

        if (fields.size() >= 5) {
            QString name = fields[0].trimmed();
            QString barcode = fields[1].trimmed();
            double price = fields[2].trimmed().toDouble();
            int stock = fields[3].trimmed().toInt();
            QString category = fields[4].trimmed();

            if (dbManager->addProduct(name, barcode, price, stock, category)) {
                successCount++;
            } else {
                failCount++;
            }
        } else {
            failCount++;
        }
    }

    file.close();
    QMessageBox::information(this, "导入完成",
                             QString("成功导入 %1 条，失败 %2 条。").arg(successCount).arg(failCount));
    loadProducts();
}

// 导出数据
void Product::onExportDataClicked()
{
    QString filePath = QFileDialog::getSaveFileName(
        this, "选择导出位置", "", "CSV文件 (*.csv);;所有文件 (*)"
        );

    if (filePath.isEmpty()) return;

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "错误", "无法创建文件！");
        return;
    }

    QTextStream out(&file);
    out << "商品名称,条码,价格,库存,分类\n";

    for (int i = 0; i < productModel->rowCount(); ++i) {
        QString name = productModel->item(i, 1)->text();
        QString barcode = productModel->item(i, 2)->text();
        QString price = productModel->item(i, 3)->text();
        QString stock = productModel->item(i, 4)->text();
        QString category = productModel->item(i, 5)->text();

        // 处理含逗号的字段
        if (name.contains(',')) name = "\"" + name + "\"";
        if (category.contains(',')) category = "\"" + category + "\"";

        out << name << "," << barcode << "," << price << "," << stock << "," << category << "\n";
    }

    file.close();
    QMessageBox::information(this, "导出完成", "商品数据已成功导出！");
}


void Product::onBackToLoginTriggered()
{
    emit backToLogin(); // 发送返回登录的信号
    this->close();      // 关闭商品管理窗口
}
