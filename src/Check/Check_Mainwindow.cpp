#include "Check/Check_Mainwindow.h"
#include "ui_Check_Mainwindow.h"

Check_Mainwindow::Check_Mainwindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Check_Mainwindow)
{
    if (!DBManager::instance().initialize()) {
        QMessageBox::critical(this, "数据库错误", "无法初始化数据库，请检查配置");
        exit(1); // 数据库初始化失败则退出
    }
    ui->setupUi(this);
    setupUI();
    setWindowTitle("超市收银系统");
    initcategory();
}

Check_Mainwindow::~Check_Mainwindow()
{
    delete ui;
}


void Check_Mainwindow::setupUI(){
    m_cartModel=new QStandardItemModel(0, 4, this);                             //初始化购物车模型
    m_cartModel->setHorizontalHeaderLabels({"商品", "单价", "数量", "小计"});     //设置购物车条目的标题

    ui->cartlistView->setModel(m_cartModel);                                   //关联模型和视图
}


//获取所有商品分类
QStringList Check_Mainwindow::getcategory() {
    QStringList categories;
    categories << "全部";              //默认显示全部

    //从数据库查询分类
    QSqlQuery query = DBManager::instance().executeQuery(
        "SELECT DISTINCT category FROM products WHERE category != ''"
        );

    while (query.next()) {
        categories << query.value(0).toString();              //查询后添加到字符串列表
    }
    return categories;
}


//获取商品分类并创建分类按钮
void Check_Mainwindow::initcategory(){
    QStringList categories = getcategory();                    //先获取商品分类
    QHBoxLayout* categoriesLayout = qobject_cast<QHBoxLayout*>(
        findChild<QGroupBox*>("productgroupBox")->layout()->itemAt(0)->layout()//将groupBox里的按钮改为水平布局
        );

    for (const QString& category : categories) {
        QPushButton* btn = new QPushButton(category);
        btn->setCheckable(true);                                //设置为单选按钮
        if (category == "全部") btn->setChecked(true);          //默认选中“全部”分类
        m_categoryButtons.push_back(btn);                      //把按钮存入容器
        categoriesLayout->addWidget(btn);                      //把每个按钮添加到布局中
    }

    // 初始显示全部商品
    updateProduct();

}


//根据不同分类来更新商品列表
void Check_Mainwindow::updateProduct(const QString& category){
    ui->productlistWidget->clear();
    QString sql ="SELECT id,name,price,stock,category FROM products";   //创建sql字符串
    QVariantList params;                                                //创建容器用来储存sql查询的分类参数

    if (category != "全部商品") {
        sql+=" WHERE category=?";
        params << category;                                             //添加筛选的条件
    }
    QSqlQuery query = DBManager::instance().executeQuery(sql, params);  //执行数据库查询

    while (query.next()) {
        Product product(
            query.value("id").toInt(),
            query.value("name").toString(),
            "",
            query.value("price").toDouble(),
            query.value("stock").toInt(),
            query.value("category").toString()                           //构建商品对象
            );

        QListWidgetItem* item = new QListWidgetItem;
        item->setText(QString("%1\n%2元/个\n库存: %3")
                          .arg(product.name())
                          .arg(product.price(), 0, 'f', 2)
                          .arg(product.stock()));                         //给productlistWidget创建列表项并设置文本


        item->setData(Qt::UserRole, product.id());                        //设置商品ID作为标识
        item->setIcon(QIcon::fromTheme("package-x-generic", QIcon(":/icons/product.png")));   //设置商品图片

        ui->productlistWidget->addItem(item);
    }




}


//更新购物车的总价
void Check_Mainwindow::updateCarttotal(){
    double total=0;
    for (const auto &item:m_cartItems) {                                              //历遍购物车，累加总价
        total+=item.getTotalPrice();
    }
    ui->totalLabel->setText(QString("总计: %1元").arg(total, 0, 'f', 2));              //显示总价
}


