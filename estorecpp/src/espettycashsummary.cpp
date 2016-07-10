#include "espettycashsummary.h"
#include "QDateTime"
#include "QSqlQuery"
#include "QStringList"
#include "qnamespace.h"
#include "QString"

PettyCashSummary::PettyCashSummary(QWidget *parent) : QWidget(parent)
{
	ui.setupUi(this);

	ui.fromDate->setDate(QDate::currentDate());
	ui.toDate->setDate(QDate::currentDate().addDays(1));

	QObject::connect(ui.fromDate, SIGNAL(dateChanged(const QDate &)), this, SLOT(slotDateChanged()));
	QObject::connect(ui.toDate, SIGNAL(dateChanged(const QDate &)), this, SLOT(slotDateChanged()));

	QStringList userWiseLabels;
	userWiseLabels.append("INCOME");
	userWiseLabels.append("EXPENSE");
/*	userWiseLabels.append("BALANCE");*/

	ui.tableWidgetByUser->setHorizontalHeaderLabels(userWiseLabels);
	ui.tableWidgetByUser->horizontalHeader()->setStretchLastSection(true);
	ui.tableWidgetByUser->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tableWidgetByUser->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableWidgetByUser->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableWidgetByUser->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.tableWidgetByUser->verticalHeader()->setMinimumWidth(200);

	QStringList headerLabels2;
	headerLabels2.append("INCOME");
	headerLabels2.append("EXPENSE");
/*	headerLabels2.append("BALANCE");*/

	ui.tableWidgetTotal->setHorizontalHeaderLabels(headerLabels2);
	ui.tableWidgetTotal->horizontalHeader()->setStretchLastSection(true);
	ui.tableWidgetTotal->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tableWidgetTotal->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableWidgetTotal->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableWidgetTotal->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.tableWidgetTotal->verticalHeader()->setMinimumWidth(200);

	//ui.datelbl->setText(QDate::currentDate().toString("yyyy-MM-dd"));

	displayResults();


}


PettyCashSummary::~PettyCashSummary()
{

}

void PettyCashSummary::slotDateChanged()
{
	displayResults();
}

void PettyCashSummary::displayResults()
{
	while (ui.tableWidgetByUser->rowCount() > 0)
	{
		ui.tableWidgetByUser->removeRow(0);
	}
	int row = 0;

	QDateTime startDate = QDateTime::fromString(ui.fromDate->text(), Qt::ISODate);
	QDateTime endDate = QDateTime::fromString(ui.toDate->text(), Qt::ISODate);
	QString stardDateStr = startDate.date().toString("yyyy-MM-dd");
	QString endDateStr = endDate.date().toString("yyyy-MM-dd");

	QSqlQuery queryUserType("SELECT * FROM user JOIN usertype ON user.usertype_id = usertype.usertype_id WHERE user.active = 1");
	while (queryUserType.next())
	{
		double income = 0, expense = 0;
		QString uId = queryUserType.value("user_id").toString();
		QString uName = queryUserType.value("display_name").toString();

		row = ui.tableWidgetByUser->rowCount();
		ui.tableWidgetByUser->insertRow(row);
		QTableWidgetItem* nameItem = new QTableWidgetItem(uName);
		ui.tableWidgetByUser->setVerticalHeaderItem(row, nameItem);

		QSqlQuery queryPettyCash("SELECT * FROM petty_cash WHERE user_id = " + uId + " AND  DATE(date) BETWEEN '" + stardDateStr + "' AND '" + endDateStr + "'");
		while (queryPettyCash.next())
		{
			QString uId = queryPettyCash.value("user_id").toString();
			int type = queryPettyCash.value("type").toInt();
			double amount = queryPettyCash.value("amount").toDouble();
			if (type == 1)
			{
				//income
				income += amount;
			}
			else if (type == 0)
			{
				//expense
				expense += amount;
			}
		}
		QTableWidgetItem* incomeWidgetItem = new QTableWidgetItem(QString::number(income, 'f', 2));
		incomeWidgetItem->setTextAlignment(Qt::AlignRight);
		ui.tableWidgetByUser->setItem(row, 0, incomeWidgetItem);

		QTableWidgetItem* expenseWidgetItem = new QTableWidgetItem(QString::number(expense, 'f', 2));
		expenseWidgetItem->setTextAlignment(Qt::AlignRight);
		ui.tableWidgetByUser->setItem(row, 1, expenseWidgetItem);
	}

	while (ui.tableWidgetTotal->rowCount() > 0)
	{
		ui.tableWidgetTotal->removeRow(0);
	}
	row = ui.tableWidgetTotal->rowCount();
	ui.tableWidgetTotal->insertRow(row);
	ui.tableWidgetTotal->setVerticalHeaderItem(row, new QTableWidgetItem("Total"));

	QSqlQuery totalQuery("SELECT sum(amount) as total,type FROM petty_cash WHERE  DATE(date) BETWEEN '" + stardDateStr + "' AND '" + endDateStr + "'" + " group by type");
	double income = 0, expense = 0;
	while (totalQuery.next())
	{
		int type = totalQuery.value("type").toUInt();
		if (type == 0)
		{
			//expense
			expense = totalQuery.value("total").toDouble();

			QTableWidgetItem* expenseWidgetItem = new QTableWidgetItem(QString::number(expense, 'f', 2));
			expenseWidgetItem->setTextAlignment(Qt::AlignRight);
			ui.tableWidgetTotal->setItem(row, 1, expenseWidgetItem);
		}
		else if (type == 1)
		{
			//income
			income = totalQuery.value("total").toDouble();

			QTableWidgetItem* incomeWidgetItem = new QTableWidgetItem(QString::number(income, 'f', 2));
			incomeWidgetItem->setTextAlignment(Qt::AlignRight);
			ui.tableWidgetTotal->setItem(row, 0, incomeWidgetItem);
		}
	}
}
