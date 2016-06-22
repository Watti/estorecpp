#include "escashbalancestatus.h"
#include "QSqlQuery"
#include "QMessageBox"
#include "utility\esdbconnection.h"
#include "QDateTime"
#include "utility\session.h"
#include "entities\user.h"
#include "easylogging++.h"

ESCashBalanceStatus::ESCashBalanceStatus(QWidget *parent) : QWidget(parent)
{
	if (!ES::DbConnection::instance()->open())
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Critical);
		mbox.setText(QString("Cannot connect to the database : ESCashBalanceStatus"));
		mbox.exec();
	}
	ui.setupUi(this);
	QStringList headerLabels;
	headerLabels.append("Type");
	headerLabels.append("Description");
	headerLabels.append("Amount");
	//headerLabels.append("Handover Amount");
	ui.tableWidget->setHorizontalHeaderLabels(headerLabels);
	ui.tableWidget->horizontalHeader()->setStretchLastSection(true);
	ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
	ui.tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.tableWidget->setColumnWidth(1,500);

	QObject::connect(ui.btnDayoff, SIGNAL(clicked()), this, SLOT(dayOff()));
	QObject::connect(ui.note5000, SIGNAL(textChanged(QString)), this, SLOT(slotCalculateTotal()));
	QObject::connect(ui.note2000, SIGNAL(textChanged(QString)), this, SLOT(slotCalculateTotal()));
	QObject::connect(ui.note1000, SIGNAL(textChanged(QString)), this, SLOT(slotCalculateTotal()));
	QObject::connect(ui.note500, SIGNAL(textChanged(QString)), this, SLOT(slotCalculateTotal()));
	QObject::connect(ui.note200, SIGNAL(textChanged(QString)), this, SLOT(slotCalculateTotal()));
	QObject::connect(ui.note100, SIGNAL(textChanged(QString)), this, SLOT(slotCalculateTotal()));
	QObject::connect(ui.note50, SIGNAL(textChanged(QString)), this, SLOT(slotCalculateTotal()));
	QObject::connect(ui.note20, SIGNAL(textChanged(QString)), this, SLOT(slotCalculateTotal()));
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
	int userId = ES::Session::getInstance()->getUser()->getId();
	int row = 0;
	QString curDate = QDate::currentDate().toString("yyyy-MM-dd");
	QString selectQryStr("SELECT start_amount FROM cash_config WHERE DATE(date) = CURDATE() AND user_id = " + QString::number(userId));
	QSqlQuery selectQuery(selectQryStr);
	float handOver = 0, startAmount = 0; 
	QColor red(245, 169, 169);
	QColor green(169, 245, 208);
	if (selectQuery.size() == 1)
	{
		while (selectQuery.next())
		{
			startAmount = selectQuery.value("start_amount").toFloat();
			handOver += startAmount;
			row = ui.tableWidget->rowCount();
			ui.tableWidget->insertRow(row);
			QTableWidgetItem* tWidget = new QTableWidgetItem("(+)");
			tWidget->setBackgroundColor(green);
			ui.tableWidget->setItem(row, 0, tWidget);

			QTableWidgetItem* descWidget = new QTableWidgetItem("Starting Balance");
			descWidget->setBackgroundColor(green);
			ui.tableWidget->setItem(row, 1, descWidget);
			
			QTableWidgetItem* amountWidget = new QTableWidgetItem(QString::number(startAmount, 'f', 2));
			amountWidget->setBackgroundColor(green);
			ui.tableWidget->setItem(row, 2, amountWidget);
		}
	}
	QSqlQuery billQueary("SELECT SUM(total_amount) as totalAmount, user_id FROM payment as p JOIN bill as b ON p.bill_id = b.bill_id WHERE b.status = 1 AND p.payment_type = 'CASH' AND b.deleted = 0 AND b.user_id = "+QString::number(userId)+" AND b.date >= CURDATE() - INTERVAL 1 DAY");
	while (billQueary.next())
	{
		float totalCashSales = billQueary.value("totalAmount").toFloat();
		if (totalCashSales > 0)
		{
			row = ui.tableWidget->rowCount();
			ui.tableWidget->insertRow(row);
			handOver += totalCashSales;
			QString userId = billQueary.value("user_id").toString();
			QString paymentMethod = billQueary.value(1).toString();
			QSqlQuery paymentQuery("SELECT type FROM payment WHERE type_id = " + paymentMethod);
			QTableWidgetItem* tWidget = new QTableWidgetItem("(+)");
			tWidget->setBackgroundColor(green);
			ui.tableWidget->setItem(row, 0, tWidget);

			QTableWidgetItem* descWidget = new QTableWidgetItem("Total Sales");
			descWidget->setBackgroundColor(green);
			ui.tableWidget->setItem(row, 1, descWidget);

			QTableWidgetItem* amountWidget = new QTableWidgetItem(QString::number(totalCashSales, 'f', 2));
			amountWidget->setBackgroundColor(green);
			ui.tableWidget->setItem(row, 2, amountWidget);
		}
	}

	//Expenses
	QString pCashQStr("SELECT amount, remarks, type FROM petty_cash WHERE DATE(date) = CURDATE() AND user_id = " + QString::number(userId));
	QSqlQuery qPettyCash(pCashQStr);
	while (qPettyCash.next())
	{
		row = ui.tableWidget->rowCount();
		ui.tableWidget->insertRow(row);
		float pCashAmount = qPettyCash.value("amount").toFloat();
		int cType = qPettyCash.value("type").toInt();
		if (cType == 0)
		{
			QTableWidgetItem* tWidget = new QTableWidgetItem("(-)");
			tWidget->setBackgroundColor(red);
			ui.tableWidget->setItem(row, 0, tWidget);

			QTableWidgetItem* descWidget = new QTableWidgetItem(qPettyCash.value("remarks").toString());
			descWidget->setBackgroundColor(red);
			ui.tableWidget->setItem(row, 1, descWidget);

			QTableWidgetItem* amountWidget = new QTableWidgetItem(QString::number(pCashAmount, 'f', 2));
			amountWidget->setBackgroundColor(red);
			ui.tableWidget->setItem(row, 2, amountWidget);
		}
		else
		{
			QTableWidgetItem* tWidget = new QTableWidgetItem("(+)");
			tWidget->setBackgroundColor(green);
			ui.tableWidget->setItem(row, 0, tWidget);

			QTableWidgetItem* descWidget = new QTableWidgetItem(qPettyCash.value("remarks").toString());
			descWidget->setBackgroundColor(green);
			ui.tableWidget->setItem(row, 1, descWidget);

			QTableWidgetItem* amountWidget = new QTableWidgetItem(QString::number(pCashAmount, 'f', 2));
			amountWidget->setBackgroundColor(green);
			ui.tableWidget->setItem(row, 2, amountWidget);
		}
		handOver -= pCashAmount;
	}
	ui.handoverLbl->setText(QString::number(handOver, 'f', 2));
	ui.startBalanceLbl->setText(QString::number(startAmount, 'f', 2));

	QSqlQuery userQuery("SELECT display_name FROM user WHERE user_id = " + QString::number(userId));
	while (userQuery.next())
	{
		ui.userLbl->setText(userQuery.value("display_name").toString());
	}
	ui.dateLbl->setText(QDate::currentDate().toString("yyyy-MM-dd"));
}

