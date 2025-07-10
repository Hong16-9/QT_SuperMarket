#ifndef SALESSTATISTICSDIALOG_H
#define SALESSTATISTICSDIALOG_H

#include <QDialog>
#include <QTableView>
#include <QStandardItemModel>
#include <QHeaderView>

class SalesStatisticsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SalesStatisticsDialog(QWidget *parent = nullptr);
    ~SalesStatisticsDialog() override;

private:
    QTableView *tableView;
    QStandardItemModel *salesModel;

    void loadMonthlySales();
};

#endif // SALESSTATISTICSDIALOG_H
