#include <QSqlQuery>
#include <QPrintPreviewWidget>
#include <QPrintPreviewDialog>
#include <KDReportsReport.h>
#include <KDReportsTextElement.h>
#include <KDReportsTableElement.h>
#include <KDReportsCell.h>
#include "utility\session.h"
#include "qlogging.h"
#include <QTextEdit>
#include "utility/esmainwindowholder.h"
#include "esmainwindow.h"
#include "overallstockitemreport.h"
#include "utility/utility.h"

ESOverallStockItemReport::ESOverallStockItemReport(QWidget *parent /*= 0*/) : QWidget(parent),
m_startingLimit(0), m_pageOffset(50), m_nextCounter(0), m_maxNextCount(0), m_report(NULL)
{
	ui.setupUi(this);

	QStringList headerLabels;
	headerLabels.append("Item Code");
	headerLabels.append("Item");
	headerLabels.append("Current Stock Qty");
	headerLabels.append("Minimum Qty");
	headerLabels.append("Floor");
	headerLabels.append("Stock Value");

	ui.tableWidget->setHorizontalHeaderLabels(headerLabels);
	ui.tableWidget->horizontalHeader()->setStretchLastSection(true);
	ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);

	QObject::connect(ui.nextBtn, SIGNAL(clicked()), this, SLOT(slotNext()));
	QObject::connect(ui.prevBtn, SIGNAL(clicked()), this, SLOT(slotPrev()));
	QObject::connect(ui.generateButton, SIGNAL(clicked()), this, SLOT(slotGenerate()));

	ui.prevBtn->setDisabled(true);
	ui.nextBtn->setDisabled(true);

	displayResults();

}

ESOverallStockItemReport::~ESOverallStockItemReport()
{

}

