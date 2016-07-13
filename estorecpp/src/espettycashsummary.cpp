#include "espettycashsummary.h"
#include "QDateTime"
#include "QSqlQuery"
#include "QStringList"
#include "qnamespace.h"
#include "QString"

PettyCashSummary::PettyCashSummary(QWidget *parent) : QWidget(parent)
{
	ui.setupUi(this);

	m_generateReportSignalMapper = new QSignalMapper(this);

	ui.fromDate->setDate(QDate::currentDate());
	ui.toDate->setDate(QDate::currentDate().addDays(1));

	QObject::connect(ui.fromDate, SIGNAL(dateChanged(const QDate &)), this, SLOT(slotDateChanged()));
	QObject::connect(ui.toDate, SIGNAL(dateChanged(const QDate &)), this, SLOT(slotDateChanged()));

	QStringList userWiseLabels;
	userWiseLabels.append("Income");
	userWiseLabels.append("Expenses");
	userWiseLabels.append("Action");

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
	QObject::connect(m_generateReportSignalMapper, SIGNAL(mapped(QString)), this, SLOT(slotGenerateReportForGivenUser(QString)));
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

	QString stardDateStr = ui.fromDate->date().toString("yyyy-MM-dd");
	QString endDateStr = ui.toDate->date().toString("yyyy-MM-dd");

	QSqlQuery queryUserType("SELECT * FROM user JOIN usertype ON user.usertype_id = usertype.usertype_id WHERE user.active = 1 AND usertype.usertype_name <> 'DEV'");
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

		QWidget* base = new QWidget(ui.tableWidgetByUser);
		QPushButton* generateReportBtn = new QPushButton(base);
		generateReportBtn->setIcon(QIcon("icons/pdf.png"));
		generateReportBtn->setIconSize(QSize(24, 24));
		generateReportBtn->setMaximumWidth(100);

		m_generateReportSignalMapper->setMapping(generateReportBtn, uId);
		QObject::connect(generateReportBtn, SIGNAL(clicked()), m_generateReportSignalMapper, SLOT(map()));

		QHBoxLayout *layout = new QHBoxLayout;
		layout->setContentsMargins(0, 0, 0, 0);
		layout->addWidget(generateReportBtn);
		layout->insertStretch(2);
		base->setLayout(layout);
		ui.tableWidgetByUser->setCellWidget(row, 2, base);
		base->show();
	}

	while (ui.tableWidgetTotal->rowCount() > 0)
	{
		ui.tableWidgetTotal->removeRow(0);
	}
	row = ui.tableWidgetTotal->rowCount();
	ui.tableWidgetTotal->insertRow(row);
	ui.tableWidgetTotal->setVerticalHeaderItem(row, new QTableWidgetItem("Total"));

	QSqlQuery totalQuery("SELECT * FROM petty_cash WHERE  DATE(date) BETWEEN '" + stardDateStr + "' AND '" + endDateStr + "'");
	QString srtr("SELECT * FROM petty_cash WHERE  DATE(date) BETWEEN '" + stardDateStr + "' AND '" + endDateStr + "'");
	double income = 0, expense = 0;
	while (totalQuery.next())
	{
		QSqlQuery queryUserType("SELECT * FROM user JOIN usertype ON user.usertype_id = usertype.usertype_id WHERE user.active = 1 AND user.user_id = " + totalQuery.value("user_id").toString() + " AND usertype.usertype_name <> 'DEV'");
		if (queryUserType.next())
		{
			int type = totalQuery.value("type").toUInt();
			if (type == 0)
			{
				//expense
				expense += totalQuery.value("amount").toDouble();

			}
			else if (type == 1)
			{
				//income
				income += totalQuery.value("amount").toDouble();

			}
		}
	}
	QTableWidgetItem* incomeWidgetItem = new QTableWidgetItem(QString::number(income, 'f', 2));
	incomeWidgetItem->setTextAlignment(Qt::AlignRight);
	ui.tableWidgetTotal->setItem(row, 0, incomeWidgetItem);

	QTableWidgetItem* expenseWidgetItem = new QTableWidgetItem(QString::number(expense, 'f', 2));
	expenseWidgetItem->setTextAlignment(Qt::AlignRight);
	ui.tableWidgetTotal->setItem(row, 1, expenseWidgetItem);
}

void PettyCashSummary::slotGenerateReportForGivenUser(QString userId)
{

}
