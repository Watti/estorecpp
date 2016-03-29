#include "escashbalancestatus.h"
#include "QSqlQuery"

ESCashBalanceStatus::ESCashBalanceStatus(QWidget *parent) : QWidget(parent)
{
	ui.setupUi(this); 
	QStringList headerLabels;
	headerLabels.append("User");
	headerLabels.append("Total");
	headerLabels.append("Payment Method");
	ui.tableWidget->setHorizontalHeaderLabels(headerLabels);
	ui.tableWidget->horizontalHeader()->setStretchLastSection(true);
	ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	displayStatus();
}

ESCashBalanceStatus::~ESCashBalanceStatus()
{}

Ui::CashBalanceStatus& ESCashBalanceStatus::getUI()
{
	return ui;
}

void ESCashBalanceStatus::displayStatus()
{
	int row = 0;
	QSqlQuery billQueary("SELECT SUM(amount), payment_method, user_id FROM bill WHERE status = 1 AND DATE(`date`) = CURDATE() AND deleted = 0 GROUP BY payment_method, user_id");
	while (billQueary.next())
	{
		row = ui.tableWidget->rowCount();
		ui.tableWidget->insertRow(row);
		QString total = billQueary.value(0).toString();
		QString userId = billQueary.value(2).toString();
		QString paymentMethod = billQueary.value(1).toString();
		QSqlQuery paymentQuery("SELECT type FROM payment WHERE type_id = "+paymentMethod);
		while (paymentQuery.next())
		{
			ui.tableWidget->setItem(row, 2, new QTableWidgetItem(paymentQuery.value(0).toString()));
		}

		QSqlQuery userQuery("SELECT display_name FROM user WHERE user_id = " + userId);
		while (userQuery.next())
		{
			ui.tableWidget->setItem(row, 0, new QTableWidgetItem(userQuery.value(0).toString()));
		}
		ui.tableWidget->setItem(row, 1, new QTableWidgetItem(total));
	}
}
