#include "esrevenuemasterreport.h"
#include "QSqlQuery"
#include "KDReportsTextElement.h"
#include "utility\session.h"
#include "KDReportsHtmlElement.h"
#include "KDReportsTableElement.h"
#include "KDReportsCell.h"
#include "QPrintPreviewDialog"
#include "QPrinter"
#include "utility\esmainwindowholder.h"
#include "QMainWindow"
#include "QObject"
#include "esmainwindow.h"
#include <memory>
#include "QString"

struct SaleDataHolder
{
	QString priceStr;
	QString discountStr;
	double cost;
	double lineTotal;
	double lineTotalQty;
	QString stockId;
};
ESRevenueMasterReport::ESRevenueMasterReport(QWidget *parent /*= 0*/) : QWidget(parent), report(NULL)
{
	ui.setupUi(this);
	m_generateReportSignalMapper = new QSignalMapper(this);
	ui.fromDate->setDate(QDate::currentDate());
	ui.toDate->setDate(QDate::currentDate().addDays(1));

	QStringList headerLabels;
	headerLabels.append("Item");
	headerLabels.append("Sale Price");
	headerLabels.append("Sale Qty");
	headerLabels.append("Discount");
	headerLabels.append("Total Sales");
	headerLabels.append("Item Cost");
	headerLabels.append("Total Cost");
	headerLabels.append("Return Qty");
	headerLabels.append("Return Total");
	headerLabels.append("Return Cost");

	QFont font = this->font();
	font.setBold(true);
	font.setPointSize(12);

	ui.tableWidget->setHorizontalHeaderLabels(headerLabels);
	ui.tableWidget->horizontalHeader()->setStretchLastSection(true);
	ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	//ui.tableWidget->verticalHeader()->setMinimumWidth(200);

	QObject::connect(ui.fromDate, SIGNAL(dateChanged(const QDate &)), this, SLOT(slotSearch()));
	QObject::connect(ui.toDate, SIGNAL(dateChanged(const QDate &)), this, SLOT(slotSearch()));
	QObject::connect(ui.generateButton, SIGNAL(clicked()), this, SLOT(slotGenerateReport()));
}

ESRevenueMasterReport::~ESRevenueMasterReport()
{
	delete report;
}

