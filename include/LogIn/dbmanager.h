#ifndef DBMANAGER_H
#define DBMANAGER_H

#pragma once

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QString>
#include <QDebug>
#include <QCryptographicHash>
#include <QSqlError>
#include <QVariantMap>
#include <QDateTime>
#include <QList>
#include <QMap>

class DBManager : public QObject
{
    Q_OBJECT

public:
    // 单例模式访问点
    static DBManager& instance();

    // 禁止复制和赋值
    DBManager(const DBManager&) = delete;
    DBManager& operator=(const DBManager&) = delete;

    // 数据库初始化
    bool initialize(const QString& path = "supermarket.db");

    // 用户管理
    bool createUser(const QString& username, const QString& password, const QString& role, const QString& gender, const int& age);
    QString authenticateUser(const QString& username, const QString& password);

    // 商品管理
    bool addProduct(const QString& name, const QString& barcode, double price, int stock, const QString& category = "");
    bool updateProductStock(int productId, int delta);
    bool updateProductPrice(int productId, double newPrice); // 新增：修改单价
    bool deleteProduct(int productId);  // 新增：删除商品
    QList<QMap<QString, QVariant>> searchProducts(const QString& keyword);  // 新增：按关键词搜索

    // 会员管理
    bool addMember(const QString& phone, const QString& name,
                   double discount = 1.0, const QString& birthday = "",
                   int points = 0);
    bool updateMemberPoints(const QString& phone, int points);  // 更新会员积分
    double getMemberDiscount(const QString& phone);
    int getMemberPoints(const QString& phone); // 更新会员积分
    double calculateDiscountByPoints(int points);  // 根据积分获取折扣率

    // 数据库状态
    bool isOpen() const;
    QString lastError() const;

    // 实用函数（密码加密）
    static QString encryptPassword(const QString& password);

    // ====== 查询接口 ======
    // 商品查询
    QList<QMap<QString, QVariant>> getAllProducts();
    QMap<QString, QVariant> getProductById(int productId);
    QList<QMap<QString, QVariant>> getProductsByName(const QString& name);
    QList<QMap<QString, QVariant>> getProductsByCategory(const QString& category);

    // 会员查询
    QList<QMap<QString, QVariant>> getAllMembers();
    QMap<QString, QVariant> getMemberByPhone(const QString& phone);
    QList<QMap<QString, QVariant>> getMembersByName(const QString& name);

    // 销售记录查询
    QList<QMap<QString, QVariant>> getSalesByDateRange(const QDateTime& start, const QDateTime& end);
    QList<QMap<QString, QVariant>> getSaleItemsBySaleId(int saleId);

    // 添加销售记录和销售明细
    bool addSale(int cashierId, double total, double payment,
                 const QList<QVariantMap>& items,
                 const QString& memberPhone = "");

    // 获取当月商品销量统计
    QList<QMap<QString, QVariant>> getMonthlyProductSales();

    // 用户管理
    bool registerUser(const QString& username, const QString& password, const QString& gender, const int& age, const QString& role = "cashier");
    int getUserIdByName(const QString& username);

    // 新增管理员认证方法
    bool authenticateAdmin(const QString& adminUsername, const QString& adminPassword);


    //重置月销量
    void resetmonthlysale();

    // 在dbmanager.h的public部分添加
    QSqlDatabase& getDatabase() { return m_database; }

    bool isBarcodeExists(const QString& barcode);
private:
    // 事务执行
    bool executeTransaction(const QString& sql, const QVariantList& params = QVariantList());
    QSqlQuery executeQuery(const QString& sql, const QVariantList& params = QVariantList());

    // 私有构造函数
    explicit DBManager(QObject *parent = nullptr);
    ~DBManager();

    // 创建数据库表结构
    bool createTables();

    // 初始化默认数据
    bool initDefaultData();

    QSqlDatabase m_database;  // 数据库连接对象
};

#endif // DBMANAGER_H
