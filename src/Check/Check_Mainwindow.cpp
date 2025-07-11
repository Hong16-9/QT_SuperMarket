#include "Check/Check_Mainwindow.h"
#include "ui_Check_Mainwindow.h"
#include "LogIn/LoginDialog.h"

Check_Mainwindow::Check_Mainwindow(QString username,QWidget *parent)
    : QMainWindow(parent)
    , username(username)
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
        ui->cartlistView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);   //设置购物车区域商品条目占满视图宽度
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
    connect(ui->productlistWidget, &QListWidget::itemDoubleClicked,this, &Check_Mainwindow::chooselistWidgetitemDoubleClicked);
    connect(ui->changebtn, &QPushButton::clicked,this, &Check_Mainwindow::changebtnclicked);
    connect(ui->deletebtn, &QPushButton::clicked,this, &Check_Mainwindow::deletebtnclicked);
    connect(ui->clearbtn, &QPushButton::clicked,this, &Check_Mainwindow::clearbtnclicked);
    connect(ui->paybtn, &QPushButton::clicked,this, &Check_Mainwindow::paybtnclicked);
    connect(ui->backbtn,&QPushButton::clicked,this,&Check_Mainwindow::backbtnclicked);

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
    categories << "全部";                         //默认显示全部


    //调用getAllProducts() 获取所有商品的ID
    QList<QMap<QString, QVariant>> allProducts = DBManager::instance().getAllProducts();
    QSet<int> productIds;                                 //存储所有商品的ID同时去重
    for (const auto& product : allProducts) {
        productIds.insert(product["id"].toInt());
    }

    //遍历每个商品ID，调用getProductById获取分类
    QSet<QString> categorySet;  //分类的去重
    for (int productId : productIds) {
        QMap<QString, QVariant> product = DBManager::instance().getProductById(productId);  //调用getProductById 获取单个商品信息
        if (product.isEmpty()) continue;              //跳过无效商品

        QString category = product["category"].toString();
        if (!category.isEmpty()) {                    //只收集非空分类
            categorySet.insert(category);
        }
    }

    //将去重后的分类添加到列表
    for (const QString& category : categorySet) {
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
            productData["category"].toString(),
            productData["monthly_sales"].toInt()

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
    QuantityDialog dialog(product.stock(), this);
    if (dialog.exec() != QDialog::Accepted) {
        return; //用户取消选择，直接返回
    }
    int addQuantity = dialog.getQuantity();  //新添商品数量

    //检查购物车中该商品的已有数量
    int existingQuantity = 0;
    bool found = false;
    for (const auto& cartItem : m_cartItems) {
        if (cartItem.product().id() == productId) {
            existingQuantity = cartItem.quantity();
            found = true;
            break;
        }
    }

    //计算已有数量+新添数量是否超过库存，超过就return
    if (existingQuantity + addQuantity > product.stock()) {
        QMessageBox::warning(this, "库存不足",
                             QString("当前库存仅%1个，无法添加%2个（购物车中已有%3个）")
                                 .arg(product.stock())
                                 .arg(addQuantity)
                                 .arg(existingQuantity));
        return; //不执行添加操作
    }

    //若未超过库存，则更新购物车
    if (found) {
        //若购物车中已有该商品，就更新数量
        for (auto& cartItem : m_cartItems) {
            if (cartItem.product().id() == productId) {
                cartItem.setQuantity(existingQuantity + addQuantity);
                break;
            }
        }
    } else {
        //若购物车中没有该商品，就新加商品
        m_cartItems.emplace_back(product, addQuantity);
    }

    updateCartview(); // 刷新购物车显示


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
    //检查购物车是否为空
    if (m_cartItems.empty()) {
        QMessageBox::information(this, "提示", "购物车为空，无需支付");
        return;
    }


    // 计算总金额
    double total = 0;
    for (const auto &item : m_cartItems) {
        total += item.getTotalPrice();
    }


    //会员折扣处理

    QString memberPhone;
    double discount = 1.0; // 默认无折扣
    int pointsEarned = static_cast<int>(total); // 1元=1积分
    bool isBirthday = false; //生日标志
    bool hasmember = false;  //会员标志

    //弹出对话框询问用户是否有会员
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "会员", "是否使用会员折扣?",
        QMessageBox::Yes | QMessageBox::No             //点击yes进入会员流程，点击no正常结算
        );

    if (reply == QMessageBox::Yes) {

        bool ok;
        memberPhone = QInputDialog::getText(                 //储存用户输入的手机号
            this, "会员", "请输入会员手机号:",
            QLineEdit::Normal, "", &ok
            );
        if (ok && !memberPhone.isEmpty()) {                  //点击ok且输入的手机号不是空的
            DBManager& db = DBManager::instance();
            QMap<QString, QVariant> member = db.getMemberByPhone(memberPhone);  //从数据库查询手机号信息（以判断是否是会员）

            //若会员存在则进行折扣计算
            if (!member.empty()) {
                hasmember = true;      //标记为有效会员

                // 生日判断（对比会员生日与当天的日期）
                QString birthdayStr = member["birthday"].toString();
                QDate birthday = QDate::fromString(birthdayStr, "yyyy-MM-dd");
                QDate today = QDate::currentDate();
                isBirthday = (birthday.month() == today.month() && birthday.day() == today.day());

                // 折扣计算：生日优先，再取会员折扣与积分折扣的最小值
                if (isBirthday) {
                    discount = 0.85; // 生日专属折扣
                } else {
                    double baseDiscount = db.getMemberDiscount(memberPhone);     //会员等级对应的基础折扣
                    int currentPoints = db.getMemberPoints(memberPhone);         //会员当前的积分
                    double pointsDiscount = db.calculateDiscountByPoints(currentPoints);      //积分对应的折扣
                    discount = qMin(baseDiscount, pointsDiscount);               //取基础折扣和积分折扣中的最优折扣
                }

                // 积分更新（生日双倍）
                int pointsToAdd = isBirthday ? pointsEarned * 2 : pointsEarned;                       //计算新积分
                db.updateMemberPoints(memberPhone, db.getMemberPoints(memberPhone) + pointsToAdd);    //累加积分到数据库

            }

            //若会员不存在则提示原价支付
            else {
                QMessageBox::warning(this, "提示", "该手机号不是会员，将按原价支付");
            }
        }
    }


    //计算折扣后最终金额
    double finalTotal = total * discount;            //计算应付金额
    double originalTotal = total;                    //保存原价（用于展示节省金额）
    double savedAmount = originalTotal - finalTotal; // 计算节省的金额

    //让用户输入实付金额
    bool ok;
    double payment = QInputDialog::getDouble(
        this, "支付",
        QString("应付金额: %1元\n请输入实付金额:").arg(finalTotal),        //显示应付金额
        finalTotal, 0, 1000000, 2, &ok                                   //默认值为应付金额，精度保留2位小数
        );

    //验证实付金额（取消输入或金额不足则支付失败）
    if (!ok || payment < finalTotal) {
        QMessageBox::warning(this, "支付失败", "实付金额不足或取消支付");
        return;
    }
    double change = payment - finalTotal;    //计算找零




    //将购物车商品转化为销售明细列表，准备插入数据库
    QList<QVariantMap> saleItems;
    for (const auto& cartItem : m_cartItems) {
        QVariantMap itemData;
        itemData["product_id"] = cartItem.product().id();      //商品ID
        itemData["quantity"] = cartItem.quantity();            //购买数量
        itemData["price"] = cartItem.product().price();        //商品单价（原价）
        saleItems.append(itemData);
    }


    //数据库事务处理（便于进行库存减扣）
    DBManager& db = DBManager::instance();
    QSqlDatabase sqlDb = QSqlDatabase::database();       //获取当前数据库连接
    sqlDb.transaction();                                 //手动开始事务
    bool success = true;                                 //标记操作成功与否


    //历遍购物车商品进行库存减扣
    for (const auto& cartItem : m_cartItems) {
        int productId = cartItem.product().id();
        int buyQuantity = cartItem.quantity();

        //直接使用SQL语句扣减库存，同时检查库存是否足够
        QSqlQuery query;
        query.prepare("UPDATE products SET stock = stock - ? WHERE id = ? AND stock >= ?");
        query.addBindValue(buyQuantity);              //减扣商品数量
        query.addBindValue(productId);                //商品ID
        query.addBindValue(buyQuantity);              //确保库存足够

        //执行SQL并检查结果
        if (!query.exec() || query.numRowsAffected() == 0) {
            success = false;
            qCritical() << "库存更新失败，可能库存不足:" << query.lastError().text();
            break;
        }
    }

    //如果库存扣减成功，继续调用addSale
    if (success) {
        success = db.addSale(
            1,
            finalTotal,
            payment,
            saleItems,
            hasmember ? memberPhone : ""           //是会员就记录手机号，不是就记为空
            );
    }

    //提交或回滚事务
    if (success && sqlDb.commit()) {
        // 构建支付成功信息
        QString message = QString("支付金额: %.2f元\n").arg(finalTotal);
        if (discount < 1.0) {
            message += QString("原价: %.2f元\n节省: %.2f元\n")
                           .arg(originalTotal).arg(savedAmount);
            if (hasmember) {
                message += isBirthday ? "折扣详情: 生日85折\n"
                                      : QString("折扣详情: 会员折扣(%.1f折)\n")
                                            .arg(discount * 10);
            }
        }
        if (hasmember) {
            message += QString("获得积分: %1").arg(pointsEarned);
            if (isBirthday) {
                message += " (生日双倍积分)";
            }
        }

            // 显示结果并刷新界面
            QMessageBox::information(
                this,
                isBirthday ? "生日快乐！支付成功" : "支付成功",    //会员生日与普通支付的提示
                message
                );
            m_cartItems.clear();         //清空购物车数据
            updateCartview();            //更新购物车界面
            updateProduct("全部");       //刷新商品列表
        }
    else {                                 //如果失败，回滚事务，并取消已执行SQL的操作
            sqlDb.rollback();
            QMessageBox::critical(this, "支付失败",
                                  success ? "添加销售记录失败" : "库存不足，支付已取消");
        }

}


void Check_Mainwindow::backbtnclicked(){
    //询问用户是否确认返回
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "确认返回", "返回登录界面将清空当前购物车，是否继续？",
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply == QMessageBox::Yes) {
        //清空购物车
        m_cartItems.clear();
        updateCartview();

        // 隐藏当前窗口
        this->close();

        // 创建并显示登录窗口
        LoginDialog* loginDialog = new LoginDialog();
        // 确保登录窗口关闭时释放资源
        loginDialog->setAttribute(Qt::WA_DeleteOnClose);
        // 显示登录窗口
        loginDialog->show();


    }
}
