#include "Product/Product.h"
#include "Product/AddProductDialog.h"
#include "Product/StockDialog.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QHeaderView>
#include <QStandardItem>
#include <QFormLayout>
#include <QFile>
#include <QTextStream>
#include <QDebug>


Product::Product(QWidget *parent) : QMainWindow(parent)

{
    setWindowTitle("商品管理");
    setMinimumSize(1024, 600);

    // 使用单例实例
    dbManager = &DBManager::instance();

    // 创建UI
    setupUI();

    // 加载商品数据
    loadProducts();
}

Product::~Product()
{
    // 单例无需手动删除
}

// 设置UI界面
void Product::setupUI()
{
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);

    // 按钮布局
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(8);

    // 创建按钮
    Add_Product = new QPushButton("添加商品", this);
    Delete_Product = new QPushButton("删除商品", this);
    Put_Product = new QPushButton("商品入库", this);
    importButton = new QPushButton("导入数据", this);
    exportButton = new QPushButton("导出数据", this);

    // 搜索区域
    categoryComboBox = new QComboBox(this);
    categoryComboBox->setMinimumWidth(120);
    searchLineEdit = new QLineEdit(this);
    searchLineEdit->setPlaceholderText("输入商品名称搜索...");
    searchLineEdit->setMinimumWidth(200);
    Search_Product = new QPushButton("搜索", this);

    // 添加组件到布局
    buttonLayout->addWidget(Add_Product);
    buttonLayout->addWidget(Delete_Product);
    buttonLayout->addWidget(Put_Product);
    buttonLayout->addWidget(importButton);
    buttonLayout->addWidget(exportButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(categoryComboBox);
    buttonLayout->addWidget(searchLineEdit);
    buttonLayout->addWidget(Search_Product);

    // 设置表格
    setupTableView();

    // 添加到主布局
    mainLayout->addLayout(buttonLayout);
    mainLayout->addWidget(productTableView);

    // 连接信号槽
    connect(Add_Product, &QPushButton::clicked, this, &Product::onAddProductClicked);
    connect(Delete_Product, &QPushButton::clicked, this, &Product::onDeleteProductClicked);
    connect(Put_Product, &QPushButton::clicked, this, &Product::onPutProductClicked);
    connect(Search_Product, &QPushButton::clicked, this, &Product::onSearchProductClicked);
    connect(importButton, &QPushButton::clicked, this, &Product::onImportDataClicked);
    connect(exportButton, &QPushButton::clicked, this, &Product::onExportDataClicked);
    connect(searchLineEdit, &QLineEdit::returnPressed, this, &Product::onSearchProductClicked);
}

// 设置表格视图
void Product::setupTableView()
{
    productModel = new QStandardItemModel(0, 6, this);
    productModel->setHorizontalHeaderLabels({"ID", "商品名称", "条码", "价格", "库存", "分类"});

    productTableView = new QTableView(this);
    productTableView->setModel(productModel);
    productTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    productTableView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    productTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    productTableView->setSortingEnabled(true);
    productTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    productTableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    productTableView->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    productTableView->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    productTableView->setAlternatingRowColors(true);
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

    int index = categoryComboBox->findText(currentCategory);
    if (index != -1) {
        categoryComboBox->setCurrentIndex(index);
    }
}

// 添加商品按钮点击事件
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

// 删除商品按钮点击事件
void Product::onDeleteProductClicked()
{
    QModelIndexList selectedRows = productTableView->selectionModel()->selectedRows();

    if (selectedRows.isEmpty()) {
        QMessageBox::warning(this, "警告", "请先选择要删除的商品！");
        return;
    }

    if (QMessageBox::question(this, "确认删除",
                              QString("确定要删除选中的 %1 个商品吗？").arg(selectedRows.size()),
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) {
        return;
    }

    // 按行号降序排列，避免删除行后索引变化
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

// 商品入库按钮点击事件
void Product::onPutProductClicked()
{
    QModelIndexList selectedRows = productTableView->selectionModel()->selectedRows();

    if (selectedRows.isEmpty()) {
        QMessageBox::warning(this, "警告", "请先选择要入库的商品！");
        return;
    }

    if (selectedRows.size() > 1) {
        QMessageBox::warning(this, "警告", "一次只能对一个商品进行入库操作！");
        return;
    }

    QModelIndex index = selectedRows.first();
    int productId = productModel->item(index.row(), 0)->text().toInt();
    QString productName = productModel->item(index.row(), 1)->text();
    int currentStock = productModel->item(index.row(), 4)->text().toInt();

    StockDialog dialog(productName, currentStock, this);
    if (dialog.exec() == QDialog::Accepted) {
        int addStock = dialog.getAddStock();  // 新增库存

        // 调用DBManager的updateProductStock（参数为id和变动量）
        if (dbManager->updateProductStock(productId, addStock)) {
            QMessageBox::information(this, "成功", "商品入库成功！");
            loadProducts();
        } else {
            QMessageBox::critical(this, "失败", "商品入库失败！");
        }
    }
}

// 搜索商品按钮点击事件
void Product::onSearchProductClicked()
{
    QString keyword = searchLineEdit->text().trimmed();
    QString category = categoryComboBox->currentText();

    if (keyword.isEmpty() && category == "全部") {
        // 如果没有搜索条件，加载全部商品
        loadProducts();
        return;
    }

    searchProducts(keyword, category);
}

// 搜索商品的辅助函数（适配DBManager接口）
void Product::searchProducts(const QString &keyword, const QString &category)
{
    productModel->removeRows(0, productModel->rowCount());
    QList<QMap<QString, QVariant>> products;

    if (category == "全部") {
        // 按名称搜索（如果keyword不为空）
        products = keyword.isEmpty() ?
                       dbManager->getAllProducts() :
                       dbManager->getProductsByName(keyword);
    } else {
        // 先按分类筛选，再按名称筛选
        QList<QMap<QString, QVariant>> categoryProducts = dbManager->getProductsByCategory(category);
        if (keyword.isEmpty()) {
            products = categoryProducts;
        } else {
            // 手动过滤名称包含keyword的商品
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

    updateCategoryComboBox();
}

// 导入数据按钮点击事件
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
                             QString("成功导入 %1 条记录，失败 %2 条记录。").arg(successCount).arg(failCount));
    loadProducts();
}

// 导出数据按钮点击事件
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
    // 写入标题行
    out << "商品名称,条码,价格,库存,分类\n";

    // 写入数据
    for (int i = 0; i < productModel->rowCount(); ++i) {
        QString name = productModel->item(i, 1)->text();
        QString barcode = productModel->item(i, 2)->text();
        QString price = productModel->item(i, 3)->text();
        QString stock = productModel->item(i, 4)->text();
        QString category = productModel->item(i, 5)->text();

        // 处理可能包含逗号的字段
        if (name.contains(',')) name = "\"" + name + "\"";
        if (category.contains(',')) category = "\"" + category + "\"";

        out << name << "," << barcode << "," << price << "," << stock << "," << category << "\n";
    }

    file.close();
    QMessageBox::information(this, "导出完成", "商品数据已成功导出！");
}
