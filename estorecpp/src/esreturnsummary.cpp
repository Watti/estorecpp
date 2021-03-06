#include "includes/esreturnsummary.h"
#include "KDReportsTableElement.h"
#include "KDReportsCell.h"
#include "KDReportsTextElement.h"
#include "QPrintPreviewDialog"
#include "utility/esmainwindowholder.h"
#include "QMainWindow"
#include "utility/esmenumanager.h"
#include "QSqlQuery"
#include "esmainwindow.h"
#include "QString"
#include "qnamespace.h"
#include "utility/session.h"
#include <iostream>
#include <fstream>
#include "QMessageBox"

ESReturnSummary::ESReturnSummary(QWidget *parent /*= 0*/) : QWidget(parent), m_report(NULL)
{
	ui.setupUi(this);

	m_detailButtonSignalMapper = new QSignalMapper(this);
	ui.pdf->hide();
	QStringList headerLabels;
	headerLabels.append("# of Bills");
	headerLabels.append("Total");
	headerLabels.append("Actions");

	ui.tableWidgetByUser->setHorizontalHeaderLabels(headerLabels);
	ui.tableWidgetByUser->horizontalHeader()->setStretchLastSection(true);
	ui.tableWidgetByUser->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tableWidgetByUser->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableWidgetByUser->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableWidgetByUser->setSelectionMode(QAbstractItemView::SingleSelection);

	ui.tableWidgetByUser->verticalHeader()->setMinimumWidth(200);

	ui.fromDate->setDate(QDate::currentDate());
	ui.toDate->setDate(QDate::currentDate().addDays(1));

	QObject::connect(ui.fromDate, SIGNAL(dateChanged(const QDate &)), this, SLOT(slotDateChanged()));
	QObject::connect(ui.toDate, SIGNAL(dateChanged(const QDate &)), this, SLOT(slotDateChanged()));
	QObject::connect(ui.generateButton, SIGNAL(clicked()), this, SLOT(slotGenerateReport()));
	QObject::connect(m_detailButtonSignalMapper, SIGNAL(mapped(QString)), this, SLOT(slotGenerateReportForGivenUser(QString)));
	//slotSearch();
}

ESReturnSummary::~ESReturnSummary()
{

}

