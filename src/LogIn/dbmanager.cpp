#include "LogIn/dbmanager.h"

// 单例模式实现
DBManager::DBManager(QObject *parent) : QObject(parent)
{
    // 注册SQLite驱动
    QSqlDatabase::addDatabase("QSQLITE");
}

DBManager::~DBManager()
{
    if (m_database.isOpen()) {
        m_database.close();
    }
}

DBManager& DBManager::instance()
{
    static DBManager instance;
    return instance;
}

bool DBManager::initialize(const QString& path)
{
    m_database = QSqlDatabase::database();
    m_database.setDatabaseName(path);

    if (!m_database.open()) {
        qCritical() << "无法打开数据库:" << m_database.lastError().text();
        return false;
    }

    // 启用外键支持
    QSqlQuery query("PRAGMA foreign_keys = ON;");
    if (!query.exec()) {
        qWarning() << "无法启用外键支持:" << query.lastError().text();
    }

    // 创建表结构
    if (!createTables()) {
        qCritical() << "创建数据库表失败";
        return false;
    }

    // 初始化默认数据
    if (!initDefaultData()) {
        qWarning() << "初始化默认数据失败";
    }

    qInfo() << "数据库初始化成功:" << path;


    return true;
}

bool DBManager::createTables()
{
    bool success = true;

    // 用户表
    success &= executeTransaction(
        "CREATE TABLE IF NOT EXISTS users ("
        "   id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "   username TEXT UNIQUE NOT NULL,"
        "   password TEXT NOT NULL,"
        "   role TEXT CHECK(role IN ('admin', 'cashier')),"
        "   gender TEXT,"
        "   age INTEGER)"
        );

    // 商品表
    success &= executeTransaction(
        "CREATE TABLE IF NOT EXISTS products ("
        "   id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "   name TEXT NOT NULL,"
        "   barcode TEXT UNIQUE,"
        "   price REAL NOT NULL CHECK(price >= 0),"
        "   stock INTEGER DEFAULT 0 CHECK(stock >= 0),"
        "   category TEXT,"
        "   monthly_sales INTEGER DEFAULT 0 CHECK(monthly_sales >= 0))"  // 新增月销量字段

        );

    // 会员表
    success &= executeTransaction(
        "CREATE TABLE IF NOT EXISTS members ("
        "   id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "   phone TEXT UNIQUE NOT NULL,"
        "   name TEXT,"
        "   discount REAL DEFAULT 1.0 CHECK(discount BETWEEN 0.1 AND 1.0),"
        "   birthday TEXT,"  // 新增生日字段
        "   points INTEGER DEFAULT 0)"
    );

    // 销售记录表
    success &= executeTransaction(
        "CREATE TABLE IF NOT EXISTS sales ("
        "   id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "   cashier_id INTEGER NOT NULL REFERENCES users(id),"
        "   total REAL NOT NULL CHECK(total >= 0),"
        "   payment REAL NOT NULL CHECK(payment >= 0),"
        "   member_phone TEXT REFERENCES members(phone),"  // 新增会员字段
        "   timestamp DATETIME DEFAULT CURRENT_TIMESTAMP)"
        );

    // 销售明细表
    success &= executeTransaction(
        "CREATE TABLE IF NOT EXISTS sale_items ("
        "   sale_id INTEGER NOT NULL REFERENCES sales(id) ON DELETE CASCADE,"
        "   product_id INTEGER NOT NULL REFERENCES products(id),"
        "   quantity INTEGER NOT NULL CHECK(quantity > 0),"
        "   price REAL NOT NULL CHECK(price >= 0))"
        );

    return success;
}

bool DBManager::initDefaultData()
{
    // 创建默认管理员账户
    if (!executeQuery("SELECT 1 FROM users WHERE username = 'admin'").next()) {
        createUser("admin", "admin123", "admin","",0);
        qInfo() << "创建默认管理员账户: admin/admin123";
    }

    // 添加示例商品
    QSqlQuery result = executeQuery("SELECT COUNT(*) FROM products");
    if (result.next() && result.value(0).toInt() == 0) {
        addProduct("可口可乐", "695476741231", 3.5, 100, "饮料");
        addProduct("乐事薯片", "692345065618", 8.5, 50, "零食");
        addProduct("金龙鱼大米", "692880401016", 65.0, 30, "粮油");
        qInfo() << "添加了示例商品数据";
    }

    return true;
}

bool DBManager::createUser(const QString& username, const QString& password, const QString& role, const QString& gender, const int& age)
{
    QVariantList params;
    params << username << encryptPassword(password) << role <<gender<<age;

    return executeTransaction(
        "INSERT INTO users (username, password, role, gender, age) "
        "VALUES (?, ?, ?, ?, ?)",
        params
        );
}

