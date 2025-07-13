// SalesHistoryDialog.h
#ifndef SALESHISTORYDIALOG_H
#define SALESHISTORYDIALOG_H

#include <QDialog>
#include <QDateEdit>
#include <QTableView>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>
#include <QStandardItemModel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QGroupBox>

class SalesHistoryDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SalesHistoryDialog(int cashierId, QWidget *parent = nullptr);
    ~SalesHistoryDialog();

private slots:
    void onRefreshClicked();
    void onDateRangeChanged();

private:
    int m_cashierId;

    // UI 组件
    QDateEdit *m_startDateEdit;
    QDateEdit *m_endDateEdit;
    QCheckBox *m_autoRefreshCheckBox;
    QPushButton *m_refreshButton;
    QLabel *m_countLabel;
    QTableView *m_tableView;
    QStandardItemModel *m_model;
};

#endif // SALESHISTORYDIALOG_H
