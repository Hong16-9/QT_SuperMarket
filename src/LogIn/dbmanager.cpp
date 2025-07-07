// database/dbmanager.cpp
#include "LogIn/dbmanager.h"

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
        "   role TEXT CHECK(role IN ('admin', 'cashier')))"
        );

    // 商品表
    success &= executeTransaction(
        "CREATE TABLE IF NOT EXISTS products ("
        "   id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "   name TEXT NOT NULL,"
        "   barcode TEXT UNIQUE,"
        "   price REAL NOT NULL CHECK(price >= 0),"
        "   stock INTEGER DEFAULT 0 CHECK(stock >= 0),"
        "   category TEXT)"
        );

    // 会员表
    success &= executeTransaction(
        "CREATE TABLE IF NOT EXISTS members ("
        "   id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "   phone TEXT UNIQUE NOT NULL,"
        "   name TEXT,"
        "   discount REAL DEFAULT 1.0 CHECK(discount BETWEEN 0.1 AND 1.0))"
        );

    // 销售记录表
    success &= executeTransaction(
        "CREATE TABLE IF NOT EXISTS sales ("
        "   id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "   cashier_id INTEGER NOT NULL REFERENCES users(id),"
        "   total REAL NOT NULL CHECK(total >= 0),"
        "   payment REAL NOT NULL CHECK(payment >= 0),"
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
        createUser("admin", "admin123", "admin");
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

bool DBManager::createUser(const QString& username, const QString& password, const QString& role)
{
    QVariantList params;
    params << username << encryptPassword(password) << role;

    return executeTransaction(
        "INSERT INTO users (username, password, role) "
        "VALUES (?, ?, ?)",
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

bool DBManager::addMember(const QString& phone, const QString& name, double discount)
{
    QVariantList params;
    params << phone << name << discount;

    return executeTransaction(
        "INSERT INTO members (phone, name, discount) "
        "VALUES (?, ?, ?)",
        params
        );
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
    //SHA-256哈希存储对数据库中密码进行加密
}