QString DBManager::authenticateUser(const QString& username, const QString& password)
{
    QSqlQuery query = executeQuery(
        "SELECT password, role FROM users WHERE username = ?",
        {username}
        );

    if (query.next()) {
        QString storedHash = query.value(0).toString();
        QString inputHash = encryptPassword(password);

        if (storedHash == inputHash) {
            return query.value(1).toString();
        }
    }

    return "";
}

bool DBManager::addProduct(const QString& name, const QString& barcode, double price, int stock, const QString& category)
{
    QVariantList params;
    params << name << barcode << price << stock << category;

    return executeTransaction(
        "INSERT INTO products (name, barcode, price, stock, category) "
        "VALUES (?, ?, ?, ?, ?)",
        params
        );
}

bool DBManager::updateProductStock(int productId, int delta)
{
    QVariantList params;
    params << delta << productId;

    return executeTransaction(
        "UPDATE products SET stock = stock + ? WHERE id = ?",
        params
        );
}

// 新增：按关键词搜索商品（名称或条码）
QList<QMap<QString, QVariant>> DBManager::searchProducts(const QString& keyword)
{
    QSqlQuery query = executeQuery(
        "SELECT id, name, barcode, price, stock, category FROM products "
        "WHERE name LIKE ? OR barcode LIKE ?",
        {"%" + keyword + "%", "%" + keyword + "%"}
        );

    QList<QMap<QString, QVariant>> results;
    while (query.next()) {
        QMap<QString, QVariant> record;
        record["id"] = query.value("id");
        record["name"] = query.value("name");
        record["barcode"] = query.value("barcode");
        record["price"] = query.value("price");
        record["stock"] = query.value("stock");
        record["category"] = query.value("category");
        results.append(record);
    }
    return results;
}

bool DBManager::addMember(const QString& phone, const QString& name,
                          double discount, const QString& birthday,
                          int points)
{
    QVariantList params;
    params << phone << name << discount << birthday << points;

    return executeTransaction(
        "INSERT INTO members (phone, name, discount, birthday, points) "
        "VALUES (?, ?, ?, ?, ?)",
        params
    );
}

bool DBManager::updateMemberPoints(const QString& phone, int points)
{
    QVariantList params;
    params << points << phone;

    return executeTransaction(
        "UPDATE members SET points = ? WHERE phone = ?",
        params
    );
}

int DBManager::getMemberPoints(const QString& phone)
{
    QSqlQuery query = executeQuery(
        "SELECT points FROM members WHERE phone = ?",
        {phone}
    );

    if (query.next()) {
        return query.value(0).toInt();
    }
    return 0;
}

double DBManager::calculateDiscountByPoints(int points)
{
    // 积分折扣规则:
    // 0-1000分: 无折扣
    // 1001-5000分: 95折
    // 5001-10000分: 9折
    // 10001分以上: 85折
    if (points > 10000) return 0.85;
    if (points > 5000) return 0.9;
    if (points > 1000) return 0.95;
    return 1.0;
}

double DBManager::getMemberDiscount(const QString& phone)
{
    QSqlQuery query = executeQuery(
        "SELECT discount FROM members WHERE phone = ?",
        {phone}
        );

    if (query.next()) {
        return query.value(0).toDouble();
    }

    return 1.0; // 默认无折扣
}

bool DBManager::executeTransaction(const QString& sql, const QVariantList& params)
{
    QSqlQuery query;
    query.prepare(sql);

    for (int i = 0; i < params.size(); ++i) {
        query.bindValue(i, params[i]);
    }

    if (!query.exec()) {
        qWarning() << "SQL错误:" << query.lastError().text();
        qWarning() << "SQL语句:" << sql;
        return false;
    }

    return true;
}

QSqlQuery DBManager::executeQuery(const QString& sql, const QVariantList& params)
{
    QSqlQuery query;
    query.prepare(sql);

    for (int i = 0; i < params.size(); ++i) {
        query.bindValue(i, params[i]);
    }

    if (!query.exec()) {
        qWarning() << "SQL查询错误:" << query.lastError().text();
        qWarning() << "SQL语句:" << sql;
    }

    return query;
}

bool DBManager::isOpen() const
{
    return m_database.isOpen();
}

QString DBManager::lastError() const
{
    return m_database.lastError().text();
}

QString DBManager::encryptPassword(const QString& password)
{
    return QCryptographicHash::hash(password.toUtf8(),
                                    QCryptographicHash::Sha256).toHex();
}