void ESCashBalanceStatus::dayOff()
{
	int userId = ES::Session::getInstance()->getUser()->getId();
	QString selectQryStr("DELETE FROM cash_config WHERE DATE(date) = CURDATE() AND user_id = " + QString::number(userId));
	QSqlQuery selectQuery(selectQryStr);
	this->close();
}

void ESCashBalanceStatus::slotCalculateTotal()
{
	int total = 0;

	int total5000 = ui.note5000->text().toInt() * 5000;
	ui.note5000Total->setText(QString::number(total5000));

	int total2000 = ui.note2000->text().toInt() * 2000;
	ui.note2000Total->setText(QString::number(total2000));

	int total1000 = ui.note1000->text().toInt() * 1000;
	ui.note1000Total->setText(QString::number(total1000));

	int total500 = ui.note500->text().toInt() * 500;
	ui.note500Total->setText(QString::number(total500));

	int total200 = ui.note200->text().toInt() * 200;
	ui.note200Total->setText(QString::number(total200));

	int total100 = ui.note100->text().toInt() * 100;
	ui.note100Total->setText(QString::number(total100));

	int total50 = ui.note50->text().toInt() * 50;
	ui.note50Total->setText(QString::number(total50));

	int total20 = ui.note20->text().toInt() * 20;
	ui.note20Total->setText(QString::number(total20));

	total = total5000 + total2000 + total1000 + total500 + total200 + total100 + total50 + total20;
	ui.grandTotal->setText(QString::number(total));
}
