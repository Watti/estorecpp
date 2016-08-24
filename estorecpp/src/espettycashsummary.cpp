#include "espettycashsummary.h"
#include "QDateTime"
#include "QSqlQuery"
#include "QStringList"
#include "qnamespace.h"
#include "QString"
#include "KDReportsTextElement.h"
#include "KDReportsCell.h"
#include "QPrintPreviewDialog"
#include "QPrinter"
#include "utility\esmainwindowholder.h"
#include "QMainWindow"
#include "utility\session.h"
#include "QVariant"
#include "QMainWindow"
#include "esmainwindow.h"
PettyCashSummary::PettyCashSummary(QWidget *parent) : QWidget(parent)
{
	ui.setupUi(this);

	m_generateReportSignalMapper = new QSignalMapper(this);

	ui.fromDate->setDate(QDate::currentDate());
	ui.toDate->setDate(QDate::currentDate().addDays(1));

	QStringList userWiseLabels;
	userWiseLabels.append("Cash In");
	userWiseLabels.append("Cash Out");
	userWiseLabels.append("Action");

	ui.tableWidgetByUser->setHorizontalHeaderLabels(userWiseLabels);
	ui.tableWidgetByUser->horizontalHeader()->setStretchLastSection(true);
	ui.tableWidgetByUser->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tableWidgetByUser->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableWidgetByUser->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableWidgetByUser->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.tableWidgetByUser->verticalHeader()->setMinimumWidth(200);

	QStringList headerLabels2;
	headerLabels2.append("Cash In");
	headerLabels2.append("Cash Out");
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
	QObject::connect(ui.fromDate, SIGNAL(dateChanged(const QDate &)), this, SLOT(slotDateChanged()));
	QObject::connect(ui.toDate, SIGNAL(dateChanged(const QDate &)), this, SLOT(slotDateChanged()));
	QObject::connect(ui.generateButton, SIGNAL(clicked()), this, SLOT(slotGenerateReport()));
	//displayResults();


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
		QString uId = queryUserType.value("user_id").toString();
		QString uName = queryUserType.value("display_name").toString();

		row = ui.tableWidgetByUser->rowCount();
		ui.tableWidgetByUser->insertRow(row);
		QTableWidgetItem* nameItem = new QTableWidgetItem(uName);
		ui.tableWidgetByUser->setVerticalHeaderItem(row, nameItem);

		double income = 0, expense = 0;
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
	KDReports::TextElement titleElement("Petty Cash Report");
	titleElement.setPointSize(13);
	titleElement.setBold(true);
	report.addElement(titleElement, Qt::AlignHCenter);

	QString stardDateStr = ui.fromDate->date().toString("yyyy-MM-dd");
	QString endDateStr = ui.toDate->date().toString("yyyy-MM-dd");

	QString dateStr = "Date : ";
	dateStr.append(stardDateStr).append(" - ").append(endDateStr);

	QString userStr = "User : ";

	QSqlQuery queryUser("SELECT * FROM user WHERE user_id = " + userId);
	if (queryUser.next())
	{
		userStr.append(queryUser.value("display_name").toString());
	}

	KDReports::TableElement infoTableElement;
	infoTableElement.setHeaderRowCount(2);
	infoTableElement.setHeaderColumnCount(2);
	infoTableElement.setBorder(0);
	infoTableElement.setWidth(100, KDReports::Percent);

	{
		KDReports::Cell& dateCell = infoTableElement.cell(0, 1);
		KDReports::TextElement t(dateStr);
		t.setPointSize(10);
		dateCell.addElement(t, Qt::AlignRight);
	}

	{
		KDReports::Cell& userCell = infoTableElement.cell(0, 0);
		KDReports::TextElement t(userStr);
		t.setPointSize(10);
		userCell.addElement(t, Qt::AlignLeft);
	}

	report.addElement(infoTableElement);
	report.addVerticalSpacing(5);

	KDReports::TableElement tableElement;
	tableElement.setHeaderColumnCount(4);
	tableElement.setBorder(1);
	tableElement.setWidth(100, KDReports::Percent);

	{
		KDReports::Cell& cell = tableElement.cell(0, 0);
		KDReports::TextElement cTextElement("Date");
		cTextElement.setPointSize(11);
		cTextElement.setBold(true);
		cell.addElement(cTextElement, Qt::AlignCenter);
	}
	{
		KDReports::Cell& cell = tableElement.cell(0, 1);
		KDReports::TextElement cTextElement("Description");
		cTextElement.setPointSize(11);
		cTextElement.setBold(true);
		cell.addElement(cTextElement, Qt::AlignCenter);
	}
	{
		KDReports::Cell& cell = tableElement.cell(0, 2);
		KDReports::TextElement cTextElement("Cash In");
		cTextElement.setPointSize(11);
		cTextElement.setBold(true);
		cell.addElement(cTextElement, Qt::AlignCenter);
	}
	{
		KDReports::Cell& cell = tableElement.cell(0, 3);
		KDReports::TextElement cTextElement("Cash Out");
		cTextElement.setPointSize(11);
		cTextElement.setBold(true);
		cell.addElement(cTextElement, Qt::AlignCenter);
	}

	QString pCashQStr("SELECT * FROM petty_cash WHERE user_id = " + userId + " AND  DATE(date) BETWEEN '" + stardDateStr + "' AND '" + endDateStr + "'");
	QSqlQuery qPettyCash(pCashQStr);
	float totalIn = 0, totalOut = 0;
	int row = 1;
	while (qPettyCash.next())
	{
		float pCashAmount = qPettyCash.value("amount").toFloat();
		int cType = qPettyCash.value("type").toInt();
		QString dateStr = qPettyCash.value("date").toDateTime().date().toString("yyyy-MM-dd");
		QString description = qPettyCash.value("remarks").toString();
		printRow(tableElement, row, 0, dateStr);
		printRow(tableElement, row, 1, description);
		if (cType == 0)
		{
			//expense
			printRow(tableElement, row, 3, QString::number(pCashAmount, 'f', 2), Qt::AlignRight);
			totalOut += pCashAmount;
		}
		else
		{
			//income
			printRow(tableElement, row, 2, QString::number(pCashAmount, 'f', 2), Qt::AlignRight);
			totalIn += pCashAmount;
		}
		row++;
	}

	printRow(tableElement, row, 1, "Total");
	printRow(tableElement, row, 2, QString::number(totalIn, 'f', 2), Qt::AlignRight);
	printRow(tableElement, row, 3, QString::number(totalOut, 'f', 2), Qt::AlignRight);
	report.addElement(tableElement);

	QPrinter printer;
	printer.setPaperSize(QPrinter::A4);

	printer.setFullPage(false);
	printer.setOrientation(QPrinter::Portrait);

	QPrintPreviewDialog *dialog = new QPrintPreviewDialog(&printer, this);
	QObject::connect(dialog, SIGNAL(paintRequested(QPrinter*)), this, SLOT(slotPrint(QPrinter*)));
	dialog->setWindowTitle(tr("Print Document"));
	ES::MainWindowHolder::instance()->getMainWindow()->setCentralWidget(dialog);
	dialog->exec();
}