// ================= 商品查询实现 =================
QList<QMap<QString, QVariant>> DBManager::getAllProducts() {
    QSqlQuery query = executeQuery(
        "SELECT id, name, barcode, price, stock, category, monthly_sales FROM products"
        );

    QList<QMap<QString, QVariant>> results;
    while (query.next()) {
        QMap<QString, QVariant> record;
        record["id"] = query.value("id");
        record["name"] = query.value("name");
        record["barcode"] = query.value("barcode");
        record["price"] = query.value("price");
        record["stock"] = query.value("stock");
        record["category"] = query.value("category");
        record["monthly_sales"] = query.value("monthly_sales");  // 新增
        results.append(record);
    }
    if (query.lastError().isValid()) {
        qCritical() << "查询错误：" << query.lastError().text();
    } else {
        qDebug() << "查询成功，返回记录数：" << results.size();
    }
    return results;
}

QMap<QString, QVariant> DBManager::getProductById(int productId) {
    QSqlQuery query = executeQuery(
        "SELECT id, name, barcode, price, stock, category, monthly_sales FROM products WHERE id = ?",
        {productId}
        );

    QMap<QString, QVariant> record;
    if (query.next()) {
        record["id"] = query.value("id");
        record["name"] = query.value("name");
        record["barcode"] = query.value("barcode");
        record["price"] = query.value("price");
        record["stock"] = query.value("stock");
        record["category"] = query.value("category");
        record["monthly_sales"] = query.value("monthly_sales");  // 新增
    }
    return record;
}

QList<QMap<QString, QVariant>> DBManager::getProductsByName(const QString& name) {
    QSqlQuery query = executeQuery(
        "SELECT id, name, barcode, price, stock, category, monthly_sales FROM products "
        "WHERE name LIKE ?",
        {"%" + name + "%"}
        );

    QList<QMap<QString, QVariant>> results;
    while (query.next()) {
        QMap<QString, QVariant> record;
        record["id"] = query.value("id");
        record["name"] = query.value("name");
        record["barcode"] = query.value("barcode");
        record["price"] = query.value("price");
        record["stock"] = query.value("stock");
        record["category"] = query.value("category");
        record["monthly_sales"] = query.value("monthly_sales");  // 新增
        results.append(record);
    }
    return results;
}

QList<QMap<QString, QVariant>> DBManager::getProductsByCategory(const QString& category) {
    QSqlQuery query = executeQuery(
        "SELECT id, name, barcode, price, stock, category, monthly_sales FROM products "
        "WHERE category = ?",
        {category}
        );

    QList<QMap<QString, QVariant>> results;
    while (query.next()) {
        QMap<QString, QVariant> record;
        record["id"] = query.value("id");
        record["name"] = query.value("name");
        record["barcode"] = query.value("barcode");
        record["price"] = query.value("price");
        record["stock"] = query.value("stock");
        record["category"] = query.value("category");
        record["monthly_sales"] = query.value("monthly_sales");  // 新增
        results.append(record);
    }
    return results;
}

// ================= 会员查询实现 =================
QList<QMap<QString, QVariant>> DBManager::getAllMembers() {

    QSqlQuery query = executeQuery(
        "SELECT id, phone, name, discount, gender, age, points FROM members"
    );

    QList<QMap<QString, QVariant>> results;
    while (query.next()) {
        QMap<QString, QVariant> record;
        record["id"] = query.value("id");
        record["phone"] = query.value("phone");
        record["name"] = query.value("name");
        record["discount"] = query.value("discount");
        record["gender"] = query.value("gender");
        record["age"] = query.value("age");
        record["points"] = query.value("points");
        results.append(record);
    }
    return results;
}

QMap<QString, QVariant> DBManager::getMemberByPhone(const QString& phone)
{
    QSqlQuery query = executeQuery(
        "SELECT id, phone, name, discount, birthday, points FROM members WHERE phone = ?",  // 添加birthday
        {phone}
    );

    QMap<QString, QVariant> record;
    if (query.next()) {
        record["id"] = query.value("id");
        record["phone"] = query.value("phone");
        record["name"] = query.value("name");
        record["discount"] = query.value("discount");
        record["birthday"] = query.value("birthday");  // 新增
        record["points"] = query.value("points");
    }
    return record;
}

