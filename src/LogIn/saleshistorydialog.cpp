// SalesHistoryDialog.cpp
#include "LogIn/SalesHistoryDialog.h"
#include "LogIn/dbmanager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QMessageBox>
#include <QDateTime>
#include <QDebug>

SalesHistoryDialog::SalesHistoryDialog(int cashierId, QWidget *parent)
    : QDialog(parent), m_cashierId(cashierId)
{
    setWindowTitle("销售历史记录");
    setMinimumSize(500, 600);

    // 创建主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 日期范围组
    QGroupBox *dateGroup = new QGroupBox("日期范围", this);
    QHBoxLayout *dateLayout = new QHBoxLayout(dateGroup);

    // 开始日期
    dateLayout->addWidget(new QLabel("开始日期:", this));
    m_startDateEdit = new QDateEdit(this);
    m_startDateEdit->setCalendarPopup(true);
    m_startDateEdit->setDate(QDate::currentDate().addDays(-7)); // 默认显示最近7天
    dateLayout->addWidget(m_startDateEdit);

    // 结束日期
    dateLayout->addWidget(new QLabel("结束日期:", this));
    m_endDateEdit = new QDateEdit(this);
    m_endDateEdit->setCalendarPopup(true);
    m_endDateEdit->setDate(QDate::currentDate());
    dateLayout->addWidget(m_endDateEdit);

    // 自动刷新选项
    m_autoRefreshCheckBox = new QCheckBox("自动刷新", this);
    m_autoRefreshCheckBox->setChecked(true);
    dateLayout->addWidget(m_autoRefreshCheckBox);

    // 刷新按钮
    m_refreshButton = new QPushButton("刷新", this);
    dateLayout->addWidget(m_refreshButton);

    // 记录计数标签
    m_countLabel = new QLabel("记录: 0", this);
    dateLayout->addWidget(m_countLabel);

    dateLayout->addStretch();
    mainLayout->addWidget(dateGroup);

    // 创建表格模型
    m_model = new QStandardItemModel(0, 5, this);
    m_model->setHorizontalHeaderLabels({
        "时间", "交易号", "总金额", "会员手机", "商品明细"
    });

    // 创建表格视图
    m_tableView = new QTableView(this);
    m_tableView->setModel(m_model);
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tableView->setSortingEnabled(true);
    m_tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // 设置列宽
    m_tableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents); // 时间
    m_tableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents); // 交易号
    m_tableView->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents); // 总金额
    m_tableView->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents); // 会员手机
    m_tableView->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);          // 商品明细

    mainLayout->addWidget(m_tableView);

    // 连接信号
    connect(m_refreshButton, &QPushButton::clicked, this, &SalesHistoryDialog::onRefreshClicked);
    connect(m_startDateEdit, &QDateEdit::dateChanged, this, &SalesHistoryDialog::onDateRangeChanged);
    connect(m_endDateEdit, &QDateEdit::dateChanged, this, &SalesHistoryDialog::onDateRangeChanged);

    // 初始加载数据
    onRefreshClicked();
}

SalesHistoryDialog::~SalesHistoryDialog()
{
    // 自动删除子对象
}

void SalesHistoryDialog::onRefreshClicked()
{
    m_model->removeRows(0, m_model->rowCount());

    QDateTime start = QDateTime(m_startDateEdit->date(), QTime(0, 0, 0));
    QDateTime end = QDateTime(m_endDateEdit->date(), QTime(23, 59, 59));

    // 获取销售历史数据
    QList<QMap<QString, QVariant>> sales = DBManager::instance().getSalesHistory(m_cashierId, start, end);

    for (const auto &sale : sales) {
        QList<QStandardItem*> items;

        // 时间列
        QDateTime timestamp = sale["timestamp"].toDateTime();
        items << new QStandardItem(timestamp.toString("yyyy-MM-dd HH:mm"));

        // 交易号列
        items << new QStandardItem(sale["id"].toString());

        // 总金额列
        items << new QStandardItem(QString::number(sale["total"].toDouble(), 'f', 2));

        // 会员手机列
        QString memberPhone = sale["member_phone"].toString();
        items << new QStandardItem(memberPhone.isEmpty() ? "非会员" : memberPhone);

        // 商品明细列
        items << new QStandardItem(sale["items"].toString());

        // 添加到模型
        foreach (QStandardItem* item, items) {
            item->setEditable(false);
        }
        m_model->appendRow(items);
    }

    // 更新计数标签
    m_countLabel->setText(QString("记录: %1").arg(sales.size()));
}

void SalesHistoryDialog::onDateRangeChanged()
{
    if (m_autoRefreshCheckBox->isChecked()) {
        onRefreshClicked();
    }
}