void ESRevenueMasterReport::slotPrint(QPrinter* printer)
{
	report->print(printer);
	this->close();
}

 void ESRevenueMasterReport::slotSearch()
 {
 	while (ui.tableWidget->rowCount() > 0)
 	{
 		ui.tableWidget->removeRow(0);
 	}
 	QVector<SaleDataHolder> salesData;
 	QString stardDateStr = ui.fromDate->date().toString("yyyy-MM-dd");
 	QString endDateStr = ui.toDate->date().toString("yyyy-MM-dd");
 	double grandSalesTotal = 0, salesGrandCost=0, returnGrandTotal =0 , returnGrandCost = 0;
 	QString qSaleStr = "SELECT stock_id, discount, item_price, w_cost, SUM(total) as total, SUM(quantity) as qty FROM sale WHERE deleted= 0 AND DATE(date) BETWEEN '" + stardDateStr + "' AND '" + endDateStr + "' GROUP BY stock_id";
 	QSqlQuery querySale;
 	querySale.prepare(qSaleStr);
 	querySale.setForwardOnly(true);
 	if (querySale.exec())
 	{
 		while (querySale.next())
 		{
 			SaleDataHolder dh;
 			dh.priceStr = querySale.value("item_price").toString();
 			dh.discountStr = querySale.value("discount").toString();
 			dh.cost = querySale.value("w_cost").toDouble();
 			dh.lineTotal = querySale.value("total").toDouble();
 			dh.lineTotalQty = querySale.value("qty").toDouble();
 			dh.stockId = querySale.value("stock_id").toString();
 			salesData.push_back(dh);
 			
 		}
 	}
 	int row = 0;
 	for (SaleDataHolder sdh : salesData)
 	{
 		QString queryStockItemStr = "SELECT item.item_name, item.w_cost, item.item_id FROM stock, item WHERE stock.item_id = item.item_id AND stock.deleted=0 AND item.deleted = 0 AND stock.stock_id = " + sdh.stockId;
 		QSqlQuery queryStockItem;
 		queryStockItem.prepare(queryStockItemStr);
 		queryStockItem.setForwardOnly(true);
 		queryStockItem.exec();
 		if (queryStockItem.first())
 		{
 			if (sdh.cost == -1)
 			{
 				sdh.cost = queryStockItem.value("w_cost").toDouble();
 			}
 			double totalCost = sdh.cost*sdh.lineTotalQty;
 			grandSalesTotal += sdh.lineTotal;
 			salesGrandCost += totalCost;
 			row = ui.tableWidget->rowCount();
 			ui.tableWidget->insertRow(row);
 
 			QString itemName = queryStockItem.value("item_name").toString();
 			QString itemId = queryStockItem.value("item_id").toString();
 
 			QString returnQryStr = "SELECT SUM(qty) as totalQty, SUM(return_total) as retTotal FROM return_item WHERE item_id = " + itemId + " AND deleted = 0 AND DATE(date) BETWEEN '" + stardDateStr + "' AND '" + endDateStr + "'";
 			QSqlQuery queryReturn;
 			queryReturn.prepare(returnQryStr);
 			queryReturn.setForwardOnly(true);
 			queryReturn.exec();
 
 			float returnLineQty = 0, returnLineTotal = 0;
 			if (queryReturn.first())
 			{
 				returnLineQty = queryReturn.value("totalQty").toFloat();
 				//returnLineTotal = queryReturn.value("retTotal").toFloat();
 			}
			returnLineTotal = (sdh.priceStr).toFloat() * returnLineQty;
 			float returnCost = returnLineQty * sdh.cost;
 			returnGrandCost += returnCost;
 			returnGrandTotal += returnLineTotal;
 
 			QTableWidgetItem *itemNameWidget = new QTableWidgetItem(itemName);
 			itemNameWidget->setTextAlignment(Qt::AlignLeft);
 			ui.tableWidget->setItem(row, 0, itemNameWidget);
 
 			QTableWidgetItem *itemPriceWidget = new QTableWidgetItem(sdh.priceStr);
 			itemPriceWidget->setTextAlignment(Qt::AlignRight);
 			ui.tableWidget->setItem(row, 1, itemPriceWidget);
 
 			QTableWidgetItem *itemQtyWidget = new QTableWidgetItem(QString::number(sdh.lineTotalQty));
 			itemQtyWidget->setTextAlignment(Qt::AlignRight);
 			ui.tableWidget->setItem(row, 2, itemQtyWidget);
 
 			QTableWidgetItem *discountWidget = new QTableWidgetItem(sdh.discountStr);
 			discountWidget->setTextAlignment(Qt::AlignRight);
 			ui.tableWidget->setItem(row, 3, discountWidget);
 
 			QTableWidgetItem *totalSalesWidget = new QTableWidgetItem(QString::number(sdh.lineTotal, 'f', 2));
 			totalSalesWidget->setTextAlignment(Qt::AlignRight);
 			ui.tableWidget->setItem(row, 4, totalSalesWidget);
 
 			QTableWidgetItem *itemCostWidget = new QTableWidgetItem(QString::number(sdh.cost, 'f', 2));
 			itemCostWidget->setTextAlignment(Qt::AlignRight);
 			ui.tableWidget->setItem(row, 5, itemCostWidget);
 
 			QTableWidgetItem *totalItemCostWidget = new QTableWidgetItem(QString::number(totalCost, 'f', 2));
 			totalItemCostWidget->setTextAlignment(Qt::AlignRight);
 			ui.tableWidget->setItem(row, 6, totalItemCostWidget);
 
 			QTableWidgetItem *returnQtyWidget = new QTableWidgetItem(QString::number(returnLineQty));
 			returnQtyWidget->setTextAlignment(Qt::AlignRight);
 			ui.tableWidget->setItem(row, 7, returnQtyWidget);
 
 			QTableWidgetItem *returnTotalWidget = new QTableWidgetItem(QString::number(returnLineTotal, 'f', 2));
 			returnTotalWidget->setTextAlignment(Qt::AlignRight);
 			ui.tableWidget->setItem(row, 8, returnTotalWidget);
 
 			QTableWidgetItem *returnCostWidget = new QTableWidgetItem(QString::number(returnCost, 'f', 2));
 			returnCostWidget->setTextAlignment(Qt::AlignRight);
 			ui.tableWidget->setItem(row, 9, returnCostWidget);
 			float tempProfit = (sdh.lineTotal - returnLineTotal) - (totalCost - returnCost);
 		}
 	}
 	double profit = (grandSalesTotal - returnGrandTotal) - (salesGrandCost - returnGrandCost);
 	ui.profitLbl->setText(QString::number(profit,'f', 2));
 	ui.salesLbl->setText(QString::number((grandSalesTotal - returnGrandTotal), 'f', 2));
 }

