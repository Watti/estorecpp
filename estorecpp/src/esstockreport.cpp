#include "esstockreport.h"
#include <QSqlQuery>
#include <KDReportsReport.h>
#include <KDReportsTextElement.h>
#include <KDReportsTableElement.h>
#include <KDReportsCell.h>

ESStockReport::ESStockReport(QWidget *parent /*= 0*/) : QWidget(parent)
{
	ui.setupUi(this);

	QStringList headerLabels;
	headerLabels.append("Item Code");
	headerLabels.append("Current Stock Qty");
	headerLabels.append("Minimum Qty");
	headerLabels.append("Excess");
	headerLabels.append("Re-order");

	ui.tableWidget->setHorizontalHeaderLabels(headerLabels);
	ui.tableWidget->horizontalHeader()->setStretchLastSection(true);
	ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);

	QObject::connect(ui.generateButton, SIGNAL(clicked()), this, SLOT(slotGenerate()));
	
	while (ui.tableWidget->rowCount() > 0)
	{
		ui.tableWidget->removeRow(0);
	}

	QSqlQuery q("SELECT stock.qty, stock.min_qty, item.item_code FROM stock JOIN item ON stock.item_id = item.item_id WHERE stock.deleted = 0 ");
	while (q.next())
	{
		int row = ui.tableWidget->rowCount();
		ui.tableWidget->insertRow(row);

		QColor red(245, 169, 169);
		double qty = q.value("qty").toDouble();
		double minQty = q.value("min_qty").toDouble();
		double excess = qty - minQty;

		QTableWidgetItem* item = NULL;
		item = new QTableWidgetItem(q.value("item_code").toString());
		if (excess < 0)	item->setBackgroundColor(red);
		ui.tableWidget->setItem(row, 0, item);

		item = new QTableWidgetItem(QString::number(qty, 'f', 2));
		item->setTextAlignment(Qt::AlignRight);
		if (excess < 0)	item->setBackgroundColor(red);
		ui.tableWidget->setItem(row, 1, item);

		item = new QTableWidgetItem(QString::number(minQty, 'f', 2));
		item->setTextAlignment(Qt::AlignRight);
		if (excess < 0)	item->setBackgroundColor(red);
		ui.tableWidget->setItem(row, 2, item);
				
		QString s, reorder = "No";

		if (excess > 0)
		{
			s = QString::number(excess, 'f', 2);
			s.prepend("(+) ");
		}
		else if (excess < 0)
		{
			s = QString::number(-excess, 'f', 2);
			s.prepend("(-) ");
			reorder = "Yes";
		}
		else
		{
			s = QString::number(excess, 'f', 2);
		}
		
		item = new QTableWidgetItem(s);
		item->setTextAlignment(Qt::AlignRight);
		if (excess < 0)	item->setBackgroundColor(red);
		ui.tableWidget->setItem(row, 3, item);

		item = new QTableWidgetItem(reorder);
		item->setTextAlignment(Qt::AlignCenter);
		if (excess < 0)	item->setBackgroundColor(red);
		ui.tableWidget->setItem(row, 4, item);
	}
}

ESStockReport::~ESStockReport()
{

}

void ESStockReport::slotGenerate()
{
	// Create a report
	KDReports::Report report;

	// Add a text element for the title
	KDReports::TextElement titleElement(QObject::tr("STOCK STATUS REPORT"));
	titleElement.setPointSize(15);
	report.addElement(titleElement, Qt::AlignHCenter);

	report.addVerticalSpacing(2);

	QString dateStr = "Date : ";
	dateStr.append(QDateTime::currentDateTime().toString("yyyy-MM-dd"));
	QString timeStr = "Time : ";
	timeStr.append(QDateTime::currentDateTime().toString("hh : mm"));

	KDReports::TextElement date(dateStr);
	report.addElement(date, Qt::AlignLeft);
	KDReports::TextElement time(timeStr);
	report.addElement(time, Qt::AlignLeft);

	// add 20 mm of vertical space:
	report.addVerticalSpacing(10);

	// add some more text
	//KDReports::TextElement textElement(QObject::tr("This is a report generated with KDReports"));
	//report.addElement(textElement, Qt::AlignLeft);

	KDReports::TableElement tableElement;
	tableElement.setHeaderRowCount(5);
	tableElement.setHeaderColumnCount(5);
	tableElement.setBorder(1);
	tableElement.setWidth(100, KDReports::Percent);

	//////////////////////////////////////////////////////////////////////////

	int row = 0;
	QSqlQuery q("SELECT stock.qty, stock.min_qty, item.item_code FROM stock JOIN item ON stock.item_id = item.item_id WHERE stock.deleted = 0 ");
	while (q.next())
	{
		double qty = q.value("qty").toDouble();
		double minQty = q.value("min_qty").toDouble();
		double excess = qty - minQty;

		QString itemCode = q.value("item_code").toString();
		QString qtyStr = QString::number(qty, 'f', 2);
		QString minQtyStr = QString::number(minQty, 'f', 2);
		QString s, reorder = "No";

		if (excess > 0)
		{
			s = QString::number(excess, 'f', 2);
			s.prepend("(+) ");
		}
		else if (excess < 0)
		{
			s = QString::number(-excess, 'f', 2);
			s.prepend("(-) ");
			reorder = "Yes";
		}
		else
		{
			s = QString::number(excess, 'f', 2);
		}

		KDReports::Cell& c1 = tableElement.cell(row, 0);
		KDReports::TextElement t1(itemCode);
		t1.setPointSize(12);
		if (excess < 0) t1.setTextColor(Qt::red);
		c1.addElement(t1);
		KDReports::Cell& c2 = tableElement.cell(row, 1);
		KDReports::TextElement t2(qtyStr);
		t2.setPointSize(12);
		if (excess < 0) t2.setTextColor(Qt::red);
		c2.addElement(t2);
		KDReports::Cell& c3 = tableElement.cell(row, 2);
		KDReports::TextElement t3(minQtyStr);
		t3.setPointSize(12);
		if (excess < 0) t3.setTextColor(Qt::red);
		c3.addElement(t3);
		KDReports::Cell& c4 = tableElement.cell(row, 3);
		KDReports::TextElement t4(s);
		t4.setPointSize(12);
		if (excess < 0) t4.setTextColor(Qt::red);
		c4.addElement(t4);
		KDReports::Cell& c5 = tableElement.cell(row, 4);
		KDReports::TextElement t5(reorder);
		t5.setPointSize(12);
		if (excess < 0) t5.setTextColor(Qt::red);
		c5.addElement(t5);

		row++;
	}

	report.addElement(tableElement);

	QPrinter printer;
	printer.setOutputFormat(QPrinter::PdfFormat);
	printer.setOutputFileName("../stock_status_report.pdf");

	report.print(&printer, 0);
}
