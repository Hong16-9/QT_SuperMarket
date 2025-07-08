#ifndef PRODUCT_H
#define PRODUCT_H

#include <QMainWindow>
#include <QTableView>
#include <QStandardItemModel>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QWidget>
#include <QMap>
#include <QPair>

// 引入小组成员的数据库接口（确保路径正确）
#include "LogIn/dbmanager.h"  // 假设该头文件中定义了DBManager类

class Product : public QMainWindow
{
    Q_OBJECT

public:
    explicit Product(QWidget *parent = nullptr);
    ~Product() override;

private slots:
    void onAddProductClicked();
    void onDeleteProductClicked();
    void onPutProductClicked();
    void onSearchProductClicked();
    void onImportDataClicked();
    void onExportDataClicked();

private:
    // 界面组件（声明）
    QTableView *productTableView;
    QStandardItemModel *productModel;
    QLineEdit *searchLineEdit;
    QComboBox *categoryComboBox;
    QPushButton *Add_Product;
    QPushButton *Delete_Product;
    QPushButton *Put_Product;
    QPushButton *Search_Product;
    QPushButton *importButton;
    QPushButton *exportButton;

    // 数据库管理器（修正类名为DBManager，与组员保持一致）
    DBManager *dbManager;  // 关键：类名改为DBManager（大写DB）

    // 声明所有成员函数（之前缺失，导致"未声明"错误）
    void setupUI();                  // 初始化UI
    void setupTableView();           // 设置表格视图
    void loadProducts();             // 加载商品数据
    void updateCategoryComboBox();   // 更新分类下拉框
};

#endif // PRODUCT_H
