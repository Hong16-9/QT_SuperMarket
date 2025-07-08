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

class StockDialog : public QDialog
{
    Q_OBJECT

public:
    explicit StockDialog(QWidget *parent = nullptr);
    ~StockDialog();

    QList<QPair<int, int>> getStockUpdates() const;

private slots:
    void onCategoryChanged(const QString &category);
    void onProductChanged(const QString &productText);
    void onAddToTableClicked();
    void onRemoveFromTableClicked();
    void onClearTableClicked();
    void onOkButtonClicked();

private:
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

    // 数据库接口（在实际项目中实现）
    QList<QMap<QString, QVariant>> getAllProducts();
    QList<QMap<QString, QVariant>> getProductsByCategory(const QString &category);
    QMap<QString, QVariant> getProductById(int productId);
};

#endif // STOCKDIALOG_H
