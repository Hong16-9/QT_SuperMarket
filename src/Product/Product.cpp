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

Product::Product(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle("商品管理");
    setMinimumSize(1024, 600);

    // 创建UI
    setupUI();

    // 加载商品数据
    loadProducts();
}

Product::~Product()
{

}

// 实现setupUI（之前声明缺失）
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

// 实现setupTableView（之前声明缺失）
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

// 实现loadProducts（之前声明缺失）
void Product::loadProducts()
{
    productModel->removeRows(0, productModel->rowCount());

    // 调用DBManager的函数（需与组员的函数名一致）
    QList<QMap<QString, QVariant>> products = dbManager->getAllProducts();  // 假设函数名一致

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

// 实现updateCategoryComboBox（之前声明缺失）
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

// 其他函数实现（保持不变，但需确保数据库函数调用正确）
void Product::onAddProductClicked()
{
    AddProductDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        QString name = dialog.getName();
        QString barcode = dialog.getBarcode();
        double price = dialog.getPrice();
        int stock = dialog.getStock();
        QString category = dialog.getCategory();

        if (dbManager->addProduct(name, barcode, price, stock, category)) {  // 调用DBManager
            QMessageBox::information(this, "成功", "商品添加成功！");
            loadProducts();
        } else {
            QMessageBox::critical(this, "失败", "商品添加失败！可能是条码已存在。");
        }
    }
}

// 其余函数（onDeleteProductClicked、onPutProductClicked等）保持不变，
// 但需确保所有dbManager的调用都使用DBManager类，且函数名与组员一致。
