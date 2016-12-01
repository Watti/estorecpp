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
#include "esitemwisesalessummary.h"
#include "utility/utility.h"
#include <iostream>
#include <fstream>
#include "QMessageBox"

ESItemWiseSalesSummary::ESItemWiseSalesSummary(QWidget *parent /*= 0*/) : QWidget(parent)
{
	ui.setupUi(this);


	QStringList headerLabels;
	headerLabels.append("Item Code");
	headerLabels.append("Item");
	headerLabels.append("Sold Qty");
	headerLabels.append("Returned Qty");
	//headerLabels.append("Line Total");

	ui.tableWidgetByItems->setHorizontalHeaderLabels(headerLabels);
	ui.tableWidgetByItems->horizontalHeader()->setStretchLastSection(true);
	ui.tableWidgetByItems->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tableWidgetByItems->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableWidgetByItems->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableWidgetByItems->setSelectionMode(QAbstractItemView::SingleSelection);

	ui.fromDate->setDate(QDate::currentDate());
	ui.toDate->setDate(QDate::currentDate().addDays(1));

	QObject::connect(ui.fromDate, SIGNAL(dateChanged(const QDate &)), this, SLOT(slotDateChanged()));
	QObject::connect(ui.toDate, SIGNAL(dateChanged(const QDate &)), this, SLOT(slotDateChanged()));
	QObject::connect(ui.generateButton, SIGNAL(clicked()), this, SLOT(slotGenerateReport()));
	//slotSearch();
}

ESItemWiseSalesSummary::~ESItemWiseSalesSummary()
{

}

void ESItemWiseSalesSummary::slotSearch()
{
	while (ui.tableWidgetByItems->rowCount() > 0)
	{
		ui.tableWidgetByItems->removeRow(0);
	}

	int itemCount = 0;

	QString stardDateStr = ui.fromDate->date().toString("yyyy-MM-dd");
	QString endDateStr = ui.toDate->date().toString("yyyy-MM-dd");
	//QString stardDateStr = QDate::currentDate().addDays(-14).toString("yyyy-MM-dd");
	QSqlQuery q("SELECT stock_id, SUM(quantity) as totalQty, item_price, discount FROM sale JOIN bill ON sale.bill_id = bill.bill_id WHERE sale.deleted = 0 AND DATE(bill.date) BETWEEN '" + stardDateStr + "' AND '" + endDateStr + "'" + "GROUP BY stock_id");
	while (q.next())
	{
		itemCount++;
		float discount = q.value("discount").toFloat();
		QString stock_id = q.value("stock_id").toString();
		QString qty = q.value("totalQty").toString();
		QString price = QString::number(q.value("item_price").toFloat(),'f',2);
		float grandtotal = (price.toFloat() * ((100 - discount) / 100))*qty.toFloat();
		QString totalPrice = QString::number(grandtotal,'f',2);
		QString name = "";
		QString itemCode = "";

		int row = ui.tableWidgetByItems->rowCount();
		ui.tableWidgetByItems->insertRow(row);

		QSqlQuery stockItemQry("SELECT * FROM stock JOIN item ON stock.item_id = item.item_id WHERE stock.stock_id = " + stock_id);
		if (stockItemQry.next())
		{
			name = stockItemQry.value("item_name").toString();
			itemCode = stockItemQry.value("item_code").toString();
			QString itemId = stockItemQry.value("item_id").toString();
			QString qryStrReturn("SELECT SUM(qty) as retTotal FROM return_item WHERE item_id = " + itemId + " AND Date(date) BETWEEN '" + stardDateStr + "' AND '" + endDateStr + "'");
			QSqlQuery qryReturn;
			if (qryReturn.exec(qryStrReturn))
			{
				if (qryReturn.next())
				{
					float retQty = qryReturn.value("retTotal").toFloat();
					float netSoldQty = qty.toFloat() - retQty;
					//qty = QString::number(netSoldQty, 'f', 2);

					QTableWidgetItem* returnedItem = new QTableWidgetItem(QString::number(retQty));
					returnedItem->setTextAlignment(Qt::AlignRight);
					ui.tableWidgetByItems->setItem(row, 3, returnedItem);
				}
			}
		}

		QTableWidgetItem* iCode = new QTableWidgetItem(itemCode);
		iCode->setTextAlignment(Qt::AlignLeft);
		ui.tableWidgetByItems->setItem(row, 0, iCode);

		QTableWidgetItem* itemName = new QTableWidgetItem(name);
		itemName->setTextAlignment(Qt::AlignLeft);
		ui.tableWidgetByItems->setItem(row, 1, itemName);

		QTableWidgetItem* qtyItem = new QTableWidgetItem(qty);
		qtyItem->setTextAlignment(Qt::AlignRight);
		ui.tableWidgetByItems->setItem(row, 2, qtyItem);
	}

	ui.itemTotal->setText(QString::number(itemCount));
}

void ESItemWiseSalesSummary::slotDateChanged()
{
	slotSearch();
}