//更新购物车
void Check_Mainwindow::updateCartview(){
    m_cartModel->removeRows(0, m_cartModel->rowCount());                              //清空购物车界面的旧显示（从第0行删到总行）

    for (const CartItem& item:m_cartItems) {                                          //逐个处理m_cartItems中的商品条目
        const Product& product=item.product();                                        //获取商品信息

        QList<QStandardItem*> rowItems;                                               //把商品信息转化为QStandardItem类型的单元格数据，每行有四个单元格
        rowItems<<new QStandardItem(product.name());
        rowItems<<new QStandardItem(QString::number(product.price(), 'f', 2));
        rowItems<<new QStandardItem(QString::number(item.quantity()));
        rowItems<<new QStandardItem(QString::number(item.getTotalPrice(), 'f', 2));

        rowItems[0]->setData(product.id(),Qt::UserRole);                              //存储商品ID便于后续操作

        m_cartModel->appendRow(rowItems);                                             //把数据加到模型中
    }

    updateCarttotal();
}


//双击商品弹出选择数量的对话框，随后将商品加入购物车
void Check_Mainwindow::on_chooselistWidget_itemDoubleClicked(QListWidgetItem *item){
    if (!item) return;                                                 //确保选中有效商品

    int productId = item->data(Qt::UserRole).toInt();                  //获取选中的商品ID

    QSqlQuery query = DBManager::instance().executeQuery(              //查询商品详情
        "SELECT id, name, price, stock FROM products WHERE id = ?",
        {productId}
        );

    if (!query.next()) {                                                //验证商品存在性
        QMessageBox::warning(this, "错误", "商品信息不存在");
        return;
    }

    Product product(                                                     //构建商品对象
        productId,
        query.value("name").toString(),
        "",
        query.value("price").toDouble(),
        query.value("stock").toInt()
        );

    if (product.stock() <= 0) {                                           //检查商品库存
        QMessageBox::warning(this, "库存不足", "该商品已售罄");
        return;
    }


    //弹出模态的数量选择对话框
    QuantityDialog dialog(product.stock(), this);                         //传递商品库存
    if (dialog.exec()==QDialog::Accepted) {
        int quantity=dialog.getQuantity();                                //返回用户选择的数量


        bool found=false;                                                 //检查是否已在购物车中
        for (auto& cartItem:m_cartItems) {                                //历遍购物车，如果在购物车中（id相同）就合并商品数量
            if (cartItem.product().id()==productId) {
                cartItem.setQuantity(cartItem.quantity()+quantity);
                found=true;
                break;
            }
        }


        if (!found) {
            m_cartItems.emplace_back(product,quantity);                     //不在购物车则添加至购物车中
        }

        updateCartview();
    }

}


//改数键的功能
void Check_Mainwindow::on_changebtn_clicked()
{   QModelIndex index=ui->cartlistView->currentIndex();                                 //获取购物车项目索引
    if (!index.isValid()) {                                                             //如果没有索引就给出提示
        QMessageBox::warning(this, "提示", "请先选择要修改的商品");
        return;
    }

    int row = index.row();                                                              //获取行号，商品ID和商品数量
    int productId=m_cartModel->item(row, 0)->data(Qt::UserRole).toInt();
    int currentQuantity=m_cartModel->item(row, 2)->text().toInt();

    QSqlQuery query=DBManager::instance().executeQuery(                                 //查询数据库中的商品库存
        "SELECT stock FROM products WHERE id = ?", {productId}
        );
    if (!query.next()) return;
    int maxStock=query.value("stock").toInt();

    QuantityDialog dialog(maxStock, this);                                              //弹出修改数量对话框
    dialog.setWindowTitle("修改数量");
    dialog.findChild<QSpinBox*>()->setValue(currentQuantity);                           //把弹出的修改数量对话框初始值改为当前的商品数量

    if (dialog.exec()==QDialog::Accepted) {                                             //当用户点击确定时，将商品数量更改为确定的值
        int newQuantity=dialog.getQuantity();

        for (auto &item:m_cartItems) {                                                  //更新购物车
            if (item.product().id()==productId) {
                item.setQuantity(newQuantity);
                break;
            }
        }
        updateCartview();
    }

}



//删除键的功能
void Check_Mainwindow::on_deletebtn_clicked()
{   QModelIndex index=ui->cartlistView->currentIndex();
    if (!index.isValid()) {
        QMessageBox::warning(this, "提示", "请先选择要删除的商品");
        return;
    }

    int productId = m_cartModel->item(index.row(), 0)->data(Qt::UserRole).toInt();
    for (auto it = m_cartItems.begin(); it != m_cartItems.end(); ++it) {
        if (it->product().id() == productId) {
            m_cartItems.erase(it);
            break;
        }
    }
    updateCartview();
}

