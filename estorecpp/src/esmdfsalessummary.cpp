#include "esmdfsalessummary.h"
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
#include "QString"

struct MDFItemDataHolder
{
	QString stockId;
	QString itemName;
	QString itemId;
};
MDFSalesSummary::MDFSalesSummary(QWidget *parent /*= 0*/) : QWidget(parent), report(NULL)
{
	ui.setupUi(this);
	m_generateReportSignalMapper = new QSignalMapper(this);
	ui.fromDate->setDate(QDate::currentDate());
	ui.toDate->setDate(QDate::currentDate().addDays(1));

	QStringList headerLabels;
	headerLabels.append("Item Cost");
	//headerLabels.append("Discount");
	headerLabels.append("SellingPrice Price");
	headerLabels.append("Sold Qty");
	headerLabels.append("Total Sales");
	headerLabels.append("Sales Cost");
	headerLabels.append("Return Qty");
	headerLabels.append("Total Return");
	headerLabels.append("Return Cost");
	headerLabels.append("Profit");

	QFont font = this->font();
	font.setBold(true);
	font.setPointSize(12);

	ui.tableWidget->setHorizontalHeaderLabels(headerLabels);
	ui.tableWidget->horizontalHeader()->setStretchLastSection(true);
	ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.tableWidget->verticalHeader()->setMinimumWidth(200);

	QObject::connect(ui.generateButton, SIGNAL(clicked()), this, SLOT(slotGenerateReport()));
	QObject::connect(ui.fromDate, SIGNAL(dateChanged(const QDate &)), this, SLOT(slotSearch()));
	QObject::connect(ui.toDate, SIGNAL(dateChanged(const QDate &)), this, SLOT(slotSearch()));
	//QObject::connect(m_generateReportSignalMapper, SIGNAL(mapped(QString)), this, SLOT(slotGenerateReportForGivenItem(QString)));
	//slotSearch();
}

MDFSalesSummary::~MDFSalesSummary()
{

}

void MDFSalesSummary::slotPrint(QPrinter* printer)
{
	report->print(printer);
	this->close();
}