void ESRevenueMasterReport::printRow(KDReports::TableElement* tableElement, int row, int col, QString elementStr, Qt::AlignmentFlag alignment /*= Qt::AlignLeft*/)
{
	KDReports::Cell& cell = tableElement->cell(row, col);
	KDReports::TextElement te(elementStr);
	te.setPointSize(ES::Session::getInstance()->getBillItemFontSize());
	cell.addElement(te, alignment);
}

Ui::RevenueMasterReport& ESRevenueMasterReport::getUI()
{
	return ui;
}


// void ESRevenueMasterReport::slotGenerateReport()
// {
// 	report = new KDReports::Report;
// 
// 	KDReports::TextElement titleElement("Revenue Master Report");
// 	titleElement.setPointSize(13);
// 	titleElement.setBold(true);
// 	report->addElement(titleElement, Qt::AlignHCenter);
// 
// 
// 	QString stardDateStr = ui.fromDate->date().toString("yyyy-MM-dd");
// 	QString endDateStr = ui.toDate->date().toString("yyyy-MM-dd");
// 
// 	QString dateStr = "Date : ";
// 	dateStr.append(stardDateStr).append(" - ").append(endDateStr);
// 
// 
// 	KDReports::TableElement infoTableElement;
// 	infoTableElement.setHeaderRowCount(2);
// 	infoTableElement.setHeaderColumnCount(2);
// 	infoTableElement.setBorder(0);
// 	infoTableElement.setWidth(100, KDReports::Percent);
// 
// 	{
// 		KDReports::Cell& dateCell = infoTableElement.cell(0, 1);
// 		KDReports::TextElement t(dateStr);
// 		t.setPointSize(10);
// 		dateCell.addElement(t, Qt::AlignRight);
// 	}
// 
// 	report->addElement(infoTableElement);
// 	report->addVerticalSpacing(5);
// 
// 	KDReports::TableElement* tableElement = new KDReports::TableElement;
// 	tableElement->setHeaderColumnCount(10);
// 	tableElement->setBorder(1);
// 	tableElement->setWidth(100, KDReports::Percent);
// 
// 	{
// 		KDReports::Cell& cell = tableElement->cell(0, 0);
// 		KDReports::TextElement cTextElement("Item");
// 		cTextElement.setPointSize(11);
// 		cTextElement.setBold(true);
// 		cell.addElement(cTextElement, Qt::AlignCenter);
// 	}
// 	{
// 		KDReports::Cell& cell = tableElement->cell(0, 1);
// 		KDReports::TextElement cTextElement("Selling Price");
// 		cTextElement.setPointSize(11);
// 		cTextElement.setBold(true);
// 		cell.addElement(cTextElement, Qt::AlignCenter);
// 	}
// 	{
// 		KDReports::Cell& cell = tableElement->cell(0, 2);
// 		KDReports::TextElement cTextElement("Sold Qty");
// 		cTextElement.setPointSize(11);
// 		cTextElement.setBold(true);
// 		cell.addElement(cTextElement, Qt::AlignCenter);
// 	}
// 	{
// 		KDReports::Cell& cell = tableElement->cell(0, 3);
// 		KDReports::TextElement cTextElement("Discount");
// 		cTextElement.setPointSize(11);
// 		cTextElement.setBold(true);
// 		cell.addElement(cTextElement, Qt::AlignCenter);
// 	}
// 
// 	{
// 		KDReports::Cell& cell = tableElement->cell(0, 4);
// 		KDReports::TextElement cTextElement("Total Sales");
// 		cTextElement.setPointSize(11);
// 		cTextElement.setBold(true);
// 		cell.addElement(cTextElement, Qt::AlignCenter);
// 	}
// 	{
// 		KDReports::Cell& cell = tableElement->cell(0, 5);
// 		KDReports::TextElement cTextElement("Item Cost");
// 		cTextElement.setPointSize(11);
// 		cTextElement.setBold(true);
// 		cell.addElement(cTextElement, Qt::AlignCenter);
// 	}
// 	{
// 		KDReports::Cell& cell = tableElement->cell(0, 6);
// 		KDReports::TextElement cTextElement("Total Cost");
// 		cTextElement.setPointSize(11);
// 		cTextElement.setBold(true);
// 		cell.addElement(cTextElement, Qt::AlignCenter);
// 	}
// 	{
// 		KDReports::Cell& cell = tableElement->cell(0, 7);
// 		KDReports::TextElement cTextElement("Return Qty");
// 		cTextElement.setPointSize(11);
// 		cTextElement.setBold(true);
// 		cell.addElement(cTextElement, Qt::AlignCenter);
// 	}
// 	{
// 		KDReports::Cell& cell = tableElement->cell(0, 8);
// 		KDReports::TextElement cTextElement("Return Total");
// 		cTextElement.setPointSize(11);
// 		cTextElement.setBold(true);
// 		cell.addElement(cTextElement, Qt::AlignCenter);
// 	}
// 
// 	{
// 		KDReports::Cell& cell = tableElement->cell(0, 9);
// 		KDReports::TextElement cTextElement("Return Cost");
// 		cTextElement.setPointSize(11);
// 		cTextElement.setBold(true);
// 		cell.addElement(cTextElement, Qt::AlignCenter);
// 	}
// 	QVector<std::shared_ptr<SaleDataHolder>> salesData;
// 	double grandSalesTotal = 0, salesGrandCost = 0, returnGrandTotal = 0, returnGrandCost = 0;
// 	QString qSaleStr = "SELECT stock_id, discount, item_price, w_cost, SUM(total) as total, SUM(quantity) as qty FROM sale WHERE deleted= 0 AND DATE(date) BETWEEN '" + stardDateStr + "' AND '" + endDateStr + "' GROUP BY stock_id";
// 	QSqlQuery querySale;
// 	querySale.prepare(qSaleStr);
// 	querySale.setForwardOnly(true);
// 	int row = 1;
// 	if (querySale.exec())
// 	{
// 		while (querySale.next())
// 		{
// 			std::shared_ptr<SaleDataHolder> dh = std::make_shared<SaleDataHolder>();
// 			dh->priceStr = querySale.value("item_price").toString();
// 			dh->discountStr = querySale.value("discount").toString();
// 			dh->cost = querySale.value("w_cost").toDouble();
// 			dh->lineTotal = querySale.value("total").toDouble();
// 			dh->lineTotalQty = querySale.value("qty").toDouble();
// 			dh->stockId = querySale.value("stock_id").toString();
// 			salesData.push_back(dh);
// 
// 		}
// 	}
// 
// 	for (std::shared_ptr<SaleDataHolder> sdh : salesData)
// 	{
// 		double totalCost = sdh->cost*sdh->lineTotalQty;
// 		grandSalesTotal += sdh->lineTotal;
// 		salesGrandCost += totalCost;
// 		QString queryStockItemStr = "SELECT item.item_name, item.item_id FROM stock, item WHERE stock.item_id = item.item_id AND stock.stock_id = " + sdh->stockId;
// 		QSqlQuery queryStockItem;
// 		queryStockItem.prepare(queryStockItemStr);
// 		queryStockItem.setForwardOnly(true);
// 		queryStockItem.exec();
// 		if (queryStockItem.first())
// 		{
// 			QString itemName = queryStockItem.value("item_name").toString();
// 			QString itemId = queryStockItem.value("item_id").toString();
// 
// 			QString returnQryStr = "SELECT SUM(qty) as totalQty, SUM(return_total) as retTotal FROM return_item WHERE item_id = " + itemId + " AND deleted = 0 AND DATE(date) BETWEEN '" + stardDateStr + "' AND '" + endDateStr + "'";
// 			QSqlQuery queryReturn;
// 			queryReturn.prepare(returnQryStr);
// 			queryReturn.setForwardOnly(true);
// 			queryReturn.exec();
// 
// 			float returnLineQty = 0, returnLineTotal = 0;
// 			if (queryReturn.first())
// 			{
// 				returnLineQty = queryReturn.value("totalQty").toFloat();
// 				returnLineTotal = queryReturn.value("retTotal").toFloat();
// 			}
// 			float returnCost = returnLineQty * sdh->cost;
// 			returnGrandCost += returnCost;
// 			returnGrandTotal += returnLineTotal;
// 			printRow(tableElement, row, 0, itemName);
// 			printRow(tableElement, row, 1, sdh->priceStr);
// 			printRow(tableElement, row, 2, QString::number(sdh->lineTotalQty, 'f', 2));
// 			printRow(tableElement, row, 3, sdh->discountStr);
// 			printRow(tableElement, row, 4, QString::number(sdh->lineTotal, 'f', 2));
// 			printRow(tableElement, row, 5, QString::number(sdh->cost, 'f', 2));
// 			printRow(tableElement, row, 6, QString::number(totalCost, 'f', 2));
// 			printRow(tableElement, row, 7, QString::number(returnLineQty));
// 			printRow(tableElement, row, 8, QString::number(returnLineTotal, 'f', 2));
// 			printRow(tableElement, row, 9, QString::number(returnCost, 'f', 2));
// 			row++;
// 		}
// 	}
// 	double profit = (grandSalesTotal - returnGrandTotal) - (salesGrandCost - returnGrandCost);
// 	ui.profitLbl->setText(QString::number(profit, 'f', 2));
// 	printRow(tableElement, row, 8, "Profit : "); 
// 	printRow(tableElement, row, 9, QString::number(profit, 'f', 2));
// 
// 
// 	report->addElement(*tableElement);
// 
// 	QPrinter* printer = new QPrinter();
// 	printer->setPaperSize(QPrinter::A4);
// 
// 	printer->setFullPage(false);
// 	printer->setOrientation(QPrinter::Landscape);
// 
// 	QPrintPreviewDialog *dialog = new QPrintPreviewDialog(printer, this);
// 	QObject::connect(dialog, SIGNAL(paintRequested(QPrinter*)), this, SLOT(slotPrint(QPrinter*)));
// 	dialog->setWindowTitle(tr("Print Document"));
// 	ES::MainWindowHolder::instance()->getMainWindow()->setCentralWidget(dialog);
// 	dialog->exec();
// 	delete dialog;
// 	delete printer;
// 	delete report;
// 
// }

