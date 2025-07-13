#ifndef CHECK_MAINWINDOW_H
#define CHECK_MAINWINDOW_H

#include <QMainWindow>
#include<QSqlDatabase>
#include<QSqlQuery>
#include<QListWidgetItem>
#include "LogIn/dbmanager.h"
#include<QMessageBox>
#include<QHBoxLayout>
#include<QLabel>
#include<QSpinBox>
#include<QPushButton>
#include<QStandardItemModel>
#include<QSet>
#include<QList>
#include<QString>
#include<QCoreApplication>
#include<QDebug>
#include<QInputDialog>
#include<QFormLayout>
#include<QAbstractButton>
#include<QTimer>
#include<QDialogButtonBox>





QT_BEGIN_NAMESPACE
namespace Ui {
class Check_Mainwindow;
}
QT_END_NAMESPACE

//商品类定义
class PRoduct {
public:
    PRoduct(int id = -1, const QString& name = "", const QString& barcode = "",
            double price = 0.0, int stock = 0, const QString& category = "", int monthly_sales=0)
        : m_id(id), m_name(name), m_barcode(barcode), m_price(price),
        m_stock(stock), m_category(category),m_monthly_sales(monthly_sales){}

    int id() const { return m_id; }
    QString name() const { return m_name; }
    QString barcode() const { return m_barcode; }
    double price() const { return m_price; }
    int stock() const { return m_stock; }
    QString category() const { return m_category; }
    int monthly_sales() const {return m_monthly_sales;}

private:
    int m_id;           // 商品ID
    QString m_name;     // 商品名称
    QString m_barcode;  // 条形码
    double m_price;     // 单价
    int m_stock;        // 库存
    QString m_category; // 分类
    int m_monthly_sales;//月销量
};

//对话框类定义，方便后续购物时弹出商品数量选择的对话框
class QuantityDialog : public QDialog {
    Q_OBJECT
public:
    explicit QuantityDialog(int maxStock, QWidget* parent = nullptr)
        : QDialog(parent), maxStock(maxStock) {
        setWindowTitle("选择数量");
        setMinimumSize(300, 180);   //对话框窗口大小
        setStyleSheet("background-color: #f5f5f5;"); //浅灰色背景

        QVBoxLayout *layout=new QVBoxLayout(this);            //创建垂直布局
        layout->setContentsMargins(20, 20, 20, 20);  //边缘留白
        layout->setSpacing(15);        //控件之间的间距

        //标题标签
        QLabel *label=new QLabel("请输入商品数量:", this);
        label->setStyleSheet(
            "font-size: 14px;"
            "font-weight: bold;"
            "color: #333333;"
            );
        layout->addWidget(label);


        // 数量选择区域（使用表单布局，让标签和输入框对齐）
        QFormLayout *formLayout = new QFormLayout();
        formLayout->setRowWrapPolicy(QFormLayout::DontWrapRows);          //标签和输入框在同一行
        formLayout->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter); //标签右对齐
        formLayout->setSpacing(10);


        //数量输入框
        quantitySpinBox=new QSpinBox(this);                   //创建一个spinBox用来输入数量
        quantitySpinBox->setRange(1,maxStock);                //限制库存最大数量
        quantitySpinBox->setValue(1);
        quantitySpinBox->setStyleSheet(                       //美化一下spinbox
            "QSpinBox {"
            "   padding: 8px;"
            "   border: 1px solid #ccc;"
            "   border-radius: 4px;"
            "   font-size: 13px;"
            "}"
            "QSpinBox::up-button, QSpinBox::down-button {"
            "   width: 25px;"
            "}"
            );
        quantitySpinBox->setMinimumHeight(30);                //增加输入框高度，方便点击
        layout->addWidget(quantitySpinBox);                   //让spinBox只能在1到最大数量之间选择

        formLayout->addRow("数量:", quantitySpinBox);         //将输入框添加到表单布局
        layout->addLayout(formLayout);


        QHBoxLayout *buttonLayout=new QHBoxLayout;            //创建水平布局
        buttonLayout->setSpacing(10);
        buttonLayout->setAlignment(Qt::AlignRight);        //按钮靠右显示