void MDFSalesSummary::slotSearch()
{
	while (ui.tableWidget->rowCount() > 0)
	{
		ui.tableWidget->removeRow(0);
	}

	float grandReturnTotal, grandReturnCost = 0, grandSaleCost = 0, grandSalesTotal = 0;
	float totalProfit = 0;
	QString stardDateStr = ui.fromDate->date().toString("yyyy-MM-dd");
	QString endDateStr = ui.toDate->date().toString("yyyy-MM-dd");
	double netTotalIncome = 0;
	QVector<MDFItemDataHolder> dataHolderVec;
	QString qMDFStr = "SELECT * FROM Item JOIN item_category ON item.itemcategory_id = item_category.itemcategory_id WHERE item.deleted = 0 AND item_category.itemcategory_name ='MDF'";
	QSqlQuery categoryQry;
	categoryQry.setForwardOnly(true);
	categoryQry.exec(qMDFStr);
	while (categoryQry.next())
	{
		QString itemId = categoryQry.value("item_id").toString();
		QString itemName = categoryQry.value("item_name").toString();
		QString stockQryStr = "SELECT stock.stock_id FROM stock_purchase_order_item JOIN stock ON stock_purchase_order_item.stock_id = stock.stock_id WHERE stock.deleted = 0 AND stock.item_id = " + itemId;
		QSqlQuery qryStock;
		qryStock.setForwardOnly(true);
		qryStock.exec(stockQryStr);
		if (qryStock.next())
		{
			QString stockId = qryStock.value("stock_id").toString();
			MDFItemDataHolder midh;
			midh.itemName = itemName;
			midh.stockId = stockId;
			midh.itemId = itemId;
			dataHolderVec.push_back(midh);
		}
	}

	for (auto midh : dataHolderVec)
	{
		int row = 0;

		float itemCost = 0;
		float lineTotal = 0;
		float lineQty = 0;
		float lineCost = 0;
		QString salesQryStr = "SELECT discount, SUM(total) as totalAmount, SUM(quantity) as totalQty, w_cost FROM sale WHERE stock_id = " + midh.stockId + " AND deleted = 0 AND  DATE(date) BETWEEN '" + stardDateStr + "' AND '" + endDateStr + "'";
		QSqlQuery salesQry;
		salesQry.setForwardOnly(true);
		salesQry.exec(salesQryStr);

		if (salesQry.next())
		{

			lineTotal = salesQry.value("totalAmount").toFloat();
			if (lineTotal > 0)
			{
				row = ui.tableWidget->rowCount();
				ui.tableWidget->insertRow(row);

				QTableWidgetItem* nameItem = new QTableWidgetItem(midh.itemName);
				ui.tableWidget->setVerticalHeaderItem(row, nameItem);
				grandSalesTotal += lineTotal;
				float discount = salesQry.value("discount").toFloat();
				lineQty = salesQry.value("totalQty").toFloat();
				lineCost = salesQry.value("w_cost").toFloat()*lineQty;

				grandReturnCost += lineCost;
				itemCost = lineCost / lineQty;
				float avgSellingPrice = lineTotal / lineQty;

				QTableWidgetItem *purchasePriceWidget = new QTableWidgetItem(QString::number(itemCost, 'f', 2));
				purchasePriceWidget->setTextAlignment(Qt::AlignRight);
				ui.tableWidget->setItem(row, 0, purchasePriceWidget);

				// 				QTableWidgetItem *discountWidget = new QTableWidgetItem(QString::number(discount, 'f', 2));
				// 				discountWidget->setTextAlignment(Qt::AlignRight);
				// 				ui.tableWidget->setItem(row, 1, discountWidget);

				QTableWidgetItem *avgPriceWidget = new QTableWidgetItem(QString::number(avgSellingPrice, 'f', 2));
				avgPriceWidget->setTextAlignment(Qt::AlignRight);
				ui.tableWidget->setItem(row, 1, avgPriceWidget);

				QTableWidgetItem *totalQtyWidget = new QTableWidgetItem(QString::number(lineQty));
				totalQtyWidget->setTextAlignment(Qt::AlignRight);
				ui.tableWidget->setItem(row, 2, totalQtyWidget);

				QTableWidgetItem *totalSalesWidget = new QTableWidgetItem(QString::number(lineTotal, 'f', 2));
				totalSalesWidget->setTextAlignment(Qt::AlignRight);
				ui.tableWidget->setItem(row, 3, totalSalesWidget);

				QTableWidgetItem *totalSalesCostWidget = new QTableWidgetItem(QString::number(lineCost, 'f', 2));
				totalSalesCostWidget->setTextAlignment(Qt::AlignRight);
				ui.tableWidget->setItem(row, 4, totalSalesCostWidget);
			}

		}

		float returnTotal = 0;
		float totalReturnCost = 0;
		QSqlQuery queryReturn("SELECT SUM(qty) as totalQty , SUM(return_total) as retTotal FROM return_item WHERE item_id = " + midh.itemId + " AND deleted = 0 AND DATE(date) BETWEEN '" + stardDateStr + "' AND '" + endDateStr + "'");
		if (queryReturn.next())
		{
			float lineReturnQty = queryReturn.value("totalQty").toFloat();
			float lineReturn = queryReturn.value("retTotal").toFloat();
			grandReturnTotal += lineReturn;

			QTableWidgetItem *returnQtyWidget = new QTableWidgetItem(QString::number(lineReturnQty));
			returnQtyWidget->setTextAlignment(Qt::AlignRight);
			ui.tableWidget->setItem(row, 5, returnQtyWidget);

			QTableWidgetItem *totalReturnWidget = new QTableWidgetItem(QString::number(lineReturn, 'f', 2));
			totalReturnWidget->setTextAlignment(Qt::AlignRight);
			ui.tableWidget->setItem(row, 6, totalReturnWidget);

			totalReturnCost = lineReturnQty * itemCost;
			grandReturnCost += totalReturnCost;

			QTableWidgetItem *totalReturnCostWidget = new QTableWidgetItem(QString::number(totalReturnCost, 'f', 2));
			totalReturnCostWidget->setTextAlignment(Qt::AlignRight);
			ui.tableWidget->setItem(row, 7, totalReturnCostWidget);

		}
		float itemProfit = (lineTotal - lineCost) - (returnTotal - totalReturnCost);
		totalProfit += itemProfit;
		netTotalIncome += (lineTotal - returnTotal);
		QTableWidgetItem *profitWidget = new QTableWidgetItem(QString::number(itemProfit, 'f', 2));
		profitWidget->setTextAlignment(Qt::AlignRight);
		ui.tableWidget->setItem(row, 8, profitWidget);

		QWidget* base = new QWidget(ui.tableWidget);

		// 			QPushButton* generateReportBtn = new QPushButton(base);
		// 			generateReportBtn->setIcon(QIcon("icons/pdf.png"));
		// 			generateReportBtn->setIconSize(QSize(24, 24));
		// 			generateReportBtn->setMaximumWidth(100);

		// 			m_generateReportSignalMapper->setMapping(generateReportBtn, itemId);
		// 			QObject::connect(generateReportBtn, SIGNAL(clicked()), m_generateReportSignalMapper, SLOT(map()));
		// 
		// 			QHBoxLayout *layout = new QHBoxLayout;
		// 			layout->setContentsMargins(0, 0, 0, 0);
		// 			layout->addWidget(generateReportBtn);
		// 			layout->insertStretch(2);
		// 			base->setLayout(layout);
		// 			ui.tableWidget->setCellWidget(row, 9, base);
		// 			base->show();
	}
	ui.totalProfitLbl->setText(QString::number(totalProfit, 'f', 2));
	ui.totalIncomeLbl->setText(QString::number(netTotalIncome, 'f', 2));
}

