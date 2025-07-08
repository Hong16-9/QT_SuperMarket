#ifndef STOCKDIALOG_H
#define STOCKDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QTableView>
#include <QStandardItemModel>
#include <QMap>
#include <QPair>
#include <QLabel>
#include <QString>

class StockDialog : public QDialog
{
    Q_OBJECT

public:
    // 新增：支持单个商品入库的构造函数
    explicit StockDialog(const QString &productName, int currentStock, QWidget *parent = nullptr);
    // 原有：批量入库构造函数
    explicit StockDialog(QWidget *parent = nullptr);
    ~StockDialog();

    // 获取入库信息（商品ID与入库数量的配对列表）
    QList<QPair<int, int>> getStockUpdates() const;
    // 新增：获取单个商品的入库数量（针对单个商品场景）
    int getAddStock() const;

private slots:
    void onCategoryChanged(const QString &category);
    void onProductChanged(const QString &productText);
    void onAddToTableClicked();
    void onRemoveFromTableClicked();
    void onClearTableClicked();
    void onOkButtonClicked();

private:
    // 初始化界面（通用）
    void initUI();
    // 初始化批量入库界面
    void initBatchUI();
    // 初始化单个商品入库界面
    void initSingleProductUI(const QString &productName, int currentStock);

    QComboBox *categoryComboBox;
    QComboBox *productComboBox;
    QLabel *currentStockLabel;
    QLabel *quantityLabel;
    QSpinBox *quantitySpinBox;
    QPushButton *addButton;
    QPushButton *removeButton;
    QPushButton *clearButton;
    QPushButton *okButton;
    QPushButton *cancelButton;
    QTableView *tableView;
    QStandardItemModel *stockModel;

    QMap<QString, int> productIdMap;
    int currentStock;
    // 标记是否为单个商品模式
    bool isSingleProductMode;
    // 单个商品模式下的商品ID
    int singleProductId;

    // 数据库接口（实际项目中应调用DBManager）
    QList<QMap<QString, QVariant>> getAllProducts();
    QList<QMap<QString, QVariant>> getProductsByCategory(const QString &category);
    QMap<QString, QVariant> getProductById(int productId);
};

#endif // STOCKDIALOG_H