void ESOverallStockItemReport::slotGenerate()
{
	//	QString reportName = "stock_status_report_";
	//QSqlQuery qq("SELECT * FROM report_text WHERE report_name = 'stock_status_report'");
	//report = new KDReports::Report;
	//if (qq.next())
	// Create a report

	// 		int reportNo = qq.value("report_number").toInt();
	// 		reportNo++;
	// 
	// 		reportName.append(QString::number(reportNo).rightJustified(8, '0'));
	// 
	// 		QSqlQuery qUpdate("UPDATE report_text SET report_number = " + QString::number(reportNo) + " WHERE report_name = 'stock_status_report'");
	// 
	// 		QString reportNoStr = qq.value("report_number_text").toString() + " : " + qq.value("report_no_prefix").toString() + QString::number(reportNo).rightJustified(8, '0');
	// 		KDReports::TextElement rpNo(reportNoStr);
	// 		report.addElement(rpNo, Qt::AlignLeft);
	// 		report.addVerticalSpacing(1);

	// Add a text element for the title
	m_report = new KDReports::Report;
	KDReports::TextElement titleElement("Stock Items Report");
	titleElement.setPointSize(15);
	m_report->addElement(titleElement, Qt::AlignHCenter);

	m_report->addVerticalSpacing(2);

	QString dateStr = "Date : ";
	dateStr.append(QDateTime::currentDateTime().toString("yyyy-MM-dd"));
	QString timeStr = "Time : ";
	timeStr.append(QDateTime::currentDateTime().toString("hh : mm"));

	KDReports::TextElement date(dateStr);
	m_report->addElement(date, Qt::AlignLeft);
	KDReports::TextElement time(timeStr);
	m_report->addElement(time, Qt::AlignLeft);

	// add 20 mm of vertical space:
	m_report->addVerticalSpacing(10);

	KDReports::TableElement tableElement;
	tableElement.setHeaderRowCount(5);
	tableElement.setHeaderColumnCount(6);
	tableElement.setBorder(1);
	tableElement.setWidth(100, KDReports::Percent);

	//////////////////////////////////////////////////////////////////////////

	int row = 0;
	{
		KDReports::Cell& c1 = tableElement.cell(row, 0);
		KDReports::TextElement t1("Code");
		t1.setPointSize(12);
		c1.addElement(t1);

		KDReports::Cell& c21 = tableElement.cell(row, 1);
		KDReports::TextElement t21("Item");
		t21.setPointSize(12);
		c21.addElement(t21);

		KDReports::Cell& c2 = tableElement.cell(row, 2);
		KDReports::TextElement t2("Qty");
		t2.setPointSize(12);
		c2.addElement(t2);

		KDReports::Cell& c3 = tableElement.cell(row, 3);
		KDReports::TextElement t3("Min. Qty");
		t3.setPointSize(12);
		c3.addElement(t3);

		KDReports::Cell& c4 = tableElement.cell(row, 4);
		KDReports::TextElement t4("Floor");
		t4.setPointSize(12);
		c4.addElement(t4);

		KDReports::Cell& c5 = tableElement.cell(row, 5);
		KDReports::TextElement t5("Stock Value");
		t5.setPointSize(12);
		c5.addElement(t5);
	}
	row++;
	QString qStr;
	if (ES::Session::getInstance()->getUser()->getType() == ES::User::SENIOR_MANAGER ||
		ES::Session::getInstance()->getUser()->getType() == ES::User::DEV)
	{
		qStr = "SELECT stock.qty, stock.min_qty, stock.floor, stock.selling_price, item.item_code, item.item_name FROM stock JOIN item ON stock.item_id = item.item_id WHERE stock.deleted = 0";
	}
	else
	{
		qStr = "SELECT stock.qty, stock.min_qty, stock.floor, stock.selling_price, item.item_code, item.item_name FROM stock JOIN item ON stock.item_id = item.item_id WHERE stock.deleted = 0 AND stock.visible = 1";
	}
	//pagination start
	qStr.append(" LIMIT ").append(QString::number(m_startingLimit));
	qStr.append(" , ").append(QString::number(m_pageOffset));
	//pagination end
	QSqlQuery q(qStr);
	while (q.next())
	{

		double qty = q.value("qty").toDouble();
		double minQty = q.value("min_qty").toDouble();
		double excess = qty - minQty;
		double sellingPrice = q.value("selling_price").toDouble();

		QString itemCode = q.value("item_code").toString();
		QString itemName = q.value("item_name").toString();
		QString floorNo = q.value("floor").toString();
		QString qtyStr = QString::number(qty, 'f', 2);
		QString minQtyStr = QString::number(minQty, 'f', 2);

		ES::Utility::printRow(tableElement, row, 0, itemCode);
		ES::Utility::printRow(tableElement, row, 1, itemName);
		ES::Utility::printRow(tableElement, row, 2, qtyStr);
		ES::Utility::printRow(tableElement, row, 3, minQtyStr);
		ES::Utility::printRow(tableElement, row, 4, floorNo);

		double stockValue = sellingPrice* qty;
		ES::Utility::printRow(tableElement, row, 5, QString::number(stockValue, 'f', 2));
		row++;
	}

	m_report->addElement(tableElement);

	QPrinter printer;
	//printer.setOutputFormat(QPrinter::PdfFormat);
	//printer.setOutputFileName("reports/" + reportName + ".pdf");
	printer.setPaperSize(QPrinter::A4);

	printer.setFullPage(false);
	//printer.setResolution(QPrinter::HighResolution);
	printer.setOrientation(QPrinter::Portrait);

	QPrintPreviewDialog *dialog = new QPrintPreviewDialog(&printer, this);
	QObject::connect(dialog, SIGNAL(paintRequested(QPrinter*)), this, SLOT(slotPrint(QPrinter*)));
	dialog->setWindowTitle(tr("Print Document"));
	ES::MainWindowHolder::instance()->getMainWindow()->setCentralWidget(dialog);
	dialog->exec();
}

void ESOverallStockItemReport::slotPrint(QPrinter* printer)
{
	m_report->print(printer);
}

