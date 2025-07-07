#ifndef DBMANAGER_H
#define DBMANAGER_H

#pragma once

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QString>
#include <QDebug>
#include <QCryptographicHash>
#include<QSqlError>

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
    bool createUser(const QString& username, const QString& password, const QString& role);
    QString authenticateUser(const QString& username, const QString& password);

    // 商品管理
    bool addProduct(const QString& name, const QString& barcode, double price, int stock, const QString& category = "");
    bool updateProductStock(int productId, int delta);

    // 会员管理
    bool addMember(const QString& phone, const QString& name, double discount = 1.0);
    double getMemberDiscount(const QString& phone);

    // 事务执行
    bool executeTransaction(const QString& sql, const QVariantList& params = QVariantList());
    QSqlQuery executeQuery(const QString& sql, const QVariantList& params = QVariantList());

    // 数据库状态
    bool isOpen() const;
    QString lastError() const;

    // 实用函数
    static QString encryptPassword(const QString& password);

private:
    // 私有构造函数
    explicit DBManager(QObject *parent = nullptr);
    ~DBManager();

    // 创建数据库表结构
    bool createTables();

    // 初始化默认数据
    bool initDefaultData();

    QSqlDatabase m_database;
};

#endif // DBMANAGER_H
