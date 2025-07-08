#include "Product/StockDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QTableView>
#include <QStandardItemModel>
#include <QHeaderView>
#include <QMessageBox>
#include <QInputDialog>

StockDialog::StockDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("商品入库");
    setMinimumWidth(600);

    // 创建布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 筛选区域
    QFormLayout *filterLayout = new QFormLayout;

    // 分类选择
    categoryComboBox = new QComboBox(this);
    categoryComboBox->addItem("全部");
    // 从数据库获取分类列表
    QList<QMap<QString, QVariant>> products = getAllProducts();
    QSet<QString> categories;

    for (const auto &product : products) {
        categories.insert(product["category"].toString());
    }

    categoryComboBox->addItems(categories.values());
    filterLayout->addRow("选择分类:", categoryComboBox);

    // 商品选择
    productComboBox = new QComboBox(this);
    filterLayout->addRow("选择商品:", productComboBox);

    // 当前库存显示
    currentStockLabel = new QLabel("当前库存: --", this);
    filterLayout->addRow("", currentStockLabel);

    // 入库数量
    QHBoxLayout *quantityLayout = new QHBoxLayout;
    quantityLabel = new QLabel("入库数量:", this);
    quantitySpinBox = new QSpinBox(this);
    quantitySpinBox->setRange(1, 9999);
    quantitySpinBox->setValue(10);
    addButton = new QPushButton("添加到列表", this);

    quantityLayout->addWidget(quantityLabel);
    quantityLayout->addWidget(quantitySpinBox);
    quantityLayout->addStretch();
    quantityLayout->addWidget(addButton);

    filterLayout->addRow("", quantityLayout);

    mainLayout->addLayout(filterLayout);

    // 分割线
    QFrame *line = new QFrame(this);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    mainLayout->addWidget(line);

    // 表格视图
    stockModel = new QStandardItemModel(0, 4, this);
    stockModel->setHorizontalHeaderLabels({"商品ID", "商品名称", "当前库存", "入库数量"});

    tableView = new QTableView(this);
    tableView->setModel(stockModel);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    tableView->setColumnHidden(0, true); // 隐藏商品ID列
    tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    tableView->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    tableView->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);

    mainLayout->addWidget(tableView);

    // 按钮布局
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    removeButton = new QPushButton("移除所选", this);
    clearButton = new QPushButton("清空列表", this);
    okButton = new QPushButton("确认入库", this);
    cancelButton = new QPushButton("取消", this);

    buttonLayout->addWidget(removeButton);
    buttonLayout->addWidget(clearButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);

    mainLayout->addLayout(buttonLayout);

    // 连接信号和槽
    connect(categoryComboBox, &QComboBox::currentTextChanged, this, &StockDialog::onCategoryChanged);
    connect(productComboBox, &QComboBox::currentTextChanged, this, &StockDialog::onProductChanged);
    connect(addButton, &QPushButton::clicked, this, &StockDialog::onAddToTableClicked);
    connect(removeButton, &QPushButton::clicked, this, &StockDialog::onRemoveFromTableClicked);
    connect(clearButton, &QPushButton::clicked, this, &StockDialog::onClearTableClicked);
    connect(okButton, &QPushButton::clicked, this, &StockDialog::onOkButtonClicked);
    connect(cancelButton, &QPushButton::clicked, this, &StockDialog::reject);

    // 初始化
    onCategoryChanged("全部");
}

StockDialog::~StockDialog()
{
}

void StockDialog::onCategoryChanged(const QString &category)
{
    productComboBox->clear();

    QList<QMap<QString, QVariant>> products;
    if (category == "全部") {
        products = getAllProducts();
    } else {
        products = getProductsByCategory(category);
    }

    // 存储商品ID和名称的映射
    productIdMap.clear();

    for (const auto &product : products) {
        QString name = product["name"].toString();
        int id = product["id"].toInt();
        productComboBox->addItem(name);
        productIdMap[name] = id;
    }
}