void ESOverallStockItemReport::displayResults()
{
	while (ui.tableWidget->rowCount() > 0)
	{
		ui.tableWidget->removeRow(0);
	}
	QString qStr, qRecordCountStr;
	if (ES::Session::getInstance()->getUser()->getType() == ES::User::SENIOR_MANAGER ||
		ES::Session::getInstance()->getUser()->getType() == ES::User::DEV)
	{
		qStr = "SELECT stock.qty, stock.min_qty, stock.floor, item.item_code, item.item_name FROM stock JOIN item ON stock.item_id = item.item_id WHERE stock.deleted = 0";
		qRecordCountStr = "SELECT COUNT(*) as c FROM stock JOIN item ON stock.item_id = item.item_id WHERE stock.deleted = 0";
	}
	else
	{
		qStr = "SELECT stock.qty, stock.min_qty, stock.floor, stock.selling_price, item.item_code, item.item_name FROM stock JOIN item ON stock.item_id = item.item_id WHERE stock.deleted = 0  AND stock.visible = 1";
		qRecordCountStr = "SELECT COUNT(*) as c FROM stock JOIN item ON stock.item_id = item.item_id WHERE stock.deleted = 0  AND stock.visible = 1";
	}
	QSqlQuery queryRecordCount(qRecordCountStr);
	if (queryRecordCount.next())
	{
		m_totalRecords = queryRecordCount.value("c").toInt();
	}
	//pagination start
	qStr.append(" LIMIT ").append(QString::number(m_startingLimit));
	qStr.append(" , ").append(QString::number(m_pageOffset));
	//pagination end

	QSqlQuery q;

	if (q.exec(qStr))
	{
		//pagination start
		m_maxNextCount = m_totalRecords / m_pageOffset;
		if (m_maxNextCount > m_nextCounter)
		{
			ui.nextBtn->setEnabled(true);
		}
		int currentlyShowdItemCount = (m_nextCounter + 1)*m_pageOffset;
		if (currentlyShowdItemCount >= m_totalRecords)
		{
			ui.nextBtn->setDisabled(true);
		}
		//pagination end

		while (q.next())
		{
			int row = ui.tableWidget->rowCount();
			ui.tableWidget->insertRow(row);

			QColor red(245, 169, 169);
			double qty = q.value("qty").toDouble();
			double minQty = q.value("min_qty").toDouble();
			double sellingPrice = q.value("selling_price").toDouble();
			double excess = qty - minQty;
			
			QTableWidgetItem* item = NULL;
			item = new QTableWidgetItem(q.value("item_code").toString());
			item->setTextAlignment(Qt::AlignLeft);
			ui.tableWidget->setItem(row, 0, item);

			item = new QTableWidgetItem(q.value("item_name").toString());
			item->setTextAlignment(Qt::AlignLeft);
			ui.tableWidget->setItem(row, 1, item);

			item = new QTableWidgetItem(QString::number(qty));
			item->setTextAlignment(Qt::AlignRight);
			ui.tableWidget->setItem(row, 2, item);

			item = new QTableWidgetItem(QString::number(minQty));
			item->setTextAlignment(Qt::AlignRight);
			ui.tableWidget->setItem(row, 3, item);

			item = new QTableWidgetItem(q.value("floor").toString());
			item->setTextAlignment(Qt::AlignRight);
			ui.tableWidget->setItem(row, 4, item);

			double stockValue = sellingPrice* qty;
			item = new QTableWidgetItem(QString::number(stockValue,'f',2));
			item->setTextAlignment(Qt::AlignRight);
			ui.tableWidget->setItem(row, 5, item);
		}
	}
}

void ESOverallStockItemReport::slotPrev()
{
	if (m_nextCounter == 1)
	{
		ui.prevBtn->setDisabled(true);
	}
	if (m_nextCounter > 0)
	{
		m_nextCounter--;
		m_startingLimit -= m_pageOffset;
		ui.nextBtn->setEnabled(true);
	}
	displayResults();
}

void ESOverallStockItemReport::slotNext()
{
	if (m_nextCounter < m_maxNextCount)
	{
		m_nextCounter++;
		ui.prevBtn->setEnabled(true);
		m_startingLimit += m_pageOffset;
	}
	displayResults();
}
