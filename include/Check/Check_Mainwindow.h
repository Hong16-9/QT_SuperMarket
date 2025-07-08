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





QT_BEGIN_NAMESPACE
namespace Ui {
class Check_Mainwindow;
}
QT_END_NAMESPACE

//商品类定义
class Product {
public:
    Product(int id = -1, const QString& name = "", const QString& barcode = "",
            double price = 0.0, int stock = 0, const QString& category = "")
        : m_id(id), m_name(name), m_barcode(barcode), m_price(price),
        m_stock(stock), m_category(category) {}

    int id() const { return m_id; }
    QString name() const { return m_name; }
    QString barcode() const { return m_barcode; }
    double price() const { return m_price; }
    int stock() const { return m_stock; }
    QString category() const { return m_category; }

private:
          int m_id;           // 商品ID
QString m_name;     // 商品名称
QString m_barcode;  // 条形码
double m_price;     // 单价
int m_stock;        // 库存
QString m_category; // 分类
};

//对话框类定义，方便后续购物时弹出商品数量选择的对话框
class QuantityDialog : public QDialog {
    Q_OBJECT
public:
    explicit QuantityDialog(int maxStock, QWidget* parent = nullptr)
        : QDialog(parent), maxStock(maxStock) {
        setWindowTitle("选择数量");
        setMinimumSize(500, 240);

        QVBoxLayout *layout=new QVBoxLayout(this);            //创建垂直布局

        QLabel *label=new QLabel("请输入商品数量:", this);
        layout->addWidget(label);

        quantitySpinBox=new QSpinBox(this);                   //创建一个spinBox用来输入数量
        quantitySpinBox->setRange(1,maxStock);                //限制库存最大数量
        quantitySpinBox->setValue(1);
        layout->addWidget(quantitySpinBox);                   //让spinBox只能在1到最大数量之间选择


        QHBoxLayout *buttonLayout=new QHBoxLayout;            //创建水平布局
        QPushButton *okButton=new QPushButton("确定",this);
        QPushButton *cancelButton=new QPushButton("取消",this);

        connect(okButton,&QPushButton::clicked,this,&QDialog::accept);
        connect(cancelButton,&QPushButton::clicked,this,&QDialog::reject);    //连接信号与槽

        buttonLayout->addWidget(okButton);
        buttonLayout->addWidget(cancelButton);               //把确定和取消键放在水平布局上
        layout->addLayout(buttonLayout);                     //把水平布局放在总布局上
    }

    int getQuantity() const{return quantitySpinBox->value(); }       //返回spinBox输入的值，当点击确定时，可以调用这个值

private:
    QSpinBox *quantitySpinBox;
    int maxStock;  // 最大库存限制
};

//购物车类定义
class CartItem {
public:
    CartItem(const Product &product, int quantity=1)
        : m_product(product), m_quantity(quantity) {}                //把商品对象赋给m_product，把商品数量赋给m_quantity

    const Product &product() const {return m_product; }              //获取商品对象
    int quantity() const {return m_quantity; }                       //获取购买数量
    void setQuantity(int quantity) {m_quantity=quantity; }           //允许购物车中修改购买数量

    double getTotalPrice() const{                                   //计算条目总价（小计）
        return m_product.price()*m_quantity;
    }

private:
    Product m_product;  // 商品信息
    int m_quantity;     // 数量
};

class Check_Mainwindow : public QMainWindow
{
    Q_OBJECT

public:
    Check_Mainwindow(QWidget *parent = nullptr);
    ~Check_Mainwindow();


private slots:
    void on_chooselistWidget_itemDoubleClicked(QListWidgetItem *item);

    void on_pushButton_2_clicked();

    void on_changebtn_clicked();

    void on_deletebtn_clicked();

private:
    Ui::Check_Mainwindow *ui;
    void setupDatabase();
    void setupUI();
    void updateProduct(const QString &category="全部");        //根据不同分类来更新商品列表
    void initcategory();                                      //获取商品分类并创建分类按钮
    QStringList getcategory();                                //获取所有商品分类
    void updateCartview();                                    //更新购物车
    void updateCarttotal();                                   //更新购物车的总价

    QListView *m_cartListView;                    //购物车列表视图
    QStandardItemModel *m_cartModel;              //购物车数据模型
    std::vector<QPushButton*> m_categoryButtons;  //分类按钮


    std::vector<CartItem> m_cartItems;            //创建数据模型
};

#endif // CHECK_MAINWINDOW_H
