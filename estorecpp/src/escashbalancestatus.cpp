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