void ESReturnSummary::slotSearch()
{
	while (ui.tableWidgetByUser->rowCount() > 0)
	{
		ui.tableWidgetByUser->removeRow(0);
	}

	double totalReturnAmount = 0.0;
	int totalBillCount = 0;

	QString stardDateStr = ui.fromDate->date().toString("yyyy-MM-dd");
	QString endDateStr = ui.toDate->date().toString("yyyy-MM-dd");

	QString fromDateStr = stardDateStr;
	int dayscount = ui.fromDate->date().daysTo(QDate::currentDate());
	bool hasPermission = (ES::Session::getInstance()->getUser()->getType() == ES::User::SENIOR_MANAGER ||
		ES::Session::getInstance()->getUser()->getType() == ES::User::DEV);
	int maxBackDays = ES::Session::getInstance()->getMaximumDaysToShowRecords();
	if (ES::Session::getInstance()->isEnableTaxSupport() && !hasPermission && dayscount > maxBackDays)
	{
		maxBackDays = maxBackDays*-1;
		QString maxBackDateStr = QDate::currentDate().addDays(maxBackDays).toString("yyyy-MM-dd");
		fromDateStr = maxBackDateStr;
	}

	QSqlQuery q("SELECT user_id, COUNT(bill_id) AS bills, SUM(return_total) AS total FROM return_item WHERE DATE(date) BETWEEN '" + fromDateStr + "' AND '" + endDateStr + "' AND deleted = 0 " + "GROUP BY(user_id)");
	while (q.next())
	{
		QString userId = q.value("user_id").toString();
		int bills = q.value("bills").toInt();
		double total = q.value("return_total").toDouble();

		int row = ui.tableWidgetByUser->rowCount();
		ui.tableWidgetByUser->insertRow(row);

		QSqlQuery userQuery("SELECT display_name FROM user WHERE user_id = " + userId);
		if (userQuery.next())
		{
			QTableWidgetItem* nameItem = new QTableWidgetItem(userQuery.value("display_name").toString());
			ui.tableWidgetByUser->setVerticalHeaderItem(row, nameItem);
		}

		QString billsStr = q.value("bills").toString();
		totalBillCount += billsStr.toInt();
		QTableWidgetItem* billsItem = new QTableWidgetItem(billsStr);
		billsItem->setTextAlignment(Qt::AlignRight);
		ui.tableWidgetByUser->setItem(row, 0, billsItem);

		double returnTotal = q.value("total").toDouble();
		totalReturnAmount += returnTotal;
		QTableWidgetItem* totalItem = new QTableWidgetItem(QString("%L1").arg(returnTotal, 0, 'f', 2));
		totalItem->setTextAlignment(Qt::AlignRight);
		ui.tableWidgetByUser->setItem(row, 1, totalItem);

		QWidget* base = new QWidget(ui.tableWidgetByUser);

		QPushButton* detailBtn = new QPushButton(base);
		detailBtn->setIcon(QIcon("icons/pdf.png"));
		detailBtn->setIconSize(QSize(24, 24));
		detailBtn->setMaximumWidth(100);

		m_detailButtonSignalMapper->setMapping(detailBtn, userId);
		QObject::connect(detailBtn, SIGNAL(clicked()), m_detailButtonSignalMapper, SLOT(map()));

		QHBoxLayout *layout = new QHBoxLayout;
		layout->setContentsMargins(0, 0, 0, 0);
		layout->addWidget(detailBtn);
		layout->insertStretch(2);
		base->setLayout(layout);
		ui.tableWidgetByUser->setCellWidget(row, 2, base);
		base->show();
	}

	ui.returnTotal->setText(QString("%L1").arg(totalReturnAmount, 0, 'f', 2));
	ui.billTotal->setText(QString::number(totalBillCount));
}

void ESReturnSummary::slotDateChanged()
{
	slotSearch();
}

