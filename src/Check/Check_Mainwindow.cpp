#include "Check/Check_Mainwindow.h"
#include "ui_Check_Mainwindow.h"

Check_Mainwindow::Check_Mainwindow(QString name,QWidget *parent)
    : QMainWindow(parent)
    , name(name)
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



    // 查找 productgroupBox 并检查有效性
    QGroupBox* productGroupBox = findChild<QGroupBox*>("productgroupBox");
    if (!productGroupBox) {
        qCritical() << "无法找到 productgroupBox!";
        return;
    }

    // 检查布局是否存在且有效
    QLayout* groupLayout = productGroupBox->layout();
    if (!groupLayout || groupLayout->count() == 0) {
        qCritical() << "productgroupBox 布局为空或无效!";
        return;
    }


    QWidget* leftWidget = findChild<QWidget*>("leftWidget");       // 左侧按钮区
    QWidget* cartWidget = findChild<QWidget*>("cartWidget");       // 中间购物车区
    categoryWidget = findChild<QWidget*>("categoryWidget");        //分类按钮


    //设置按钮区
    QVBoxLayout* leftLayout = qobject_cast<QVBoxLayout*>(leftWidget->layout());
    if (leftLayout) {
        leftLayout->addStretch(); // 按钮居上，下方留白
    }


    //设置购物车区
    QVBoxLayout* cartLayout = qobject_cast<QVBoxLayout*>(cartWidget->layout());
    if (cartLayout) {
        // 若总价区域已在 .ui 设好水平布局，无需重复创建
        // 如需微调，可在这里 addWidget 或调整 stretch
    }


    //设置商品区分类按钮+列表
    QWidget* categoryWidget = findChild<QWidget*>("categoryWidget");
    QHBoxLayout* categoryLayout = qobject_cast<QHBoxLayout*>(categoryWidget->layout());
    if (categoryLayout) {
        for (QPushButton* btn : m_categoryButtons) {
            categoryLayout->addWidget(btn);
        }
    }
    ui->productlistWidget->setDragDropMode(QAbstractItemView::NoDragDrop); //           禁用商品区拖放
    ui->productlistWidget->setSelectionMode(QAbstractItemView::SingleSelection); //     限制商品区单选






    //连接信号与槽
    connect(ui->productlistWidget, &QListWidget::itemDoubleClicked,
            this, &Check_Mainwindow::chooselistWidgetitemDoubleClicked);

    connect(ui->changebtn, &QPushButton::clicked,
            this, &Check_Mainwindow::changebtnclicked);

    connect(ui->deletebtn, &QPushButton::clicked,
            this, &Check_Mainwindow::deletebtnclicked);

    connect(ui->clearbtn, &QPushButton::clicked,
            this, &Check_Mainwindow::clearbtnclicked);
    connect(ui->paybtn, &QPushButton::clicked,
            this, &Check_Mainwindow::paybtnclicked);

    if (!ui) {
        qCritical() << "ui 未初始化!";
        return;
    }
    if (!ui->cartlistView || !ui->changebtn || !ui->deletebtn) {
        qCritical() << "UI 控件缺失，请检查 .ui 文件!";
        return;
    }

}


//获取所有商品分类
QStringList Check_Mainwindow::getcategory() {
    QStringList categories;
    categories << "全部";              //默认显示全部

    // 调用 DBManager的getProductsByCategory接口,传空字符串获取所有分类商品
    QList<QMap<QString, QVariant>> products = DBManager::instance().getProductsByCategory("");
    QSet<QString> categorySet;                                                  // 用集合去重
    for (const auto& product : products) {
        QString category = product["category"].toString();                      //历遍商品，把category字段的值提取出来添加到categorySet集合中
        if (!category.isEmpty()) {
            categorySet.insert(category);
        }
    }
    // 将集合转成字符串列表
    for (const QString& category : categorySet) {                               //手动将 QSet 转换为 QStringList
        categories.append(category);
    }
    return categories;
}


