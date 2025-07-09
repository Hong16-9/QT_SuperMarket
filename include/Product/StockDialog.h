#ifndef STOCKDIALOG_H
#define STOCKDIALOG_H

#include <QDialog>
#include <QSpinBox>
#include <QLabel>
#include <QTableView>
#include <QStandardItemModel>
#include <QMap>
#include <QPair>
#include <QComboBox>
#include <QPushButton>
#include <QHeaderView>

// 操作类型：入库/出库
enum class StockOperation {
    In,  // 入库
    Out  // 出库
};

class StockDialog : public QDialog
{
    Q_OBJECT

public:
    // 单个商品操作（支持入库/出库）
    StockDialog(StockOperation op, const QString &productName, int currentStock, QWidget *parent = nullptr);
    // 批量操作（支持入库/出库）
    StockDialog(StockOperation op, QWidget *parent = nullptr);
    ~StockDialog() override;

    // 获取库存变动列表（批量模式）：<商品ID, 变动量>（入库为正，出库为负）
    QList<QPair<int, int>> getStockChanges() const;
    // 获取单个商品的变动量（单个模式）
    int getChangeAmount() const;

private slots:
    void onOkButtonClicked();
    void onCategoryChanged(const QString &category);
    void onProductChanged(const QString &productText);
    void onAddToTableClicked();
    void onRemoveFromTableClicked();
    void onClearTableClicked();

private:
    void initUI();
    void initSingleProductUI(const QString &productName, int currentStock);
    void initBatchUI();

    // 操作类型（核心标识）
    StockOperation operation;
    // 模式标识
    bool isSingleProductMode;
    // 单个商品信息
    int singleProductId = -1;
    int currentStock = 0;

    // UI组件
    QSpinBox *quantitySpinBox;
    QLabel *currentStockLabel;
    QLabel *quantityLabel;
    QPushButton *okButton;
    QPushButton *cancelButton;
    QPushButton *addButton;
    QPushButton *removeButton;
    QPushButton *clearButton;
    QComboBox *categoryComboBox;
    QComboBox *productComboBox;
    QTableView *tableView;
    QStandardItemModel *stockModel;
    QMap<QString, int> productIdMap;

    // 数据库接口（复用原有）
    QList<QMap<QString, QVariant>> getAllProducts();
    QList<QMap<QString, QVariant>> getProductsByCategory(const QString &category);
    QMap<QString, QVariant> getProductById(int productId);
};

#endif // STOCKDIALOG_H
