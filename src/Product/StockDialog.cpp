#include "Product/StockDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QSpinBox>
#include <QPushButton>
#include <QTableView>
#include <QStandardItemModel>
#include <QHeaderView>
#include <QMessageBox>
#include <QFrame>
#include <QMap>
#include <QList>
#include <QPair>
#include <algorithm>
// 包含数据库头文件（根据实际项目路径调整）
#include "LogIn/dbmanager.h"

// 单个商品操作（入库/出库）
StockDialog::StockDialog(StockOperation op, const QString &productName, int currentStock, QWidget *parent)
    : QDialog(parent), operation(op), isSingleProductMode(true), currentStock(currentStock)
{
    setWindowTitle(op == StockOperation::In ? "商品入库" : "商品出库");
    setMinimumWidth(400);
    initUI();
    initSingleProductUI(productName, currentStock);
}

// 批量操作（入库/出库）
StockDialog::StockDialog(StockOperation op, QWidget *parent)
    : QDialog(parent), operation(op), isSingleProductMode(false)
{
    setWindowTitle(op == StockOperation::In ? "批量入库" : "批量出库");
    setMinimumWidth(600);
    initUI();
    initBatchUI();
}

StockDialog::~StockDialog()
{
    // 释放动态分配的UI组件（Qt父对象机制会自动处理，此处可省略）
}

// 初始化通用UI组件
void StockDialog::initUI()
{
    // 数量输入框（通用）
    quantitySpinBox = new QSpinBox(this);
    quantitySpinBox->setRange(1, 9999);
    quantitySpinBox->setValue(10);

    // 按钮（通用）
    okButton = new QPushButton("确认", this);
    cancelButton = new QPushButton("取消", this);
    connect(okButton, &QPushButton::clicked, this, &StockDialog::onOkButtonClicked);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    // 批量模式专用组件初始化（先置空，在initBatchUI中实例化）
    categoryComboBox = nullptr;
    productComboBox = nullptr;
    tableView = nullptr;
    stockModel = nullptr;
    addButton = nullptr;
    removeButton = nullptr;
    clearButton = nullptr;
    quantityLabel = nullptr;
    currentStockLabel = nullptr;
}

// 初始化单个商品界面
void StockDialog::initSingleProductUI(const QString &productName, int currentStock)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 商品名称
    QLabel *productNameLabel = new QLabel(QString("商品名称: %1").arg(productName), this);
    QFont font = productNameLabel->font();
    font.setBold(true);
    productNameLabel->setFont(font);
    mainLayout->addWidget(productNameLabel);

    // 当前库存
    currentStockLabel = new QLabel(QString("当前库存: %1").arg(currentStock), this);
    mainLayout->addWidget(currentStockLabel);

    // 数量输入
    QHBoxLayout *quantityLayout = new QHBoxLayout;
    quantityLabel = new QLabel(
        operation == StockOperation::In ? "入库数量:" : "出库数量:",
        this
        );
    quantityLayout->addWidget(quantityLabel);
    quantityLayout->addWidget(quantitySpinBox);
    quantityLayout->addStretch();
    mainLayout->addLayout(quantityLayout);

    // 按钮布局
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    mainLayout->addLayout(buttonLayout);

    // 获取商品ID（从数据库）
    QList<QMap<QString, QVariant>> products = getAllProducts();
    for (const auto &product : products) {
        if (product["name"].toString() == productName) {
            singleProductId = product["id"].toInt();
            break;
        }
    }
}

// 初始化批量操作界面
void StockDialog::initBatchUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 筛选区域
    QFormLayout *filterLayout = new QFormLayout;

    // 分类选择
    categoryComboBox = new QComboBox(this);
    categoryComboBox->addItem("全部");
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

    // 数量输入
    QHBoxLayout *quantityLayout = new QHBoxLayout;
    quantityLabel = new QLabel(
        operation == StockOperation::In ? "入库数量:" : "出库数量:",
        this
        );
    addButton = new QPushButton(
        operation == StockOperation::In ? "添加到入库列表" : "添加到出库列表",
        this
        );

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
    stockModel->setHorizontalHeaderLabels({
        "商品ID",
        "商品名称",
        "当前库存",
        operation == StockOperation::In ? "入库数量" : "出库数量"
    });

    tableView = new QTableView(this);
    tableView->setModel(stockModel);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    tableView->setColumnHidden(0, true); // 隐藏ID列
    tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableView->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    tableView->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    mainLayout->addWidget(tableView);

    // 按钮布局
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    removeButton = new QPushButton("移除所选", this);
    clearButton = new QPushButton("清空列表", this);

    buttonLayout->addWidget(removeButton);
    buttonLayout->addWidget(clearButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    mainLayout->addLayout(buttonLayout);

    // 连接信号槽
    connect(categoryComboBox, &QComboBox::currentTextChanged, this, &StockDialog::onCategoryChanged);
    connect(productComboBox, &QComboBox::currentTextChanged, this, &StockDialog::onProductChanged);
    connect(addButton, &QPushButton::clicked, this, &StockDialog::onAddToTableClicked);
    connect(removeButton, &QPushButton::clicked, this, &StockDialog::onRemoveFromTableClicked);
    connect(clearButton, &QPushButton::clicked, this, &StockDialog::onClearTableClicked);

    // 初始化商品列表
    onCategoryChanged("全部");
}

