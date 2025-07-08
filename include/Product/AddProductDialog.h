#ifndef ADDPRODUCTDIALOG_H
#define ADDPRODUCTDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QInputDialog>

class AddProductDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddProductDialog(QWidget *parent = nullptr);
    ~AddProductDialog();

    QString getName() const;
    QString getBarcode() const;
    double getPrice() const;
    int getStock() const;
    QString getCategory() const;

private slots:
    void addNewCategory();

private:
    QLineEdit *nameLineEdit;
    QLineEdit *barcodeLineEdit;
    QDoubleSpinBox *priceSpinBox;
    QSpinBox *stockSpinBox;
    QComboBox *categoryComboBox;
    QPushButton *addCategoryButton;
    QPushButton *okButton;
    QPushButton *cancelButton;
};

#endif // ADDPRODUCTDIALOG_H