void ESReturnSummary::slotGenerateReport()
{
	double totalReturnAmount = 0.0;
	int totalBillCount = 0;

	QString stardDateStr = ui.fromDate->date().toString("yyyy-MM-dd");
	QString endDateStr = ui.toDate->date().toString("yyyy-MM-dd");

	QString fromDateStr = stardDateStr;
	int dayscount = ui.fromDate->date().daysTo(QDate::currentDate());
	bool hasPermission = (ES::Session::getInstance()->getUser()->getType() == ES::User::SENIOR_MANAGER ||
		ES::Session::getInstance()->getUser()->getType() == ES::User::DEV);
	int maxBackDays = ES::Session::getInstance()->getMaximumDaysToShowRecords();
	if (ES::Session::getInstance()->isEnableTaxSupport() && !hasPermission && dayscount > maxBackDays)
	{
		maxBackDays = maxBackDays*-1;
		QString maxBackDateStr = QDate::currentDate().addDays(maxBackDays).toString("yyyy-MM-dd");
		fromDateStr = maxBackDateStr;
	}

	std::ofstream stream;
	QString filename = "";
	bool generateCSV = ui.csv->isChecked();
	if (generateCSV)
	{
		QString dateTimeStr = QDateTime::currentDateTime().toString(QLatin1String("yyyyMMdd-hhmmss"));
		QString pathToFile = ES::Session::getInstance()->getReportPath();
		filename = pathToFile.append("\\Return Summary Report-");
		filename.append(dateTimeStr).append(".csv");
		std::string s(filename.toStdString());
		stream.open(s, std::ios::out | std::ios::app);
		stream << "Return Summary Report" << "\n";
		stream << "Generated TimeStamp : ," << dateTimeStr.toLatin1().toStdString() << "\n";
		QString sDate = stardDateStr;
		QString eDate = endDateStr;
		QString dateRange = sDate.append(" - ").append(eDate);
		stream << "Date , User, Bill Id, Item, Qty, Price, Line Total" << "\n";
	}

	m_report = new KDReports::Report;

	KDReports::TextElement titleElement("Return Summary Report");
	titleElement.setPointSize(13);
	titleElement.setBold(true);
	m_report->addElement(titleElement, Qt::AlignHCenter);

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

	m_report->addElement(infoTableElement);
	m_report->addVerticalSpacing(5);

	KDReports::TableElement tableElement;
	tableElement.setHeaderColumnCount(7);
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
		KDReports::TextElement cTextElement("Bill Id");
		cTextElement.setPointSize(11);
		cTextElement.setBold(true);
		cell.addElement(cTextElement, Qt::AlignCenter);
	}
	{
		KDReports::Cell& cell = tableElement.cell(0, 3);
		KDReports::TextElement cTextElement("Item");
		cTextElement.setPointSize(11);
		cTextElement.setBold(true);
		cell.addElement(cTextElement, Qt::AlignCenter);
	}
	{
		KDReports::Cell& cell = tableElement.cell(0, 4);
		KDReports::TextElement cTextElement("Quantity");
		cTextElement.setPointSize(11);
		cTextElement.setBold(true);
		cell.addElement(cTextElement, Qt::AlignCenter);
	}

	{
		KDReports::Cell& cell = tableElement.cell(0, 5);
		KDReports::TextElement cTextElement("Price");
		cTextElement.setPointSize(11);
		cTextElement.setBold(true);
		cell.addElement(cTextElement, Qt::AlignCenter);
	}
	{
		KDReports::Cell& cell = tableElement.cell(0, 6);
		KDReports::TextElement cTextElement("Line Total");
		cTextElement.setPointSize(11);
		cTextElement.setBold(true);
		cell.addElement(cTextElement, Qt::AlignCenter);
	}
	int row = 1;
	QSqlQuery q("SELECT * FROM return_item WHERE DATE(date) BETWEEN '" + fromDateStr + "' AND '" + endDateStr + "' AND deleted = 0");
	while (q.next())
	{
		QString userId = q.value("user_id").toString();

		QSqlQuery userQuery("SELECT * FROM user JOIN usertype ON user.usertype_id = usertype.usertype_id WHERE user.active = 1 AND user.user_id = " + userId + " AND usertype.usertype_name <> 'DEV'");
		if (userQuery.next())
		{
			QString userName = userQuery.value("display_name").toString();
			QString billId = q.value("bill_id").toString();
			QString itemId = q.value("item_id").toString();
			QString qty = q.value("qty").toString();
			float subTotal = q.value("return_total").toDouble();
			totalReturnAmount += subTotal;
			float paidPrice = q.value("paid_price").toFloat();
			dateStr = q.value("date").toDateTime().date().toString("yyyy-MM-dd");

			printRow(tableElement, row, 0, dateStr);
			printRow(tableElement, row, 1, userName);
			printRow(tableElement, row, 2, billId);
			QSqlQuery queryItem("SELECT * FROM item WHERE item_id = "+itemId);
			if (queryItem.next())
			{
				printRow(tableElement, row, 3, queryItem.value("item_name").toString());
			}
			QString itemName = queryItem.value("item_name").toString();
			itemName = itemName.simplified();
			itemName.replace(" ", "");

			printRow(tableElement, row, 4, qty);
			printRow(tableElement, row, 5, QString::number(paidPrice, 'f', 2), Qt::AlignRight);
			printRow(tableElement, row, 6, QString::number(subTotal, 'f', 2), Qt::AlignRight);
			if (generateCSV)
			{
				stream << dateStr.toLatin1().data() << ", " << userName.toLatin1().data() << "," << billId.toLatin1().data() << "," <<
					itemName.toLatin1().data()<<","<<qty.toLatin1().data() << ", " << QString::number(paidPrice, 'f', 2).toLatin1().data() << "," <<
					QString::number(subTotal, 'f', 2).toLatin1().data()<<"\n";
			}
			totalBillCount++;
			row++;
		}
	}

	if (generateCSV)
	{
		stream << ",,,,,Total," << QString::number(totalReturnAmount, 'f', 2).toLatin1().data() << "\n";
		stream << ",,,,,Bill Count," << QString::number(totalBillCount).toLatin1().data() << "\n";
		stream.close();
		ui.csv->setChecked(false);
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Information);
		mbox.setText(QString("Return Item Summary Report has been saved in : ").append(filename));
		mbox.exec();
	}
	else
	{
		printRow(tableElement, row, 5, "Total", Qt::AlignRight);
		printRow(tableElement, row++, 6, QString::number(totalReturnAmount, 'f', 2), Qt::AlignRight);
		printRow(tableElement, row, 5, "Bill Count", Qt::AlignRight);
		printRow(tableElement, row, 6, QString::number(totalBillCount), Qt::AlignRight);

		m_report->addElement(tableElement);

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
}