//获取商品分类并创建分类按钮
void Check_Mainwindow::initcategory(){

    QStringList categories = getcategory();                                    //查找布局
    QHBoxLayout* categoriesLayout = findChild<QHBoxLayout*>("categoryButtonsLayout");
    if (!categoriesLayout) {
        qCritical() << "未找到分类按钮布局 categoryButtonsLayout!";
        return;
    }


    for (const QString& category : categories) {
        QPushButton* btn = new QPushButton(category,categoryWidget);
        btn->setCheckable(true);                                //设置为单选按钮
        if (category == "全部") btn->setChecked(true);          //默认选中“全部”分类
        m_categoryButtons.push_back(btn);                      //把按钮存入容器
        categoriesLayout->addWidget(btn);                      //把每个按钮添加到布局中

        // 连接按钮点击事件
        connect(btn, &QPushButton::clicked, this, [this, btn]() {      //实现单选效果：遍历所有按钮，只选中当前点击的按钮
            for (QPushButton* otherBtn : m_categoryButtons) {
                otherBtn->setChecked(otherBtn == btn);
            }
            updateProduct(btn->text());                                //更新商品列表
        });
    }

    // 初始显示全部商品
    updateProduct();

}


//根据不同分类来更新商品列表
void Check_Mainwindow::updateProduct(const QString& category){
    ui->productlistWidget->clear();
    QList<QMap<QString, QVariant>> products;

    if (category == "全部") {
        products = DBManager::instance().getAllProducts();                //全部商品，调用getAllProducts接口
    } else {
        products = DBManager::instance().getProductsByCategory(category); //指定分类，调用getProductsByCategory接口
    }

    for (const auto& productData : products) {
        PRoduct product(
            productData["id"].toInt(),
            productData["name"].toString(),
            "",
            productData["price"].toDouble(),
            productData["stock"].toInt(),
            productData["category"].toString()
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
        const PRoduct& product=item.product();                                        //获取商品信息

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
void Check_Mainwindow::chooselistWidgetitemDoubleClicked(QListWidgetItem *item){
    if (!item) return;                                                 //确保选中有效商品

    int productId = item->data(Qt::UserRole).toInt();                  //获取选中的商品ID

    QMap<QString, QVariant> productData = DBManager::instance().getProductById(productId);      //调用getProductById接口查询商品详情
    if (productData.isEmpty()) {
        QMessageBox::warning(this, "错误", "商品信息不存在");
        return;
    }

    PRoduct product(
        productId,
        productData["name"].toString(),
        "",
        productData["price"].toDouble(),
        productData["stock"].toInt()
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


//改数键
void Check_Mainwindow::changebtnclicked()
{   QModelIndex index=ui->cartlistView->currentIndex();                                 //获取购物车项目索引
    if (!index.isValid()) {                                                             //如果没有索引就给出提示
        QMessageBox::warning(this, "提示", "请先选择要修改的商品");
        return;
    }

    int row = index.row();                                                              //获取行号，商品ID和商品数量
    int productId=m_cartModel->item(row, 0)->data(Qt::UserRole).toInt();
    int currentQuantity=m_cartModel->item(row, 2)->text().toInt();

    QMap<QString, QVariant> productData = DBManager::instance().getProductById(productId);  //调用getProductById接口查询商品库存
    if (productData.isEmpty()) return;
    int maxStock = productData["stock"].toInt();

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



//删除键
void Check_Mainwindow::deletebtnclicked()
{   QModelIndex index=ui->cartlistView->currentIndex();                             //获取选中项的模型索引
    if (!index.isValid()) {                                                         //检查索引是否有效
        QMessageBox::warning(this, "提示", "请先选择要删除的商品");
        return;
    }

    int productId = m_cartModel->item(index.row(), 0)->data(Qt::UserRole).toInt();   //通过索引获取商品ID
    for (auto it = m_cartItems.begin(); it != m_cartItems.end(); ++it) {             //历遍购物车，检查获得的商品ID是否与要删除的商品ID一致
        if (it->product().id() == productId) {
            m_cartItems.erase(it);
            break;
        }
    }
    updateCartview();                                                                 //更新购物车
}



//清空键
void Check_Mainwindow::clearbtnclicked()
{
    if (m_cartItems.empty()) {                                                         //检查购物车是否为空
        QMessageBox::information(this, "提示", "购物车已经是空的");
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(                         //确认对话框
        this, "确认清空", "确定要清空购物车中的所有商品吗？",
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply == QMessageBox::Yes) {

        m_cartItems.clear();

        updateCartview();                                                                //更新购物车显示

        QMessageBox::information(this, "成功", "购物车已清空");
    }
}


//支付键
void Check_Mainwindow::paybtnclicked()
{
    if (m_cartItems.empty()) {
        QMessageBox::information(this, "提示", "购物车为空，无需支付");
        return;
    }

    // 计算总金额
    double total = 0;
    for (const auto &item : m_cartItems) {
        total += item.getTotalPrice();
    }


//    // 会员信息初始化
//    QString memberPhone = ui->memberPhoneEdit->text().trimmed(); // 假设有会员输入框
//    double finalDiscount = 1.0; // 默认无折扣
//    int pointsEarned = static_cast<int>(total); // 1元=1积分
//    bool isBirthday = false; // 生日标志

//    QString discountInfo = ""; // 折扣信息详情

//    if (!memberPhone.isEmpty()) {
//        // 获取会员信息
//        auto member = DBManager::instance().getMemberByPhone(memberPhone);

//        // 检查是否是生日
//        QString birthdayStr = member["birthday"].toString();
//        QDate birthday = QDate::fromString(birthdayStr, "yyyy-MM-dd");
//        QDate today = QDate::currentDate();

//        // 生日折扣（85折）
//        if (birthday.month() == today.month() && birthday.day() == today.day()) {
//            finalDiscount = 0.85; // 生日折扣
//            isBirthday = true;
//            discountInfo += "生日折扣: 85折\n";
//        }

//        // 如果不是生日，使用其他会员折扣
//        if (!isBirthday) {
//            // 获取当前折扣
//            double baseDiscount = DBManager::instance().getMemberDiscount(memberPhone);

//            // 获取当前积分
//            int currentPoints = DBManager::instance().getMemberPoints(memberPhone);

//            // 计算积分折扣
//            double pointsDiscount = DBManager::instance().calculateDiscountByPoints(currentPoints);

//            // 应用最优惠折扣
//            finalDiscount = qMin(baseDiscount, pointsDiscount);

//            // 记录折扣详情
//            if (baseDiscount < 1.0) {
//                discountInfo += QString("会员折扣: %1%\n").arg(baseDiscount * 100);
//            }
//            if (pointsDiscount < 1.0) {
//                discountInfo += QString("积分折扣: %1%\n").arg(pointsDiscount * 100);
//            }
//        }

//        // 更新会员积分（生日当天积分双倍）
//        int pointsToAdd = isBirthday ? pointsEarned * 2 : pointsEarned;
//        DBManager::instance().updateMemberPoints(memberPhone,
//                                                 DBManager::instance().getMemberPoints(memberPhone) + pointsToAdd);
//    }

//    // 应用折扣
//    double originalTotal = total;
//    double finalTotal = total * finalDiscount;
//    double savedAmount = originalTotal - finalTotal;

//    // 构建支付信息
//    QString message = QString("支付金额: %1元").arg(finalTotal, 0, 'f', 2);

//    // 如果有折扣
//    if (finalDiscount < 1.0) {
//        message += QString("\n原价: %1元").arg(originalTotal, 0, 'f', 2);
//        message += QString("\n节省: %1元").arg(savedAmount, 0, 'f', 2);

//        // 添加折扣信息
//        if (!discountInfo.isEmpty()) {
//            message += "\n\n折扣详情:\n" + discountInfo;
//        }
//    }

//    // 如果有会员
//    if (!memberPhone.isEmpty()) {
//        message += QString("\n获得积分: %1").arg(pointsEarned);

//        // 如果是生日，显示额外积分
//        if (isBirthday) {
//            message += QString(" (生日双倍积分，实际获得: %1)").arg(pointsEarned * 2);
//        }
//    }

//    // 显示支付信息
//    if (isBirthday) {
//        QMessageBox::information(this, "生日快乐！支付成功", message);
//    } else {
//        QMessageBox::information(this, "支付成功", message);
//    }
    QMessageBox::information(this, "支付成功","");

    // 清空购物车
    m_cartItems.clear();
    updateCartview();
}