void ESRevenueMasterReport::slotGenerateReport()
{
	report = new KDReports::Report;

	KDReports::TextElement titleElement("Revenue Master Report");
	titleElement.setPointSize(13);
	titleElement.setBold(true);
	report->addElement(titleElement, Qt::AlignHCenter);


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

	report->addElement(infoTableElement);
	report->addVerticalSpacing(5);

	KDReports::TableElement* tableElement = new KDReports::TableElement;
	tableElement->setHeaderColumnCount(5);
	tableElement->setBorder(1);
	tableElement->setWidth(100, KDReports::Percent);

	{
		KDReports::Cell& cell = tableElement->cell(0, 0);
		KDReports::TextElement cTextElement("Total Sales");
		cTextElement.setPointSize(11);
		cTextElement.setBold(true);
		cell.addElement(cTextElement, Qt::AlignCenter);
	}
	{
		KDReports::Cell& cell = tableElement->cell(0, 1);
		KDReports::TextElement cTextElement("Total Sales Cost");
		cTextElement.setPointSize(11);
		cTextElement.setBold(true);
		cell.addElement(cTextElement, Qt::AlignCenter);
	}
	{
		KDReports::Cell& cell = tableElement->cell(0, 2);
		KDReports::TextElement cTextElement("Total Return");
		cTextElement.setPointSize(11);
		cTextElement.setBold(true);
		cell.addElement(cTextElement, Qt::AlignCenter);
	}
	{
		KDReports::Cell& cell = tableElement->cell(0, 3);
		KDReports::TextElement cTextElement("Total Return Cost");
		cTextElement.setPointSize(11);
		cTextElement.setBold(true);
		cell.addElement(cTextElement, Qt::AlignCenter);
	}

	{
		KDReports::Cell& cell = tableElement->cell(0, 4);
		KDReports::TextElement cTextElement("Profit");
		cTextElement.setPointSize(11);
		cTextElement.setBold(true);
		cell.addElement(cTextElement, Qt::AlignCenter);
	}
	
	QVector<std::shared_ptr<SaleDataHolder>> salesData;
	double grandSalesTotal = 0, salesGrandCost = 0, returnGrandTotal = 0, returnGrandCost = 0;
	QString qSaleStr = "SELECT stock_id, discount, item_price, w_cost, SUM(total) as total, SUM(quantity) as qty FROM sale WHERE deleted= 0 AND DATE(date) BETWEEN '" + stardDateStr + "' AND '" + endDateStr + "' GROUP BY stock_id";
	QSqlQuery querySale;
	querySale.prepare(qSaleStr);
	querySale.setForwardOnly(true);
	int row = 1;
	if (querySale.exec())
	{
		while (querySale.next())
		{
			std::shared_ptr<SaleDataHolder> dh = std::make_shared<SaleDataHolder>();
			dh->priceStr = querySale.value("item_price").toString();
			dh->discountStr = querySale.value("discount").toString();
			dh->cost = querySale.value("w_cost").toDouble();
			dh->lineTotal = querySale.value("total").toDouble();
			dh->lineTotalQty = querySale.value("qty").toDouble();
			dh->stockId = querySale.value("stock_id").toString();
			salesData.push_back(dh);

		}
	}

	for (std::shared_ptr<SaleDataHolder> sdh : salesData)
	{
// 		double totalCost = sdh->cost*sdh->lineTotalQty;
// 		grandSalesTotal += sdh->lineTotal;
// 		salesGrandCost += totalCost;
		QString queryStockItemStr = "SELECT item.item_name, item.w_cost, item.item_id FROM stock, item WHERE stock.item_id = item.item_id AND stock.deleted=0 AND item.deleted = 0 AND stock.stock_id = " + sdh->stockId;
		QSqlQuery queryStockItem;
		queryStockItem.prepare(queryStockItemStr);
		queryStockItem.setForwardOnly(true);
		queryStockItem.exec();
		if (queryStockItem.first())
		{
			if (sdh->cost == -1)
			{
				sdh->cost = queryStockItem.value("w_cost").toDouble();
			}
			double totalCost = sdh->cost*sdh->lineTotalQty;
			grandSalesTotal += sdh->lineTotal;
			salesGrandCost += totalCost;
			QString itemName = queryStockItem.value("item_name").toString();
			QString itemId = queryStockItem.value("item_id").toString();

			QString returnQryStr = "SELECT SUM(qty) as totalQty, SUM(return_total) as retTotal FROM return_item WHERE item_id = " + itemId + " AND deleted = 0 AND DATE(date) BETWEEN '" + stardDateStr + "' AND '" + endDateStr + "'";
			QSqlQuery queryReturn;
			queryReturn.prepare(returnQryStr);
			queryReturn.setForwardOnly(true);
			queryReturn.exec();

			float returnLineQty = 0, returnLineTotal = 0;
			if (queryReturn.first())
			{
				returnLineQty = queryReturn.value("totalQty").toFloat();
				//returnLineTotal = queryReturn.value("retTotal").toFloat();
			}
			returnLineTotal = (sdh->priceStr).toFloat() * returnLineQty;
			float returnCost = returnLineQty * sdh->cost;
			returnGrandCost += returnCost;
			returnGrandTotal += returnLineTotal;
		}
	}
	double profit = (grandSalesTotal - returnGrandTotal) - (salesGrandCost - returnGrandCost);
	printRow(tableElement, row, 0, QString::number(grandSalesTotal, 'f', 2));
	printRow(tableElement, row, 1, QString::number(salesGrandCost, 'f', 2));
	printRow(tableElement, row, 2, QString::number(returnGrandTotal, 'f', 2));
	printRow(tableElement, row, 3, QString::number(returnGrandCost, 'f', 2));
	printRow(tableElement, row, 4, QString::number(profit, 'f', 2));


	report->addElement(*tableElement);

	QPrinter* printer = new QPrinter();
	printer->setPaperSize(QPrinter::A4);

	printer->setFullPage(false);
	printer->setOrientation(QPrinter::Landscape);

	QPrintPreviewDialog *dialog = new QPrintPreviewDialog(printer, this);
	QObject::connect(dialog, SIGNAL(paintRequested(QPrinter*)), this, SLOT(slotPrint(QPrinter*)));
	dialog->setWindowTitle(tr("Print Document"));
	ES::MainWindowHolder::instance()->getMainWindow()->setCentralWidget(dialog);
	dialog->exec();
// 	delete dialog;
// 	delete printer;
// 	delete report;

}