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

ESItemWiseSalesSummary::ESItemWiseSalesSummary(QWidget *parent /*= 0*/) : QWidget(parent)
{
	ui.setupUi(this);


	QStringList headerLabels;
	headerLabels.append("Item");
	headerLabels.append("Qty");
	headerLabels.append("Unit Price");
	headerLabels.append("Discount");
	headerLabels.append("Line Total");

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

		int row = ui.tableWidgetByItems->rowCount();
		ui.tableWidgetByItems->insertRow(row);

		QSqlQuery stockItemQry("SELECT * FROM stock JOIN item ON stock.item_id = item.item_id WHERE stock.stock_id = " + stock_id);
		if (stockItemQry.next())
		{
			name = stockItemQry.value("item_name").toString();
		}
		QTableWidgetItem* itemName = new QTableWidgetItem(name);
		itemName->setTextAlignment(Qt::AlignLeft);
		ui.tableWidgetByItems->setItem(row, 0, itemName);

		QTableWidgetItem* qtyItem = new QTableWidgetItem(qty);
		qtyItem->setTextAlignment(Qt::AlignRight);
		ui.tableWidgetByItems->setItem(row, 1, qtyItem);

		QTableWidgetItem* priceItem = new QTableWidgetItem(price);
		priceItem->setTextAlignment(Qt::AlignRight);
		ui.tableWidgetByItems->setItem(row, 2, priceItem);

		QTableWidgetItem* discountItem = new QTableWidgetItem(QString::number(discount,'f',2));
		discountItem->setTextAlignment(Qt::AlignRight);
		ui.tableWidgetByItems->setItem(row, 3, discountItem);

		QTableWidgetItem* totalPriceItem = new QTableWidgetItem(totalPrice);
		totalPriceItem->setTextAlignment(Qt::AlignRight);
		ui.tableWidgetByItems->setItem(row, 4, totalPriceItem);
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

	KDReports::TextElement titleElement("Stock Item Sales Report");
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
	tableElement.setHeaderColumnCount(5);
	tableElement.setBorder(1);
	tableElement.setWidth(100, KDReports::Percent);

	{
		KDReports::Cell& cell = tableElement.cell(0, 0);
		KDReports::TextElement cTextElement("Item");
		cTextElement.setPointSize(11);
		cTextElement.setBold(true);
		cell.addElement(cTextElement, Qt::AlignCenter);
	}
	{
		KDReports::Cell& cell = tableElement.cell(0, 1);
		KDReports::TextElement cTextElement("Qty");
		cTextElement.setPointSize(11);
		cTextElement.setBold(true);
		cell.addElement(cTextElement, Qt::AlignCenter);
	}
	{
		KDReports::Cell& cell = tableElement.cell(0, 2);
		KDReports::TextElement cTextElement("Unit Price");
		cTextElement.setPointSize(11);
		cTextElement.setBold(true);
		cell.addElement(cTextElement, Qt::AlignCenter);
	}
	{
		KDReports::Cell& cell = tableElement.cell(0, 3);
		KDReports::TextElement cTextElement("Discount");
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
		QString name = "";

		QSqlQuery stockItemQry("SELECT * FROM stock JOIN item ON stock.item_id = item.item_id WHERE stock.stock_id = " + stock_id);
		if (stockItemQry.next())
		{
			name = stockItemQry.value("item_name").toString();
			ES::Utility::printRow(tableElement, row, 0, name);
			ES::Utility::printRow(tableElement, row, 1, qty);
			ES::Utility::printRow(tableElement, row, 2, price);
			ES::Utility::printRow(tableElement, row, 3, QString::number(discount,'f',2));
			ES::Utility::printRow(tableElement, row, 4, totalPrice);
			row++;
		}
	}

	ES::Utility::printRow(tableElement, row, 3, "# Items", Qt::AlignRight);
	ES::Utility::printRow(tableElement, row, 4, QString::number(itemCount), Qt::AlignRight);

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

void ESItemWiseSalesSummary::slotPrint(QPrinter* printer)
{
	report.print(printer);
	this->close();
}

