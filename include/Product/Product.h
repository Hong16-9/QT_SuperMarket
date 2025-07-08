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
#include <QSet>

// 引入小组成员的数据库接口
#include "LogIn/dbmanager.h"

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
    // 界面组件
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

    // 数据库管理器（使用单例）
    DBManager *dbManager;

    // 辅助函数
    void setupUI();
    void setupTableView();
    void loadProducts();
    void updateCategoryComboBox();
    void searchProducts(const QString &keyword, const QString &category);
};

#endif // PRODUCT_H