void PettyCashSummary::printRow(KDReports::TableElement& tableElement, int row, int col, QString elementStr, Qt::AlignmentFlag alignment /*= Qt::AlignLeft*/)
{
	KDReports::Cell& cell = tableElement.cell(row, col);
	KDReports::TextElement te(elementStr);
	te.setPointSize(ES::Session::getInstance()->getBillItemFontSize());
	cell.addElement(te, alignment);
}

void PettyCashSummary::slotPrint(QPrinter* printer)
{
	report.print(printer);
	this->close();
}

void PettyCashSummary::slotGenerateReport()
{
	KDReports::TextElement titleElement("Petty Cash Summary Report");
	titleElement.setPointSize(13);
	titleElement.setBold(true);
	report.addElement(titleElement, Qt::AlignHCenter);

	QString stardDateStr = ui.fromDate->date().toString("yyyy-MM-dd");
	QString endDateStr = ui.toDate->date().toString("yyyy-MM-dd");

	QString dateStr = "Date : ";
	dateStr.append(stardDateStr).append(" - ").append(endDateStr);


	KDReports::TableElement infoTableElement;
	infoTableElement.setHeaderRowCount(2);
	infoTableElement.setHeaderColumnCount(2);
	infoTableElement.setBorder(0);
	infoTableElement.setWidth(100, KDReports::Percent);

	{
		KDReports::Cell& dateCell = infoTableElement.cell(0, 1);
		KDReports::TextElement t(dateStr);
		t.setPointSize(10);
		dateCell.addElement(t, Qt::AlignRight);
	}

	report.addElement(infoTableElement);
	report.addVerticalSpacing(5);

	KDReports::TableElement tableElement;
	tableElement.setHeaderColumnCount(5);
	tableElement.setBorder(1);
	tableElement.setWidth(100, KDReports::Percent);

	{
		KDReports::Cell& cell = tableElement.cell(0, 0);
		KDReports::TextElement cTextElement("Date");
		cTextElement.setPointSize(11);
		cTextElement.setBold(true);
		cell.addElement(cTextElement, Qt::AlignCenter);
	}
	{
		KDReports::Cell& cell = tableElement.cell(0, 1);
		KDReports::TextElement cTextElement("User");
		cTextElement.setPointSize(11);
		cTextElement.setBold(true);
		cell.addElement(cTextElement, Qt::AlignCenter);
	}
	{
		KDReports::Cell& cell = tableElement.cell(0, 2);
		KDReports::TextElement cTextElement("Description");
		cTextElement.setPointSize(11);
		cTextElement.setBold(true);
		cell.addElement(cTextElement, Qt::AlignCenter);
	}
	{
		KDReports::Cell& cell = tableElement.cell(0, 3);
		KDReports::TextElement cTextElement("Cash In");
		cTextElement.setPointSize(11);
		cTextElement.setBold(true);
		cell.addElement(cTextElement, Qt::AlignCenter);
	}
	{
		KDReports::Cell& cell = tableElement.cell(0, 4);
		KDReports::TextElement cTextElement("Cash Out");
		cTextElement.setPointSize(11);
		cTextElement.setBold(true);
		cell.addElement(cTextElement, Qt::AlignCenter);
	}

	QString pCashQStr("SELECT * FROM petty_cash WHERE DATE(date) BETWEEN '" + stardDateStr + "' AND '" + endDateStr + "'");
	QSqlQuery qPettyCash(pCashQStr);
	float totalIn = 0, totalOut = 0;
	int row = 1;
	while (qPettyCash.next())
	{
		float pCashAmount = qPettyCash.value("amount").toFloat();
		int cType = qPettyCash.value("type").toInt();
		QString dateStr = qPettyCash.value("date").toDateTime().date().toString("yyyy-MM-dd");
		QString description = qPettyCash.value("remarks").toString();
		printRow(tableElement, row, 0, dateStr);
		QSqlQuery queryUserType("SELECT * FROM user JOIN usertype ON user.usertype_id = usertype.usertype_id WHERE user.active = 1 AND user.user_id = " + qPettyCash.value("user_id").toString() + " AND usertype.usertype_name <> 'DEV'");
		if (queryUserType.next())
		{
			QString uName = queryUserType.value("display_name").toString();
			printRow(tableElement, row, 1, uName);
			printRow(tableElement, row, 2, description);
			if (cType == 0)
			{
				//expense
				printRow(tableElement, row, 4, QString::number(pCashAmount, 'f', 2), Qt::AlignRight);
				totalOut += pCashAmount;
			}
			else
			{
				//income
				printRow(tableElement, row, 3, QString::number(pCashAmount, 'f', 2), Qt::AlignRight);
				totalIn += pCashAmount;
			}
			row++;
		}
	}

	printRow(tableElement, row, 2, "Total");
	printRow(tableElement, row, 3, QString::number(totalIn, 'f', 2), Qt::AlignRight);
	printRow(tableElement, row, 4, QString::number(totalOut, 'f', 2), Qt::AlignRight);
	report.addElement(tableElement);

	QPrinter printer;
	printer.setPaperSize(QPrinter::A4);

	printer.setFullPage(false);
	printer.setOrientation(QPrinter::Portrait);

	QPrintPreviewDialog *dialog = new QPrintPreviewDialog(&printer, this);
	QObject::connect(dialog, SIGNAL(paintRequested(QPrinter*)), this, SLOT(slotPrint(QPrinter*)));
	dialog->setWindowTitle(tr("Print Document"));
	ES::MainWindowHolder::instance()->getMainWindow()->setCentralWidget(dialog);
	dialog->exec();
}