QList<QMap<QString, QVariant>> DBManager::getMembersByName(const QString& name) {
    QSqlQuery query = executeQuery(
        "SELECT id, phone, name, discount, birthday, points FROM members "
        "WHERE name LIKE ?",
        {"%" + name + "%"}
        );

    QList<QMap<QString, QVariant>> results;

    // 遍历查询结果
    while (query.next()) {
        QMap<QString, QVariant> record;
        record["id"] = query.value("id");
        record["phone"] = query.value("phone");
        record["name"] = query.value("name");
        record["discount"] = query.value("discount");
        record["birthday"] = query.value("birthday");  // 添加生日字段
        record["points"] = query.value("points");
        results.append(record);
    }

    return results;  // 返回结果列表
}

bool DBManager::authenticateAdmin(const QString& adminUsername, const QString& adminPassword)
{
    QSqlQuery query = executeQuery(
        "SELECT password, role FROM users WHERE username = ?",
        {adminUsername}
    );

    if (query.next()) {
        QString storedHash = query.value(0).toString();
        QString inputHash = encryptPassword(adminPassword);
        QString role = query.value(1).toString();

        // 验证密码和角色
        if (storedHash == inputHash && role == "admin") {
            return true;
        }
    }

    return false;
}

bool DBManager::registerUser(const QString& username, const QString& password, const QString& gender, const int& age, const QString& role)
{
    // 检查用户名是否已存在
    QSqlQuery checkQuery = executeQuery(
        "SELECT COUNT(*) FROM users WHERE username = ?",
        {username}
    );

    if (checkQuery.next() && checkQuery.value(0).toInt() > 0) {
        qWarning() << "用户名已存在:" << username;
        return false;
    }

    // 创建新用户
    return createUser(username, password, role,gender,age);
}

bool DBManager::deleteProduct(int productId)
{
    // 先检查该商品是否有销售记录
    QSqlQuery checkQuery = executeQuery(
        "SELECT COUNT(*) FROM sale_items WHERE product_id = ?",
        {productId}
    );

    if (checkQuery.next() && checkQuery.value(0).toInt() > 0) {
        qWarning() << "无法删除商品: 该商品已有销售记录";
        return false;
    }

    return executeTransaction(
        "DELETE FROM products WHERE id = ?",
        {productId}
    );
}
bool DBManager::updateProductPrice(int productId, double newPrice)
{
    // 验证价格非负
    if (newPrice < 0) {
        qWarning() << "价格不能为负数";
        return false;
    }

    QVariantList params;
    params << newPrice << productId;

    return executeTransaction(
        "UPDATE products SET price = ? WHERE id = ?",
        params
    );
}

