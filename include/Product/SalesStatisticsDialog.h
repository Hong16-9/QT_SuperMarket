// SalesStatisticsDialog.h
#ifndef SALESSTATISTICSDIALOG_H
#define SALESSTATISTICSDIALOG_H

#include <QDialog>
#include <QTableView>
#include <QStandardItemModel>
#include <QHeaderView>
#include <QPushButton>
#include <QComboBox>
#include <QDateEdit>

class SalesStatisticsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SalesStatisticsDialog(QWidget *parent = nullptr);
    ~SalesStatisticsDialog() override;

private slots:
    void onRefreshClicked();
    void onExportClicked();

private:
    QTableView *tableView;
    QStandardItemModel *salesModel;
//    QComboBox *periodCombo;
//    QDateEdit *startDateEdit;
//    QDateEdit *endDateEdit;

    void loadSalesData();
    void setupUI();
};

#endif // SALESSTATISTICSDIALOG_H