void ESItemWiseSalesSummary::slotGenerateReport()
{
	double totalReturnAmount = 0.0;
	int totalBillCount = 0;

	QString stardDateStr = ui.fromDate->date().toString("yyyy-MM-dd");
	QString endDateStr = ui.toDate->date().toString("yyyy-MM-dd");

	KDReports::TextElement titleElement("Stock Item Wise Sales Summary Report");
	titleElement.setPointSize(13);
	titleElement.setBold(true);
	report.addElement(titleElement, Qt::AlignHCenter);

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
	tableElement.setHeaderColumnCount(4);
	tableElement.setBorder(1);
	tableElement.setWidth(100, KDReports::Percent);
	{
		KDReports::Cell& cell = tableElement.cell(0, 0);
		KDReports::TextElement cTextElement("Item Code");
		cTextElement.setPointSize(11);
		cTextElement.setBold(true);
		cell.addElement(cTextElement, Qt::AlignCenter);
	}
	{
		KDReports::Cell& cell = tableElement.cell(0, 1);
		KDReports::TextElement cTextElement("Item");
		cTextElement.setPointSize(11);
		cTextElement.setBold(true);
		cell.addElement(cTextElement, Qt::AlignCenter);
	}
	{
		KDReports::Cell& cell = tableElement.cell(0, 2);
		KDReports::TextElement cTextElement("Sold Qty");
		cTextElement.setPointSize(11);
		cTextElement.setBold(true);
		cell.addElement(cTextElement, Qt::AlignCenter);
	}
	{
		KDReports::Cell& cell = tableElement.cell(0, 3);
		KDReports::TextElement cTextElement("Returned Qty");
		cTextElement.setPointSize(11);
		cTextElement.setBold(true);
		cell.addElement(cTextElement, Qt::AlignCenter);
	}
	std::ofstream stream;
	QString filename = "";
	bool generateCSV = ui.csv->isChecked();
	if (generateCSV)
	{
		QString dateTimeStr = QDateTime::currentDateTime().toString(QLatin1String("yyyyMMdd-hhmmss"));
		QString pathToFile = ES::Session::getInstance()->getReportPath();
		filename = pathToFile.append("\\Item Wise Sales Summary Report -");
		filename.append(dateTimeStr).append(".csv");
		std::string s(filename.toStdString());
		stream.open(s, std::ios::out | std::ios::app);
		stream << "Item Wise Sales Summary Report" << "\n";
		stream << "Generated Date Time : ," << dateTimeStr.toLatin1().toStdString() << "\n";
		stream << ", ,From : " << stardDateStr.toLatin1().toStdString() << ", To : "<< endDateStr.toLatin1().toStdString()<<"\n\n";
		stream << "Code , Item, Sold Qty, Returned Qty" << "\n";

	}
	int row = 1;
	int itemCount = 0;
	QSqlQuery q("SELECT stock_id, SUM(quantity) as totalQty, item_price, discount FROM sale JOIN bill ON sale.bill_id = bill.bill_id WHERE sale.deleted = 0 AND DATE(bill.date) BETWEEN '" + stardDateStr + "' AND '" + endDateStr + "'" + "GROUP BY stock_id");
	while (q.next())
	{
		itemCount++;
		float discount = q.value("discount").toFloat();
		QString stock_id = q.value("stock_id").toString();
		QString qty = q.value("totalQty").toString();
		QString price = QString::number(q.value("item_price").toFloat(), 'f', 2);
		float grandtotal = (price.toFloat() * ((100 - discount) / 100))*qty.toFloat();
		QString totalPrice = QString::number(grandtotal, 'f', 2);
		QString name = "", itemCode = "";
		QSqlQuery stockItemQry("SELECT * FROM stock JOIN item ON stock.item_id = item.item_id WHERE stock.stock_id = " + stock_id);
		if (stockItemQry.next())
		{
			name = stockItemQry.value("item_name").toString();
			name = name.simplified();
			name.replace(" ", "");
			itemCode = stockItemQry.value("item_code").toString();
			ES::Utility::printRow(tableElement, row, 0, itemCode);
			ES::Utility::printRow(tableElement, row, 1, name);
			ES::Utility::printRow(tableElement, row, 2, qty);
			float retQty = 0;
			QString itemId = stockItemQry.value("item_id").toString();
			QString qryStrReturn("SELECT SUM(qty) as retTotal FROM return_item WHERE item_id = " + itemId + " AND DATE(date) BETWEEN '" + stardDateStr + "' AND '" + endDateStr + "'");
			QSqlQuery qryReturn;
			if (qryReturn.exec(qryStrReturn))
			{
				if (qryReturn.next())
				{
					retQty = qryReturn.value("retTotal").toFloat();
					float netSoldQty = qty.toFloat() - retQty;
					//qty = QString::number(netSoldQty, 'f', 2);
					ES::Utility::printRow(tableElement, row, 3, QString::number(retQty));
				}
			}

			if (generateCSV)
			{
				stream << itemCode.toLatin1().data() << ", " << name.toLatin1().data() << ", " << qty.toLatin1().data() << ", " << QString::number(retQty).toLatin1().data()<<"\n";
			}
			row++;
		}
	}
	
	if (generateCSV)
	{
		stream.close();
		ui.csv->setChecked(false);
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Information);
		mbox.setText(QString("Stock Items Sales Report has been saved in : ").append(filename));
		mbox.exec();
	}
	else
	{
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
}

void ESItemWiseSalesSummary::slotPrint(QPrinter* printer)
{
	report.print(printer);
	this->close();
}