// 确认按钮点击事件
void StockDialog::onOkButtonClicked()
{
    if (isSingleProductMode) {
        // 单个商品校验
        int amount = quantitySpinBox->value();
        if (amount <= 0) {
            QMessageBox::warning(this, "警告", "数量必须大于0");
            return;
        }

        // 出库时校验库存
        if (operation == StockOperation::Out && amount > currentStock) {
            QMessageBox::critical(this, "错误",
                                  QString("出库数量超过当前库存！当前库存: %1").arg(currentStock));
            return;
        }

        accept();
    } else {
        // 批量操作校验
        if (stockModel->rowCount() == 0) {
            QMessageBox::warning(this, "警告",
                                 operation == StockOperation::In ? "入库列表为空" : "出库列表为空");
            return;
        }

        // 批量出库时校验每个商品的库存
        if (operation == StockOperation::Out) {
            for (int i = 0; i < stockModel->rowCount(); ++i) {
                int stock = stockModel->data(stockModel->index(i, 2)).toInt();
                int outAmount = stockModel->data(stockModel->index(i, 3)).toInt();
                if (outAmount > stock) {
                    QString name = stockModel->data(stockModel->index(i, 1)).toString();
                    QMessageBox::critical(this, "错误",
                                          QString("商品「%1」出库数量超过库存！当前库存: %2")
                                              .arg(name).arg(stock));
                    return;
                }
            }
        }

        accept();
    }
}

// 分类下拉框变化事件
void StockDialog::onCategoryChanged(const QString &category)
{
    if (isSingleProductMode || !productComboBox) return;

    productComboBox->clear();
    productIdMap.clear();

    QList<QMap<QString, QVariant>> products;
    if (category == "全部") {
        products = getAllProducts();
    } else {
        products = getProductsByCategory(category);
    }

    for (const auto &product : products) {
        QString name = product["name"].toString();
        int id = product["id"].toInt();
        productComboBox->addItem(name);
        productIdMap[name] = id;
    }
}

// 商品下拉框变化事件
void StockDialog::onProductChanged(const QString &productText)
{
    if (isSingleProductMode || !productComboBox) return;

    if (productIdMap.contains(productText)) {
        int productId = productIdMap[productText];
        QMap<QString, QVariant> product = getProductById(productId);
        if (!product.isEmpty()) {
            currentStock = product["stock"].toInt();
            currentStockLabel->setText(QString("当前库存: %1").arg(currentStock));
        }
    }
}

// 添加到列表按钮事件
void StockDialog::onAddToTableClicked()
{
    if (isSingleProductMode || !productComboBox) return;

    QString productName = productComboBox->currentText();
    if (productName.isEmpty() || !productIdMap.contains(productName)) {
        QMessageBox::warning(this, "警告", "请选择商品");
        return;
    }

    int productId = productIdMap[productName];
    int quantity = quantitySpinBox->value();
    int stock = currentStock;

    // 检查是否已在列表中
    for (int i = 0; i < stockModel->rowCount(); ++i) {
        if (stockModel->data(stockModel->index(i, 0)).toInt() == productId) {
            QStandardItem *item = stockModel->item(i, 3);
            item->setText(QString::number(item->text().toInt() + quantity));
            return;
        }
    }

    // 添加新行
    QList<QStandardItem*> items;
    items.append(new QStandardItem(QString::number(productId)));
    items.append(new QStandardItem(productName));
    items.append(new QStandardItem(QString::number(stock)));
    items.append(new QStandardItem(QString::number(quantity)));
    foreach (QStandardItem *item, items) {
        item->setEditable(false);
    }
    stockModel->appendRow(items);
}

// 移除所选按钮事件
void StockDialog::onRemoveFromTableClicked()
{
    if (isSingleProductMode || !tableView) return;

    QModelIndexList selectedRows = tableView->selectionModel()->selectedRows();
    if (selectedRows.isEmpty()) {
        QMessageBox::warning(this, "警告", "请选择要移除的商品");
        return;
    }

    // 按行号降序删除，避免索引错乱
    std::sort(selectedRows.begin(), selectedRows.end(), [](const QModelIndex &a, const QModelIndex &b) {
        return a.row() > b.row();
    });

    foreach (const QModelIndex &index, selectedRows) {
        stockModel->removeRow(index.row());
    }
}

// 清空列表按钮事件
void StockDialog::onClearTableClicked()
{
    if (isSingleProductMode || !stockModel) return;
    stockModel->removeRows(0, stockModel->rowCount());
}

// 获取库存变动列表（入库为正，出库为负）
QList<QPair<int, int>> StockDialog::getStockChanges() const
{
    QList<QPair<int, int>> changes;

    if (isSingleProductMode) {
        int amount = operation == StockOperation::In ?
                         quantitySpinBox->value() :
                         -quantitySpinBox->value();
        if (singleProductId != -1) {
            changes.append(qMakePair(singleProductId, amount));
        }
    } else if (stockModel) {
        for (int i = 0; i < stockModel->rowCount(); ++i) {
            int productId = stockModel->data(stockModel->index(i, 0)).toInt();
            int quantity = stockModel->data(stockModel->index(i, 3)).toInt();
            int amount = operation == StockOperation::In ? quantity : -quantity;
            changes.append(qMakePair(productId, amount));
        }
    }

    return changes;
}

// 获取单个商品变动量
int StockDialog::getChangeAmount() const
{
    if (!isSingleProductMode) return 0;
    return operation == StockOperation::In ?
               quantitySpinBox->value() :
               -quantitySpinBox->value();
}

// 数据库接口：获取所有商品
QList<QMap<QString, QVariant>> StockDialog::getAllProducts()
{
    // 实际项目中调用数据库单例
    return DBManager::instance().getAllProducts();
}

// 数据库接口：按分类获取商品
QList<QMap<QString, QVariant>> StockDialog::getProductsByCategory(const QString &category)
{
    return DBManager::instance().getProductsByCategory(category);
}

// 数据库接口：按ID获取商品
QMap<QString, QVariant> StockDialog::getProductById(int productId)
{
    return DBManager::instance().getProductById(productId);
}