void StockDialog::onProductChanged(const QString &productText)
{
    if (productIdMap.contains(productText)) {
        int productId = productIdMap[productText];
        QMap<QString, QVariant> product = getProductById(productId);
        if (!product.isEmpty()) {
            currentStock = product["stock"].toInt();
            currentStockLabel->setText(QString("当前库存: %1").arg(currentStock));
        }
    }
}

void StockDialog::onAddToTableClicked()
{
    QString productName = productComboBox->currentText();
    if (productName.isEmpty() || !productIdMap.contains(productName)) {
        QMessageBox::warning(this, "警告", "请选择商品");
        return;
    }

    int productId = productIdMap[productName];
    int quantity = quantitySpinBox->value();

    // 检查是否已添加过该商品
    for (int i = 0; i < stockModel->rowCount(); ++i) {
        if (stockModel->data(stockModel->index(i, 0)).toInt() == productId) {
            // 更新数量
            QStandardItem *item = stockModel->item(i, 3);
            int currentQuantity = item->text().toInt();
            item->setText(QString::number(currentQuantity + quantity));
            return;
        }
    }

    // 添加到表格
    QList<QStandardItem*> items;
    items.append(new QStandardItem(QString::number(productId)));
    items.append(new QStandardItem(productName));
    items.append(new QStandardItem(QString::number(currentStock)));
    items.append(new QStandardItem(QString::number(quantity)));

    stockModel->appendRow(items);
}

void StockDialog::onRemoveFromTableClicked()
{
    QModelIndexList selectedRows = tableView->selectionModel()->selectedRows();
    if (selectedRows.isEmpty()) {
        QMessageBox::warning(this, "警告", "请选择要移除的商品");
        return;
    }

    // 从后往前删除，避免索引变化
    std::sort(selectedRows.begin(), selectedRows.end(), [](const QModelIndex &a, const QModelIndex &b) {
        return a.row() > b.row();
    });

    for (const auto &index : selectedRows) {
        stockModel->removeRow(index.row());
    }
}

void StockDialog::onClearTableClicked()
{
    stockModel->removeRows(0, stockModel->rowCount());
}

void StockDialog::onOkButtonClicked()
{
    if (stockModel->rowCount() == 0) {
        QMessageBox::warning(this, "警告", "入库列表为空，请添加商品");
        return;
    }

    accept();
}

QList<QPair<int, int>> StockDialog::getStockUpdates() const
{
    QList<QPair<int, int>> updates;

    for (int i = 0; i < stockModel->rowCount(); ++i) {
        int productId = stockModel->data(stockModel->index(i, 0)).toInt();
        int quantity = stockModel->data(stockModel->index(i, 3)).toInt();
        updates.append(qMakePair(productId, quantity));
    }

    return updates;
}

// 模拟数据库接口
QList<QMap<QString, QVariant>> StockDialog::getAllProducts()
{
    // 实际项目中应调用数据库接口
    QList<QMap<QString, QVariant>> products;

    // 示例数据
    QMap<QString, QVariant> product1;
    product1["id"] = 1;
    product1["name"] = "可乐";
    product1["barcode"] = "10001";
    product1["price"] = 3.5;
    product1["stock"] = 100;
    product1["category"] = "饮料";

    QMap<QString, QVariant> product2;
    product2["id"] = 2;
    product2["name"] = "饼干";
    product2["barcode"] = "10002";
    product2["price"] = 5.0;
    product2["stock"] = 50;
    product2["category"] = "食品";

    products.append(product1);
    products.append(product2);

    return products;
}

QList<QMap<QString, QVariant>> StockDialog::getProductsByCategory(const QString &category)
{
    QList<QMap<QString, QVariant>> allProducts = getAllProducts();
    QList<QMap<QString, QVariant>> result;

    for (const auto &product : allProducts) {
        if (product["category"].toString() == category) {
            result.append(product);
        }
    }

    return result;
}

QMap<QString, QVariant> StockDialog::getProductById(int productId)
{
    QList<QMap<QString, QVariant>> allProducts = getAllProducts();

    for (const auto &product : allProducts) {
        if (product["id"].toInt() == productId) {
            return product;
        }
    }

    return QMap<QString, QVariant>();
}