void MDFSalesSummary::printRow(KDReports::TableElement& tableElement, int row, int col, QString elementStr, Qt::AlignmentFlag alignment /*= Qt::AlignLeft*/)
{
	KDReports::Cell& cell = tableElement.cell(row, col);
	KDReports::TextElement te(elementStr);
	te.setPointSize(ES::Session::getInstance()->getBillItemFontSize());
	cell.addElement(te, alignment);
}

Ui::MDFSalesSummary& MDFSalesSummary::getUI()
{
	return ui;
}

// void MDFSalesSummary::slotGenerateReportForGivenItem(QString itemId)
// {
// 	report = new KDReports::Report;
// 	float averagePrice = 0, totalQty = 0, totalAmount = 0, totalProfit = 0;
// 	QSqlQuery queryItems("SELECT * FROM Item WHERE item_id = " + itemId);
// 	if (queryItems.next())
// 	{
// 		QString itemName = queryItems.value("item_name").toString();
// 
// 		KDReports::TextElement titleElement("Profit/Loss Summary of " + itemName);
// 		titleElement.setPointSize(13);
// 		titleElement.setBold(true);
// 		report->addElement(titleElement, Qt::AlignHCenter);
// 
// 		QString stardDateStr = ui.fromDate->date().toString("yyyy-MM-dd");
// 		QString endDateStr = ui.toDate->date().toString("yyyy-MM-dd");
// 
// 		QString dateStr = "Date : ";
// 		dateStr.append(stardDateStr).append(" - ").append(endDateStr);
// 
// 
// 		KDReports::TableElement infoTableElement;
// 		infoTableElement.setHeaderRowCount(2);
// 		infoTableElement.setHeaderColumnCount(2);
// 		infoTableElement.setBorder(0);
// 		infoTableElement.setWidth(100, KDReports::Percent);
// 
// 		{
// 			KDReports::Cell& dateCell = infoTableElement.cell(0, 1);
// 			KDReports::TextElement t(dateStr);
// 			t.setPointSize(10);
// 			dateCell.addElement(t, Qt::AlignRight);
// 		}
// 		
// 		report->addElement(infoTableElement);
// 		report->addVerticalSpacing(5);
// 
// 		KDReports::TableElement tableElement;
// 		tableElement.setHeaderColumnCount(9);
// 		tableElement.setBorder(1);
// 		tableElement.setWidth(100, KDReports::Percent);
// 
// 		{
// 			KDReports::Cell& cell = tableElement.cell(0, 0);
// 			KDReports::TextElement cTextElement("Item");
// 			cTextElement.setPointSize(11);
// 			cTextElement.setBold(true);
// 			cell.addElement(cTextElement, Qt::AlignCenter);
// 		}
// 		{
// 			KDReports::Cell& cell = tableElement.cell(0, 1);
// 			KDReports::TextElement cTextElement("Item Cost");
// 			cTextElement.setPointSize(11);
// 			cTextElement.setBold(true);
// 			cell.addElement(cTextElement, Qt::AlignCenter);
// 		}
// 		{
// 			KDReports::Cell& cell = tableElement.cell(0, 2);
// 			KDReports::TextElement cTextElement("Price");
// 			cTextElement.setPointSize(11);
// 			cTextElement.setBold(true);
// 			cell.addElement(cTextElement, Qt::AlignCenter);
// 		}
// 		{
// 			KDReports::Cell& cell = tableElement.cell(0, 3);
// 			KDReports::TextElement cTextElement("Sold Qty");
// 			cTextElement.setPointSize(11);
// 			cTextElement.setBold(true);
// 			cell.addElement(cTextElement, Qt::AlignCenter);
// 		}
// 		{
// 			KDReports::Cell& cell = tableElement.cell(0, 4);
// 			KDReports::TextElement cTextElement("Total Sales");
// 			cTextElement.setPointSize(11);
// 			cTextElement.setBold(true);
// 			cell.addElement(cTextElement, Qt::AlignCenter);
// 		}
// 		{
// 			KDReports::Cell& cell = tableElement.cell(0, 5);
// 			KDReports::TextElement cTextElement("Sales Cost");
// 			cTextElement.setPointSize(11);
// 			cTextElement.setBold(true);
// 			cell.addElement(cTextElement, Qt::AlignCenter);
// 		}
// 		{
// 			KDReports::Cell& cell = tableElement.cell(0, 6);
// 			KDReports::TextElement cTextElement("Return Qty");
// 			cTextElement.setPointSize(11);
// 			cTextElement.setBold(true);
// 			cell.addElement(cTextElement, Qt::AlignCenter);
// 		}
// 		{
// 			KDReports::Cell& cell = tableElement.cell(0, 7);
// 			KDReports::TextElement cTextElement("Return Total");
// 			cTextElement.setPointSize(11);
// 			cTextElement.setBold(true);
// 			cell.addElement(cTextElement, Qt::AlignCenter);
// 		}
// 		{
// 			KDReports::Cell& cell = tableElement.cell(0, 8);
// 			KDReports::TextElement cTextElement("Return Cost");
// 			cTextElement.setPointSize(11);
// 			cTextElement.setBold(true);
// 			cell.addElement(cTextElement, Qt::AlignCenter);
// 		}
// 		{
// 			KDReports::Cell& cell = tableElement.cell(0, 9);
// 			KDReports::TextElement cTextElement("Profit");
// 			cTextElement.setPointSize(11);
// 			cTextElement.setBold(true);
// 			cell.addElement(cTextElement, Qt::AlignCenter);
// 		}
// 		int row = 1;
// 		double totalProfit = 0;
// 	double netTotalIncome = 0;
// 	QStringList stockIDList;
// 	QString qMDFStr = "SELECT * FROM Item JOIN item_category ON item.itemcategory_id = item_category.itemcategory_id WHERE item.deleted = 0 AND item_category.itemcategory_name ='MDF'";
// 	QSqlQuery categoryQry;
// 	categoryQry.setForwardOnly(true);
// 	categoryQry.exec(qMDFStr);
// 	while (categoryQry.next())
// 	{
// 		float grandReturnTotal, grandReturnCost = 0, grandSaleCost = 0, grandSalesTotal = 0;
// 		float totalProfit;
// 		QString itemId = categoryQry.value("item_id").toString();
// 		QString itemName = categoryQry.value("item_name").toString();
// 		QString stockQryStr = "SELECT * FROM stock_purchase_order_item JOIN stock ON stock_purchase_order_item.stock_id = stock.stock_id WHERE stock.deleted = 0 AND stock.item_id = " + itemId;
// 		QSqlQuery qryStock;
// 		qryStock.setForwardOnly(true);
// 		qryStock.exec(stockQryStr);
// 		if (qryStock.next())
// 		{
// 			QString stockId = qryStock.value("stock_id").toString();
// 			stockIDList.push_back(stockId);			
// 		}
// 		for (QString stockId : stockIDList)
// 		{
// 			printRow(tableElement, row, 0, itemName);
// 
// 			float itemCost = 0;
// 			float lineTotal = 0;
// 			float lineQty = 0;
// 			float lineCost = 0;
// 			QString salesQryStr = "SELECT discount, SUM(total) as totalAmount, SUM(quantity) as totalQty, SUM(w_cost) as totalCost FROM sale WHERE stock_id = " + stockId + " AND deleted = 0 AND  DATE(date) BETWEEN '" + stardDateStr + "' AND '" + endDateStr + "'";
// 			QSqlQuery salesQry;
// 			salesQry.setForwardOnly(true);
// 			salesQry.exec(salesQryStr);
// 
// 			if (salesQry.next())
// 			{
// 				lineTotal = salesQry.value("totalAmount").toFloat();
// 				grandSalesTotal += lineTotal;
// 				float discount = salesQry.value("discount").toFloat();
// 				lineQty = salesQry.value("totalQty").toFloat();
// 				lineCost = salesQry.value("totalCost").toFloat();
// 
// 				grandReturnCost += lineCost;
// 				itemCost = lineCost / lineQty;
// 				float avgSellingPrice = lineTotal / lineQty;
// 
// 				printRow(tableElement, row, 1, QString::number(itemCost, 'f', 2));
// 				printRow(tableElement, row, 2, QString::number(avgSellingPrice, 'f', 2));
// 				printRow(tableElement, row, 3, QString::number(lineQty));
// 				printRow(tableElement, row, 4, QString::number(lineTotal, 'f', 2));
// 				printRow(tableElement, row, 5, QString::number(lineCost, 'f', 2));
// 				
// 			}
// 
// 			float returnTotal = 0;
// 			float totalReturnCost = 0;
// 			QSqlQuery queryReturn("SELECT SUM(qty) as totalQty , SUM(return_total) as retTotal FROM return_item WHERE item_id = " + itemId + " AND deleted = 0 AND DATE(date) BETWEEN '" + stardDateStr + "' AND '" + endDateStr + "'");
// 			if (queryReturn.next())
// 			{
// 				float lineReturnQty = queryReturn.value("totalQty").toFloat();
// 				float lineReturn = queryReturn.value("retTotal").toFloat();
// 				grandReturnTotal += lineReturn;
// 
// 				printRow(tableElement, row, 6, QString::number(lineReturnQty));
// 				printRow(tableElement, row, 7, QString::number(lineReturn, 'f', 2));
// 
// 				totalReturnCost = lineReturnQty * itemCost;
// 				grandReturnCost += totalReturnCost;
// 
// 				printRow(tableElement, row, 8, QString::number(totalReturnCost, 'f', 2));
// 
// 			}
// 			float itemProfit = (lineTotal - lineCost) - (returnTotal - totalReturnCost);
// 			totalProfit += itemProfit;
// 			netTotalIncome += (lineTotal - returnTotal);
// 			printRow(tableElement, row, 9, QString::number(itemProfit, 'f', 2));
// 			row++;
// 		}
// 	}
// // 		float sellingPrice = 0, currentDiscount = 0, purchasingPrice = 0;
// // 		float itemPrice = 0, balanceQty = 0, totalRetunedQty = 0, soldQty = 0;
// // 		QSqlQuery qryStock("SELECT * FROM stock_purchase_order_item JOIN stock ON stock_purchase_order_item.stock_id = stock.stock_id WHERE stock.deleted = 0 AND stock.item_id = " + itemId);
// // 		if (qryStock.next())
// // 		{
// // 			averagePrice = 0;
// // 			QString stockId = qryStock.value("stock_id").toString();
// // 			float sellingPrice = qryStock.value("selling_price").toFloat();
// // 			float currentDiscount = qryStock.value("discount").toFloat();
// // 			float purchasingPrice = qryStock.value("purchasing_price").toFloat();
// // 
// // 			float subTotal = 0, /*balanceQty = 0,*/ maxDiscount = 0/*, subTotal = 0, soldQty = 0*//*, totalRetunedQty = 0*/;
// // 			QString salesQryStr = "SELECT discount, total, quantity FROM sale WHERE stock_id = " + stockId + " AND deleted = 0 AND  DATE(date) BETWEEN '" + stardDateStr + "' AND '" + endDateStr + "'";
// // 			QSqlQuery salesQry(salesQryStr);
// // 			while (salesQry.next())
// // 			{
// // 				float grossTotal = salesQry.value("total").toFloat();
// // 				float discount = salesQry.value("discount").toFloat();
// // 				float netTotal = grossTotal*(100 - discount) / 100;
// // 				soldQty += salesQry.value("quantity").toFloat();
// // 				subTotal += netTotal;
// // 				if (discount > maxDiscount)
// // 				{
// // 					maxDiscount = discount;
// // 				}
// // 			}
// // 
// // 			float totalRetunedQty = 0;
// // 			float returnTotal = 0;
// // 			QSqlQuery queryReturn("SELECT * FROM return_item WHERE item_id = " + itemId + " AND deleted = 0 AND DATE(date) BETWEEN '" + stardDateStr + "' AND '" + endDateStr + "'");
// // 			while (queryReturn.next())
// // 			{
// // 				float retQty = queryReturn.value("qty").toFloat();
// // 				totalRetunedQty += retQty;
// // 				float returnSubTotal = queryReturn.value("return_total").toFloat();
// // 				returnTotal += returnSubTotal;
// // 			}
// // 			balanceQty = soldQty - totalRetunedQty;
// // 			//totalAmount += subTotal;
// // 
// // 			float netTotal = subTotal - returnTotal;
// // 
// // 			if (balanceQty > 0)
// // 			{
// // 				averagePrice = netTotal / balanceQty;
// // 			}
// // 
// // 			double approximateProfit = netTotal - (purchasingPrice * balanceQty);
// // 			totalProfit += approximateProfit;
// // 
// // 			printRow(tableElement, row, 0, QString::number(purchasingPrice, 'f', 2));
// // 			printRow(tableElement, row, 1, QString::number(currentDiscount, 'f', 2));
// // 			printRow(tableElement, row, 2, QString::number(averagePrice, 'f', 2));
// // 			printRow(tableElement, row, 3, QString::number(soldQty));
// // 			printRow(tableElement, row, 4, QString::number(totalRetunedQty));
// // 			printRow(tableElement, row, 5, QString::number(balanceQty));
// // 			printRow(tableElement, row, 6, QString::number(netTotal, 'f', 2));
// // 			printRow(tableElement, row, 7, QString::number(approximateProfit, 'f', 2));
// // 		}
// 
// 		report->addElement(tableElement);
// 
// 		QPrinter printer;
// 		printer.setPaperSize(QPrinter::A4);
// 
// 		printer.setFullPage(false);
// 		printer.setOrientation(QPrinter::Landscape);
// 
// 		QPrintPreviewDialog *dialog = new QPrintPreviewDialog(&printer, this);
// 		QObject::connect(dialog, SIGNAL(paintRequested(QPrinter*)), this, SLOT(slotPrint(QPrinter*)));
// 		dialog->setWindowTitle(tr("Print Document"));
// 		ES::MainWindowHolder::instance()->getMainWindow()->setCentralWidget(dialog);
// 		dialog->exec();
// 	}
// }