void ESReturnSummary::printRow(KDReports::TableElement& tableElement, int row, int col, QString elementStr, Qt::AlignmentFlag alignment /*= Qt::AlignLeft*/)
{
	KDReports::Cell& cell = tableElement.cell(row, col);
	KDReports::TextElement te(elementStr);
	te.setPointSize(ES::Session::getInstance()->getBillItemFontSize());
	cell.addElement(te, alignment);
}

void ESReturnSummary::slotPrint(QPrinter* printer)
{
	m_report->print(printer);
	this->close();
}

void ESReturnSummary::slotGenerateReportForGivenUser(QString userId)
{
	m_report = new KDReports::Report;

	KDReports::TextElement titleElement("Return Item Report");
	titleElement.setPointSize(13);
	titleElement.setBold(true);
	m_report->addElement(titleElement, Qt::AlignHCenter);

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

	m_report->addElement(infoTableElement);
	m_report->addVerticalSpacing(5);

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
		KDReports::TextElement cTextElement("Bill No");
		cTextElement.setPointSize(11);
		cTextElement.setBold(true);
		cell.addElement(cTextElement, Qt::AlignCenter);
	}
	{
		KDReports::Cell& cell = tableElement.cell(0, 2);
		KDReports::TextElement cTextElement("Item");
		cTextElement.setPointSize(11);
		cTextElement.setBold(true);
		cell.addElement(cTextElement, Qt::AlignCenter);
	}
	{
		KDReports::Cell& cell = tableElement.cell(0, 3);
		KDReports::TextElement cTextElement("Qty");
		cTextElement.setPointSize(11);
		cTextElement.setBold(true);
		cell.addElement(cTextElement, Qt::AlignCenter);
	}
	{
		KDReports::Cell& cell = tableElement.cell(0, 4);
		KDReports::TextElement cTextElement("Line Total");
		cTextElement.setPointSize(11);
		cTextElement.setBold(true);
		cell.addElement(cTextElement, Qt::AlignCenter);
	}

	QString pReturnQStr("SELECT * FROM return_item WHERE user_id = " + userId + " AND  DATE(date) BETWEEN '" + stardDateStr + "' AND '" + endDateStr + "' AND deleted = 0");
	QSqlQuery qReturnItems(pReturnQStr);
	float total = 0;
	int row = 1;
	while (qReturnItems.next())
	{
		float lineTotal = qReturnItems.value("return_total").toFloat();
		total += lineTotal;
		QString dateStr = qReturnItems.value("date").toDateTime().date().toString("yyyy-MM-dd");
		QString billId = qReturnItems.value("bill_id").toString();
		QString itemId = qReturnItems.value("item_id").toString();
		printRow(tableElement, row, 0, dateStr);
		printRow(tableElement, row, 1, billId);
		QSqlQuery qItems("SELECT item_name FROM item WHERE item_id = "+itemId);
		if (qItems.next())
		{
			printRow(tableElement, row, 2, qItems.value("item_name").toString());
		}
		printRow(tableElement, row, 3, qReturnItems.value("qty").toString());
		printRow(tableElement, row, 4, QString::number(lineTotal, 'f', 2), Qt::AlignRight);
		row++;
	}

	printRow(tableElement, row,3, "Total");
	printRow(tableElement, row, 4, QString::number(total, 'f', 2), Qt::AlignRight);
	m_report->addElement(tableElement);

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
