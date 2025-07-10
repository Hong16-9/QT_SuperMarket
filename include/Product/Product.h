#ifndef PRODUCT_H
#define PRODUCT_H

#include <QMainWindow>
#include <QStandardItemModel>
#include <QLineEdit>
#include <QComboBox>

// 引入数据库接口
#include "LogIn/dbmanager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Product; }
QT_END_NAMESPACE

class Product : public QMainWindow
{
    Q_OBJECT

public:
    explicit Product(QString name,QWidget *parent = nullptr);
    ~Product() override;

private slots:
    void onAddProductClicked();       // 对应addAction
    void onDeleteProductClicked();    // 对应deleteAction
    void onPutProductClicked();       // 对应putAction
    void onSearchProductClicked();    // 搜索按钮（非Action）
    void onImportDataClicked();       // 对应importAction
    void onExportDataClicked();       // 对应exportAction
    void onTakeProductClicked();
    void onBackToLoginTriggered();

signals:
    void backToLogin();

private:

    Ui::Product *ui;                  // UI Designer生成的界面对象
    DBManager *dbManager;             // 数据库管理器（单例）
    QStandardItemModel *productModel; // 表格数据模型

    // 关联UI控件（保持原有名称）
    QLineEdit *searchLineEdit;        // 搜索文本框（外部）
    QComboBox *categoryComboBox;      // 分类组合框（外部）

    // 辅助函数
    void initUI();                    // 初始化UI关联
    void setupConnections();          // 连接信号与槽
    void loadProducts();              // 加载商品数据
    void updateCategoryComboBox();    // 更新分类下拉框
    void searchProducts(const QString &keyword, const QString &category); // 搜索商品

    QString name;
};

#endif // PRODUCT_H
