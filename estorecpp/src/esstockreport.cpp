#include "esstockreport.h"
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
#include "QMessageBox"
#include "utility/esdbconnection.h"
#include "utility/utility.h"
#include <iostream>
#include <fstream>

ESStockReport::ESStockReport(QWidget *parent /*= 0*/) : QWidget(parent), m_report(NULL)
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
	QObject::connect(ui.categoryCombo, SIGNAL(activated(QString)), this, SLOT(displayResults()));
	if (!ES::DbConnection::instance()->open())
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Critical);
		mbox.setText(QString("Cannot connect to the database : ESStockReport::displayStockItems "));
		mbox.exec();
	}
	else
	{
		QSqlQuery queryCategory("SELECT * FROM item_category WHERE deleted = 0 ORDER BY itemcategory_name");
		QString catCode = "select";
		int catId = -1;

		ui.categoryCombo->addItem(catCode, catId);

		while (queryCategory.next())
		{
			catId = queryCategory.value("itemcategory_id").toInt();
			ui.categoryCombo->addItem(queryCategory.value("itemcategory_name").toString(), catId);
		}
	}
	displayResults();

}

ESStockReport::~ESStockReport()
{

}

void ESStockReport::slotGenerate()
{
	QString reportName = "stock_status_report_";
	QSqlQuery qq("SELECT * FROM report_text WHERE report_name = 'stock_status_report'");

	//if (qq.next())
	{
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
		KDReports::TextElement titleElement("Stock Item Re-Order Report");
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
		tableElement.setHeaderRowCount(1);
		tableElement.setHeaderColumnCount(4);
		tableElement.setBorder(1);
		tableElement.setWidth(100, KDReports::Percent);

		//////////////////////////////////////////////////////////////////////////

		int row = 0;
		QString cols = qq.value("columns").toString();
		qDebug() << cols;
		QStringList colList = cols.split(",");

		KDReports::Cell& c1 = tableElement.cell(row, 0);
		KDReports::TextElement t1("Code");
		t1.setPointSize(12);
		//t1.setTextColor(Qt::gray);
		c1.addElement(t1);

		KDReports::Cell& c21 = tableElement.cell(row, 1);
		KDReports::TextElement t21("Item");
		t21.setPointSize(12);
		//t2.setTextColor(Qt::gray);
		c21.addElement(t21);

		KDReports::Cell& c2 = tableElement.cell(row, 2);
		KDReports::TextElement t2("Qty");
		t2.setPointSize(12);
		//t2.setTextColor(Qt::gray);
		c2.addElement(t2);

		KDReports::Cell& c3 = tableElement.cell(row, 3);
		KDReports::TextElement t3("Min. Qty");
		t3.setPointSize(12);
		//t3.setTextColor(Qt::gray);
		c3.addElement(t3);

		KDReports::Cell& c4 = tableElement.cell(row, 4);
		KDReports::TextElement t4("Status");
		t4.setPointSize(12);
		//t4.setTextColor(Qt::gray);
		c4.addElement(t4);

		std::ofstream stream;
		QString filename = "";
		bool generateCSV = ui.csv->isChecked();
		if (generateCSV)
		{
			QString dateTimeStr = QDateTime::currentDateTime().toString(QLatin1String("yyyyMMdd-hhmmss"));
			QString pathToFile = ES::Session::getInstance()->getReportPath();
			filename = pathToFile.append("\\Stock Items Re-Order Report-");
			filename.append(dateTimeStr).append(".csv");
			std::string s(filename.toStdString());
			stream.open(s, std::ios::out | std::ios::app);
			stream << "Stock Items Re-Order Report" << "\n";
			stream << "Date Time : ," << dateTimeStr.toLatin1().toStdString() << "\n";
			stream << "Code , Item, Existing Qty, Min Qty, Status" << "\n";

		}
		int categoryId = ui.categoryCombo->currentData().toInt();
		bool categorySelected = false;
		if (categoryId != -1)
		{
			categorySelected = true;
		}
		row++;
		QString maxRows = ui.maxRows->text();
		QString qStr;
		if (ES::Session::getInstance()->getUser()->getType() == ES::User::SENIOR_MANAGER ||
			ES::Session::getInstance()->getUser()->getType() == ES::User::DEV)
		{

			qStr = "SELECT item.item_name, stock.qty, stock.min_qty, item.item_code FROM stock JOIN item ON stock.item_id = item.item_id JOIN item_category ON item.itemcategory_id = item_category.itemcategory_id WHERE stock.deleted = 0 AND item.deleted = 0 AND item_category.deleted =0 ";

			if (categorySelected)
			{
				qStr.append(" AND item_category.itemcategory_id = " + QString::number(categoryId));
			}
			qStr.append(" AND stock.qty <= stock.min_qty LIMIT " + maxRows);
		}
		else
		{
			qStr = "SELECT item.item_name, stock.qty, stock.min_qty, item.item_code FROM stock JOIN item ON stock.item_id = item.item_id JOIN item_category ON item.itemcategory_id = item_category.itemcategory_id WHERE stock.deleted = 0 AND item.deleted = 0 AND item_category.deleted =0 AND stock.visible = 1 ";
			//qStr = "SELECT stock.qty, stock.min_qty, item.item_code FROM stock JOIN item ON stock.item_id = item.item_id WHERE stock.deleted = 0 AND item.deleted = 0 AND stock.visible = 1 AND stock.qty <= stock.min_qty LIMIT " + maxRows;

			if (categorySelected)
			{
				qStr.append(" AND item_category.itemcategory_id = " + QString::number(categoryId));
			}
			qStr.append(" AND stock.qty <= stock.min_qty LIMIT " + maxRows);
		}
		QSqlQuery q(qStr);
		while (q.next())
		{

			double qty = q.value("qty").toDouble();
			double minQty = q.value("min_qty").toDouble();
			double excess = qty - minQty;

			QString itemCode = q.value("item_code").toString();
			QString itemName = q.value("item_name").toString();
			QString qtyStr = QString::number(qty);
			QString minQtyStr = QString::number(minQty);
			QString s, reorder = "No";
			itemName = itemName.simplified();
			itemName.replace(" ", "");
			if (excess > 0)
			{
				s = QString::number(excess);
				s.prepend("(+) ");
			}
			else if (excess < 0)
			{
				s = QString::number(-excess);
				s.prepend("(-) ");
				reorder = "Yes";
			}
			else
			{
				s = QString::number(excess);
			}

			KDReports::Cell& c1 = tableElement.cell(row, 0);
			KDReports::TextElement t1(itemCode);
			t1.setPointSize(12);
			//if (excess < 0) t1.setTextColor(Qt::red);
			c1.addElement(t1);

			KDReports::Cell& c21 = tableElement.cell(row, 1);
			KDReports::TextElement t21(itemName);
			t21.setPointSize(12);
			//if (excess < 0) t21.setTextColor(Qt::red);
			c21.addElement(t21);

			KDReports::Cell& c2 = tableElement.cell(row, 2);
			KDReports::TextElement t2(qtyStr);
			t2.setPointSize(12);
			//if (excess < 0) t2.setTextColor(Qt::red);
			c2.addElement(t2);
			KDReports::Cell& c3 = tableElement.cell(row, 3);
			KDReports::TextElement t3(minQtyStr);
			t3.setPointSize(12);
			//if (excess < 0) t3.setTextColor(Qt::red);
			c3.addElement(t3);
			KDReports::Cell& c4 = tableElement.cell(row, 4);
			KDReports::TextElement t4(s);
			t4.setPointSize(12);
			//if (excess < 0) t4.setTextColor(Qt::red);
			c4.addElement(t4);

			if (generateCSV)
			{
				stream << itemCode.toLatin1().data() << ", " << itemName.toLatin1().data() << ", " << qtyStr.toLatin1().data() << ", " << minQtyStr.toLatin1().data()<< ", " << s.toLatin1().data() << "\n";
			}
			row++;
		}
		if (generateCSV)
		{
			stream.close();
			ui.csv->setChecked(false);
			QMessageBox mbox;
			mbox.setIcon(QMessageBox::Information);
			mbox.setText(QString("Stock Items Re-Order Report has been saved in : ").append(filename));
			mbox.exec();
		}
		{
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
}

void ESStockReport::slotPrint(QPrinter* printer)
{
	// 	QPainter painter(printer);
	// 	painter.setRenderHints(QPainter::Antialiasing |
	// 		QPainter::TextAntialiasing |
	// 		QPainter::SmoothPixmapTransform, true);
	// 
	// 	report.paintPage(1, painter);
	m_report->print(printer);
	//report.p
}

void ESStockReport::displayResults()
{
	while (ui.tableWidget->rowCount() > 0)
	{
		ui.tableWidget->removeRow(0);
	}

	int categoryId = ui.categoryCombo->currentData().toInt();
	bool categorySelected = false;
	if (categoryId != -1)
	{
		categorySelected = true;
	}
	QString maxRows = ui.maxRows->text();
	QString qStr;

	if (ES::Session::getInstance()->getUser()->getType() == ES::User::SENIOR_MANAGER ||
		ES::Session::getInstance()->getUser()->getType() == ES::User::DEV)
	{
		
		qStr = "SELECT item.item_name, stock.qty, stock.min_qty, item.item_code FROM stock JOIN item ON stock.item_id = item.item_id JOIN item_category ON item.itemcategory_id = item_category.itemcategory_id WHERE stock.deleted = 0 AND item.deleted = 0 AND item_category.deleted =0 ";

		if (categorySelected)
		{
			qStr.append(" AND item_category.itemcategory_id = "+QString::number(categoryId));
		}
		qStr.append(" AND stock.qty <= stock.min_qty LIMIT " + maxRows);
	}
	else
	{
		qStr = "SELECT item.item_name, stock.qty, stock.min_qty, item.item_code FROM stock JOIN item ON stock.item_id = item.item_id JOIN item_category ON item.itemcategory_id = item_category.itemcategory_id WHERE stock.deleted = 0 AND item.deleted = 0 AND item_category.deleted =0 AND stock.visible = 1 ";
		//qStr = "SELECT stock.qty, stock.min_qty, item.item_code FROM stock JOIN item ON stock.item_id = item.item_id WHERE stock.deleted = 0 AND item.deleted = 0 AND stock.visible = 1 AND stock.qty <= stock.min_qty LIMIT " + maxRows;

		if (categorySelected)
		{
			qStr.append(" AND item_category.itemcategory_id = " + QString::number(categoryId));
		}
		qStr.append(" AND stock.qty <= stock.min_qty LIMIT " + maxRows);
	}
	QSqlQuery q(qStr);
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

		item = new QTableWidgetItem(QString::number(qty));
		item->setTextAlignment(Qt::AlignRight);
		if (excess < 0)	item->setBackgroundColor(red);
		ui.tableWidget->setItem(row, 1, item);

		item = new QTableWidgetItem(QString::number(minQty));
		item->setTextAlignment(Qt::AlignRight);
		if (excess < 0)	item->setBackgroundColor(red);
		ui.tableWidget->setItem(row, 2, item);

		QString s, reorder = "No";

		if (excess > 0)
		{
			s = QString::number(excess);
			s.prepend("(+) ");
		}
		else if (excess < 0)
		{
			s = QString::number(-excess);
			s.prepend("(-) ");
			reorder = "Yes";
		}
		else
		{
			s = QString::number(excess);
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