void MDFSalesSummary::slotGenerateReport()
{
	report = new KDReports::Report;

	KDReports::TextElement titleElement("MDF Profit/Loss Summary");
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

	KDReports::TableElement tableElement;
	tableElement.setHeaderColumnCount(10);
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
		KDReports::TextElement cTextElement("Cost");
		cTextElement.setPointSize(11);
		cTextElement.setBold(true);
		cell.addElement(cTextElement, Qt::AlignCenter);
	}
	{
		KDReports::Cell& cell = tableElement.cell(0, 2);
		KDReports::TextElement cTextElement("Price");
		cTextElement.setPointSize(11);
		cTextElement.setBold(true);
		cell.addElement(cTextElement, Qt::AlignCenter);
	}
	{
		KDReports::Cell& cell = tableElement.cell(0, 3);
		KDReports::TextElement cTextElement("Sold Qty");
		cTextElement.setPointSize(11);
		cTextElement.setBold(true);
		cell.addElement(cTextElement, Qt::AlignCenter);
	}
	{
		KDReports::Cell& cell = tableElement.cell(0, 4);
		KDReports::TextElement cTextElement("Total Sales");
		cTextElement.setPointSize(11);
		cTextElement.setBold(true);
		cell.addElement(cTextElement, Qt::AlignCenter);
	}
	{
		KDReports::Cell& cell = tableElement.cell(0, 5);
		KDReports::TextElement cTextElement("Sales Cost");
		cTextElement.setPointSize(11);
		cTextElement.setBold(true);
		cell.addElement(cTextElement, Qt::AlignCenter);
	}

	{
		KDReports::Cell& cell = tableElement.cell(0, 6);
		KDReports::TextElement cTextElement("Return Qty");
		cTextElement.setPointSize(11);
		cTextElement.setBold(true);
		cell.addElement(cTextElement, Qt::AlignCenter);
	}
	{
		KDReports::Cell& cell = tableElement.cell(0, 7);
		KDReports::TextElement cTextElement("Return Total");
		cTextElement.setPointSize(11);
		cTextElement.setBold(true);
		cell.addElement(cTextElement, Qt::AlignCenter);
	}
	{
		KDReports::Cell& cell = tableElement.cell(0, 8);
		KDReports::TextElement cTextElement("Return Cost");
		cTextElement.setPointSize(11);
		cTextElement.setBold(true);
		cell.addElement(cTextElement, Qt::AlignCenter);
	}

	{
		KDReports::Cell& cell = tableElement.cell(0, 9);
		KDReports::TextElement cTextElement("Profit");
		cTextElement.setPointSize(11);
		cTextElement.setBold(true);
		cell.addElement(cTextElement, Qt::AlignCenter);
	}
	int row = 1;
	float grandReturnTotal, grandReturnCost = 0, grandSaleCost = 0, grandSalesTotal = 0, totalProfit = 0, netTotalIncome = 0;

	QVector<MDFItemDataHolder> dataHolderVec;
	QString qMDFStr = "SELECT * FROM Item JOIN item_category ON item.itemcategory_id = item_category.itemcategory_id WHERE item.deleted = 0 AND item_category.itemcategory_name ='MDF'";
	QSqlQuery categoryQry;
	categoryQry.setForwardOnly(true);
	categoryQry.exec(qMDFStr);
	while (categoryQry.next())
	{
		QString itemId = categoryQry.value("item_id").toString();
		QString itemName = categoryQry.value("item_name").toString();
		QString stockQryStr = "SELECT * FROM stock_purchase_order_item JOIN stock ON stock_purchase_order_item.stock_id = stock.stock_id WHERE stock.deleted = 0 AND stock.item_id = " + itemId;
		QSqlQuery qryStock;
		qryStock.setForwardOnly(true);
		qryStock.exec(stockQryStr);
		if (qryStock.next())
		{
			QString stockId = qryStock.value("stock_id").toString();
			MDFItemDataHolder midh;
			midh.itemName = itemName;
			midh.stockId = stockId;
			midh.itemId = itemId;
			dataHolderVec.push_back(midh);
		}
	}
	bool printLine = false;
	for (auto midh : dataHolderVec)
	{
		printLine = false;
		float itemCost = 0;
		float lineTotal = 0;
		float lineQty = 0;
		float lineCost = 0;
		QString salesQryStr = "SELECT discount, SUM(total) as totalAmount, SUM(quantity) as totalQty, w_cost FROM sale WHERE stock_id = " + midh.stockId + " AND deleted = 0 AND  DATE(date) BETWEEN '" + stardDateStr + "' AND '" + endDateStr + "'";
		QSqlQuery salesQry;
		salesQry.setForwardOnly(true);
		salesQry.exec(salesQryStr);

		if (salesQry.next())
		{
			lineTotal = salesQry.value("totalAmount").toFloat();
			grandSalesTotal += lineTotal;
			float discount = salesQry.value("discount").toFloat();
			lineQty = salesQry.value("totalQty").toFloat();
			float cost = salesQry.value("w_cost").toFloat();
			lineCost = cost*lineQty;

			grandReturnCost += lineCost;
			itemCost = lineCost / lineQty;
			float avgSellingPrice = lineTotal / lineQty;
			if (lineTotal > 0)
			{
				printRow(tableElement, row, 1, QString::number(itemCost, 'f', 2));
				printRow(tableElement, row, 2, QString::number(avgSellingPrice, 'f', 2));
				printRow(tableElement, row, 3, QString::number(lineQty));
				printRow(tableElement, row, 4, QString::number(lineTotal, 'f', 2));
				printRow(tableElement, row, 5, QString::number(lineCost, 'f', 2));
				printLine = true;
			}

		}

		float returnTotal = 0;
		float totalReturnCost = 0;
		QSqlQuery queryReturn("SELECT SUM(qty) as totalQty , SUM(return_total) as retTotal FROM return_item WHERE item_id = " + midh.itemId + " AND deleted = 0 AND DATE(date) BETWEEN '" + stardDateStr + "' AND '" + endDateStr + "'");
		if (queryReturn.next())
		{
			if (printLine)
			{
				float lineReturnQty = queryReturn.value("totalQty").toFloat();
				float lineReturn = queryReturn.value("retTotal").toFloat();
				grandReturnTotal += lineReturn;

				printRow(tableElement, row, 6, QString::number(lineReturnQty));
				printRow(tableElement, row, 7, QString::number(lineReturn, 'f', 2));

				totalReturnCost = lineReturnQty * itemCost;
				grandReturnCost += totalReturnCost;

				printRow(tableElement, row, 8, QString::number(totalReturnCost, 'f', 2));
			}
		}
		if (printLine)
		{
			float itemProfit = (lineTotal - lineCost) - (returnTotal - totalReturnCost);
			totalProfit += itemProfit;
			netTotalIncome += (lineTotal - returnTotal);
			printRow(tableElement, row, 0, midh.itemName);
			printRow(tableElement, row, 9, QString::number(itemProfit, 'f', 2));
			row++;
		}
	}
	printRow(tableElement, row, 3, "Net. Sales");
	printRow(tableElement, row, 4, QString::number(netTotalIncome, 'f', 2));
	printRow(tableElement, row, 8, "Profit ");
	printRow(tableElement, row, 9, QString::number(totalProfit, 'f', 2));
	report->addElement(tableElement);

	QPrinter printer;
	printer.setPaperSize(QPrinter::A4);

	printer.setFullPage(false);
	printer.setOrientation(QPrinter::Landscape);

	QPrintPreviewDialog *dialog = new QPrintPreviewDialog(&printer, this);
	QObject::connect(dialog, SIGNAL(paintRequested(QPrinter*)), this, SLOT(slotPrint(QPrinter*)));
	dialog->setWindowTitle(tr("Print Document"));
	ES::MainWindowHolder::instance()->getMainWindow()->setCentralWidget(dialog);
	dialog->exec();

}
