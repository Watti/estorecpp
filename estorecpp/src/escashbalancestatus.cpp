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
	headerLabels.append("User");
	headerLabels.append("Day Starting Amount");
	headerLabels.append("Total Cash Sales");
	headerLabels.append("Handover Amount");
	ui.tableWidget->setHorizontalHeaderLabels(headerLabels);
	ui.tableWidget->horizontalHeader()->setStretchLastSection(true);
	ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);

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
	QString curDate = QDate::currentDate().toString("yyyy-MM-dd");
	int userId = ES::Session::getInstance()->getUser()->getId();
	QString selectQryStr("SELECT start_amount FROM cash_config WHERE DATE(date) = CURDATE() AND user_id = " + QString::number(userId));
	QSqlQuery selectQuery(selectQryStr);
	if (selectQuery.size() == 1 )
	{
		while (selectQuery.next())
		{
			float startAmount = selectQuery.value("start_amount").toFloat();
			int row = 0;
			QSqlQuery billQueary("SELECT SUM(amount) as totalAmount, user_id FROM bill WHERE status = 1 AND payment_method = 1 AND DATE(`date`) = CURDATE() AND deleted = 0 GROUP BY user_id");
			while (billQueary.next())
			{
				row = ui.tableWidget->rowCount();
				ui.tableWidget->insertRow(row);
				float total = billQueary.value("totalAmount").toFloat();
				QString userId = billQueary.value("user_id").toString();
				QString paymentMethod = billQueary.value(1).toString();
				QSqlQuery paymentQuery("SELECT type FROM payment WHERE type_id = " + paymentMethod);
				ui.tableWidget->setItem(row, 1, new QTableWidgetItem(QString::number(startAmount)));
				ui.tableWidget->setItem(row, 3, new QTableWidgetItem(QString::number(startAmount + total)));

				QSqlQuery userQuery("SELECT display_name FROM user WHERE user_id = " + userId);
				while (userQuery.next())
				{
					ui.tableWidget->setItem(row, 0, new QTableWidgetItem(userQuery.value("display_name").toString()));
				}
				ui.tableWidget->setItem(row, 2, new QTableWidgetItem(QString::number(total)));
			}
		}
		
	}
	else
	{
		LOG(ERROR) << "ESCashBalanceStatus::displayStatus() - More than one entry for start day in cash_config";
	}
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

	int total5000 = ui.note5000->text().toInt()*5000;
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
