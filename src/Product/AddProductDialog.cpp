#include "Product/AddProductDialog.h"
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QFormLayout>

AddProductDialog::AddProductDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("添加商品");
    setMinimumWidth(400);

    // 创建表单布局
    QFormLayout *formLayout = new QFormLayout;

    // 商品名称
    nameLineEdit = new QLineEdit(this);
    formLayout->addRow("商品名称:", nameLineEdit);

    // 商品条码
    barcodeLineEdit = new QLineEdit(this);
    formLayout->addRow("商品条码:", barcodeLineEdit);

    // 商品价格
    priceSpinBox = new QDoubleSpinBox(this);
    priceSpinBox->setRange(0.01, 9999.99);
    priceSpinBox->setDecimals(2);
    priceSpinBox->setSuffix(" 元");
    formLayout->addRow("商品价格:", priceSpinBox);

    // 商品库存
    stockSpinBox = new QSpinBox(this);
    stockSpinBox->setRange(0, 99999);
    formLayout->addRow("商品库存:", stockSpinBox);

    // 商品分类
    categoryComboBox = new QComboBox(this);
    categoryComboBox->addItems({"食品", "饮料", "日用品", "文具", "电器"});

    // 添加自定义分类按钮
    QHBoxLayout *categoryLayout = new QHBoxLayout;
    categoryLayout->addWidget(categoryComboBox);

    addCategoryButton = new QPushButton("+", this);
    addCategoryButton->setToolTip("添加新分类");
    addCategoryButton->setMaximumWidth(30);
    categoryLayout->addWidget(addCategoryButton);

    formLayout->addRow("商品分类:", categoryLayout);

    // 按钮布局
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    okButton = new QPushButton("确定", this);
    cancelButton = new QPushButton("取消", this);
    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);

    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(formLayout);
    mainLayout->addLayout(buttonLayout);

    // 连接信号和槽
    connect(okButton, &QPushButton::clicked, this, &AddProductDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &AddProductDialog::reject);
    connect(addCategoryButton, &QPushButton::clicked, this, &AddProductDialog::addNewCategory);
}

AddProductDialog::~AddProductDialog()
{
}

QString AddProductDialog::getName() const
{
    return nameLineEdit->text().trimmed();
}

QString AddProductDialog::getBarcode() const
{
    return barcodeLineEdit->text().trimmed();
}

double AddProductDialog::getPrice() const
{
    return priceSpinBox->value();
}

int AddProductDialog::getStock() const
{
    return stockSpinBox->value();
}

QString AddProductDialog::getCategory() const
{
    return categoryComboBox->currentText();
}

void AddProductDialog::addNewCategory()
{
    bool ok;
    QString newCategory = QInputDialog::getText(this, "添加新分类", "请输入新分类名称:", QLineEdit::Normal, "", &ok);

    if (ok && !newCategory.isEmpty()) {
        // 修正：检查分类是否已存在
        if (categoryComboBox->findText(newCategory) == -1) {  // 关键修改：使用 == -1 判断不存在
            categoryComboBox->addItem(newCategory);
            categoryComboBox->setCurrentText(newCategory);
        } else {
            QMessageBox::warning(this, "警告", "该分类已存在!");
        }
    }
}