QList<QMap<QString, QVariant>> DBManager::getMonthlyProductSales()
{
    // 获取当前年月
    QDate currentDate = QDate::currentDate();
    int year = currentDate.year();
    int month = currentDate.month();

    // 查询当月销量数据
    QString sql =
        "SELECT p.id, p.name, SUM(si.quantity) AS total_sales "
        "FROM products p "
        "LEFT JOIN sale_items si ON si.product_id = p.id "
        "LEFT JOIN sales s ON s.id = si.sale_id "
        "WHERE strftime('%Y', s.timestamp) = ? "
        "  AND strftime('%m', s.timestamp) = ? "
        "GROUP BY p.id, p.name "
        "ORDER BY total_sales DESC";

    QVariantList params;
    params << QString::number(year) << QString::number(month).rightJustified(2, '0');

    QSqlQuery query = executeQuery(sql, params);

    QList<QMap<QString, QVariant>> results;
    while (query.next()) {
        QMap<QString, QVariant> record;
        record["id"] = query.value("id");
        record["name"] = query.value("name");
        record["total_sales"] = query.value("total_sales");
        results.append(record);
    }

    // 如果没有销售记录，显示所有商品（销量为0）
    if (results.isEmpty()) {
        QSqlQuery allProducts = executeQuery(
            "SELECT id, name, 0 AS total_sales FROM products"
        );

        while (allProducts.next()) {
            QMap<QString, QVariant> record;
            record["id"] = allProducts.value("id");
            record["name"] = allProducts.value("name");
            record["total_sales"] = 0;
            results.append(record);
        }
    }

    return results;
}
//=====================添加销售记录与细明===========================
bool DBManager::addSale(int cashierId, double total, double payment,
                        const QList<QVariantMap>& items,
                        const QString& memberPhone)
{
    // 开启事务
    m_database.transaction();

    try {
        // 1. 添加销售记录
        QSqlQuery saleQuery;
        saleQuery.prepare(
            "INSERT INTO sales (cashier_id, total, payment, member_phone) "
            "VALUES (?, ?, ?, ?)"
        );
        saleQuery.addBindValue(cashierId);
        saleQuery.addBindValue(total);
        saleQuery.addBindValue(payment);
        saleQuery.addBindValue(memberPhone.isEmpty() ? QVariant() : memberPhone);

        if (!saleQuery.exec()) {
            throw std::runtime_error("添加销售记录失败: " + saleQuery.lastError().text().toStdString());
        }

        // 获取新插入的销售记录ID
        int saleId = saleQuery.lastInsertId().toInt();
        if (saleId <= 0) {
            throw std::runtime_error("获取销售ID失败");
        }

        // 2. 添加销售明细
        for (const auto& item : items) {
            int productId = item["product_id"].toInt();
            int quantity = item["quantity"].toInt();
            double price = item["price"].toDouble();

            QSqlQuery itemQuery;
            itemQuery.prepare(
                "INSERT INTO sale_items (sale_id, product_id, quantity, price) "
                "VALUES (?, ?, ?, ?)"
            );
            itemQuery.addBindValue(saleId);
            itemQuery.addBindValue(productId);
            itemQuery.addBindValue(quantity);
            itemQuery.addBindValue(price);

            if (!itemQuery.exec()) {
                throw std::runtime_error("添加销售明细失败: " + itemQuery.lastError().text().toStdString());
            }

            // 3. 更新商品月销量
            QSqlQuery updateSalesQuery;
            updateSalesQuery.prepare(
                "UPDATE products SET monthly_sales = monthly_sales + ? WHERE id = ?"
            );
            updateSalesQuery.addBindValue(quantity);
            updateSalesQuery.addBindValue(productId);

            if (!updateSalesQuery.exec()) {
                throw std::runtime_error("更新商品月销量失败: " + updateSalesQuery.lastError().text().toStdString());
            }
        }

        // 4. 更新会员积分（如果有会员）
        if (!memberPhone.isEmpty()) {
            int pointsEarned = static_cast<int>(total); // 1元=1积分

            QSqlQuery pointsQuery;
            pointsQuery.prepare(
                "UPDATE members SET points = points + ? WHERE phone = ?"
            );
            pointsQuery.addBindValue(pointsEarned);
            pointsQuery.addBindValue(memberPhone);

            if (!pointsQuery.exec()) {
                throw std::runtime_error("更新会员积分失败: " + pointsQuery.lastError().text().toStdString());
            }
        }

        // 提交事务
        m_database.commit();
        return true;
    } catch (const std::exception& e) {
        // 回滚事务
        m_database.rollback();
        qCritical() << "销售记录添加失败:" << e.what();
        return false;
    }
}

int DBManager::getUserIdByName(const QString& username)
{
    QSqlQuery query = executeQuery(
        "SELECT id FROM users WHERE username = ?",
        {username}
    );

    if (query.next()) {
        return query.value(0).toInt();
    }
    return -1; // 未找到用户
}

void DBManager::resetmonthlysale(){
    DBManager::instance().executeTransaction(
        "UPDATE products SET monthly_sales = 0"
    );
    qInfo() << "Monthly sales data has been reset";
}

QList<QMap<QString, QVariant>> DBManager::getSalesHistory(int cashierId, const QDateTime& start, const QDateTime& end) {
    QString sql = "SELECT s.id, s.timestamp, s.total, s.payment, s.member_phone, "
                  "GROUP_CONCAT(p.name || '×' || si.quantity, ', ') AS items "
                  "FROM sales s "
                  "JOIN sale_items si ON s.id = si.sale_id "
                  "JOIN products p ON si.product_id = p.id "
                  "WHERE s.cashier_id = ? ";

    QVariantList params = {cashierId};

    if (start.isValid() && end.isValid()) {
        sql += "AND s.timestamp BETWEEN ? AND ? ";
        params << start << end;
    }

    sql += "GROUP BY s.id ORDER BY s.timestamp DESC";

    QSqlQuery query = executeQuery(sql, params);

    QList<QMap<QString, QVariant>> results;
    while (query.next()) {
        QMap<QString, QVariant> record;
        record["id"] = query.value("id");
        record["timestamp"] = query.value("timestamp");
        record["total"] = query.value("total");
        record["payment"] = query.value("payment");
        record["member_phone"] = query.value("member_phone");
        record["items"] = query.value("items");
        results.append(record);
    }
    return results;
}

QMap<QString, QVariant> DBManager::getProductByName(const QString& name) {
    QMap<QString, QVariant> product;
    QSqlQuery query;
    query.prepare("SELECT * FROM products WHERE name = :name LIMIT 1");
    query.bindValue(":name", name);

    if (query.exec() && query.next()) {
        product["id"] = query.value("id");
        product["name"] = query.value("name");
        product["stock"] = query.value("stock");
        // 其他字段按需添加
    }
    return product;
}