        QPushButton *okButton=new QPushButton("确定",this);
        okButton->setStyleSheet(                            //美化一下确定按钮
            "QPushButton {"
            "   padding: 6px 15px;"
            "   background-color: #4285f4;"
            "   color: white;"
            "   border: none;"
            "   border-radius: 4px;"
            "   font-size: 13px;"
            "}"
            "QPushButton:hover {"
            "   background-color: #3367d6;"
            "}"
            );


        QPushButton *cancelButton=new QPushButton("取消",this);
        cancelButton->setStyleSheet(                        //美化一下取消按钮
            "QPushButton {"
            "   padding: 6px 15px;"
            "   background-color: #ffffff;"
            "   border: 1px solid #ccc;"
            "   border-radius: 4px;"
            "   font-size: 13px;"
            "}"
            "QPushButton:hover {"
            "   background-color: #f0f0f0;"
            "}"
            );

        connect(okButton,&QPushButton::clicked,this,&QDialog::accept);
        connect(cancelButton,&QPushButton::clicked,this,&QDialog::reject);    //连接信号与槽

        buttonLayout->addWidget(okButton);
        buttonLayout->addWidget(cancelButton);               //把确定和取消键放在水平布局上
        layout->addLayout(buttonLayout);                     //把水平布局放在总布局上

        //使用定时器在对话框显示后将光标移动到文本末尾
        QTimer::singleShot(0, [this]() {                    //使用定时器确保在对话框出来后再执行以下操作
            if (QLineEdit *lineEdit = quantitySpinBox->findChild<QLineEdit*>()) {      //获取QLineEdit对象
                lineEdit->setCursorPosition(lineEdit->text().length());                //把光标移到文本末尾
                lineEdit->setFocus();                    //让输入框获得焦点，这样可以直接输入内容，不用先点击文本框

            }
        });

    }

    int getQuantity() const{return quantitySpinBox->value(); }       //返回spinBox输入的值，当点击确定时，可以调用这个值

private:
    QSpinBox *quantitySpinBox;
    int maxStock;  // 最大库存限制
};

//购物车类定义
class CartItem {
public:
    CartItem(const PRoduct &product, int quantity=1)
        : m_product(product), m_quantity(quantity) {}                //把商品对象赋给m_product，把商品数量赋给m_quantity

    const PRoduct &product() const {return m_product; }              //获取商品对象
    int quantity() const {return m_quantity; }                       //获取购买数量
    void setQuantity(int quantity) {m_quantity=quantity; }           //允许购物车中修改购买数量

    double getTotalPrice() const{                                   //计算条目总价（小计）
        return m_product.price()*m_quantity;
    }

private:
    PRoduct m_product;  // 商品信息
    int m_quantity;     // 数量
};

//主窗口类定义
class Check_Mainwindow : public QMainWindow
{
    Q_OBJECT

public:
    Check_Mainwindow(QString username,QWidget *parent = nullptr);
    ~Check_Mainwindow();


private slots:
    void chooselistWidgetitemDoubleClicked(QListWidgetItem *item);

    void changebtnclicked();

    void deletebtnclicked();

    void clearbtnclicked();

    void paybtnclicked();

    void backbtnclicked();

    void addmemberclicked();

    void on_historyBtn_clicked();

signals:
    void backToLogin();  //返回登录的信号


private:
    Ui::Check_Mainwindow *ui;
    void setupUI();                                           //设置UI
    void updateProduct(const QString &category="全部");        //根据不同分类来更新商品列表
    void initcategory();                                      //获取商品分类并创建分类按钮
    QStringList getcategory();                                //获取所有商品分类
    void updateCartview();                                    //更新购物车
    void updateCarttotal();                                   //更新购物车的总价

    QListView *m_cartListView;                    //购物车列表视图
    QStandardItemModel *m_cartModel;              //购物车数据模型
    std::vector<QPushButton*> m_categoryButtons;  //分类按钮
    QWidget* categoryWidget;                      //分类按钮容器


    std::vector<CartItem> m_cartItems;            //创建数据模型

    QString username;  //新增，接受收银员名称
    int cashierID=-1;      //储存收银员的ID



};

#endif // CHECK_MAINWINDOW_H
