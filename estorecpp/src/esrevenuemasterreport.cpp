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
			QString priceStr = querySale.value("item_price").toString();
			QString discountStr = querySale.value("discount").toString();
			double cost = querySale.value("w_cost").toDouble();
			double lineTotal = querySale.value("total").toDouble();
			double lineTotalQty = querySale.value("qty").toDouble();
			QString stockId = querySale.value("stock_id").toString();

			double totalCost = cost*lineTotalQty;
			grandSalesTotal += lineTotal;
			salesGrandCost += totalCost;
			QString queryStockItemStr = "SELECT item.item_name, item.item_id FROM stock, item WHERE stock.item_id = item.item_id AND stock.stock_id = "+stockId;
			QSqlQuery queryStockItem;
			queryStockItem.prepare(queryStockItemStr);
			queryStockItem.setForwardOnly(true);
			queryStockItem.exec();
			if (queryStockItem.first())
			{
				int row = ui.tableWidget->rowCount();
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
					returnLineTotal = queryReturn.value("retTotal").toFloat();
				}
				float returnCost = returnLineQty * cost;
				returnGrandCost += returnCost;
				returnGrandTotal += returnLineTotal;

				QTableWidgetItem *itemNameWidget = new QTableWidgetItem(itemName);
				itemNameWidget->setTextAlignment(Qt::AlignLeft);
				ui.tableWidget->setItem(row, 0, itemNameWidget);

				QTableWidgetItem *itemPriceWidget = new QTableWidgetItem(priceStr);
				itemPriceWidget->setTextAlignment(Qt::AlignRight);
				ui.tableWidget->setItem(row, 1, itemPriceWidget);

				QTableWidgetItem *itemQtyWidget = new QTableWidgetItem(QString::number(lineTotalQty));
				itemQtyWidget->setTextAlignment(Qt::AlignRight);
				ui.tableWidget->setItem(row, 2, itemQtyWidget);

				QTableWidgetItem *discountWidget = new QTableWidgetItem(discountStr);
				discountWidget->setTextAlignment(Qt::AlignRight);
				ui.tableWidget->setItem(row, 3, discountWidget);

				QTableWidgetItem *totalSalesWidget = new QTableWidgetItem(QString::number(lineTotal, 'f', 2));
				totalSalesWidget->setTextAlignment(Qt::AlignRight);
				ui.tableWidget->setItem(row, 4, totalSalesWidget);

				QTableWidgetItem *itemCostWidget = new QTableWidgetItem(QString::number(cost, 'f', 2));
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

			}
		}
	
	}
	double profit = (grandSalesTotal - returnGrandTotal) - (salesGrandCost - returnGrandCost);
	ui.profitLbl->setText(QString::number(profit,'f', 2));
// 	headerLabels.append("Item");
// 	headerLabels.append("Sale Price");
// 	headerLabels.append("Sale Qty");
// 	headerLabels.append("Total Sales");
// 	headerLabels.append("Discount");
// 	headerLabels.append("Item Cost");
// 	headerLabels.append("Total Cost");
// 	headerLabels.append("Return Qty");
// 	headerLabels.append("Return Total");
// 	headerLabels.append("Return Cost");


// 
// 
// 	float totalIncome = totalSalesIncome /*+ allReturnedItemCost*/;
// 	float totalExpences = totalSalesItemCost/* + allReturnTotal*/;
// 	float profit = totalIncome - totalExpences;
// 
// 	QTableWidgetItem *profitWidget = new QTableWidgetItem(QString::number(profit, 'f', 2));
// 	profitWidget->setTextAlignment(Qt::AlignRight);
// 	ui.tableWidget->setItem(row, 4, profitWidget);

}

void ESRevenueMasterReport::printRow(KDReports::TableElement& tableElement, int row, int col, QString elementStr, Qt::AlignmentFlag alignment /*= Qt::AlignLeft*/)
{
	KDReports::Cell& cell = tableElement.cell(row, col);
	KDReports::TextElement te(elementStr);
	te.setPointSize(ES::Session::getInstance()->getBillItemFontSize());
	cell.addElement(te, alignment);
}

Ui::RevenueMasterReport& ESRevenueMasterReport::getUI()
{
	return ui;
}


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

	KDReports::TableElement tableElement;
	tableElement.setHeaderColumnCount(5);
	tableElement.setBorder(1);
	tableElement.setWidth(100, KDReports::Percent);

	{
		KDReports::Cell& cell = tableElement.cell(0, 0);
		KDReports::TextElement cTextElement("Sales");
		cTextElement.setPointSize(11);
		cTextElement.setBold(true);
		cell.addElement(cTextElement, Qt::AlignCenter);
	}
	{
		KDReports::Cell& cell = tableElement.cell(0, 1);
		KDReports::TextElement cTextElement("Cost Of Sales");
		cTextElement.setPointSize(11);
		cTextElement.setBold(true);
		cell.addElement(cTextElement, Qt::AlignCenter);
	}
	{
		KDReports::Cell& cell = tableElement.cell(0, 2);
		KDReports::TextElement cTextElement("Return");
		cTextElement.setPointSize(11);
		cTextElement.setBold(true);
		cell.addElement(cTextElement, Qt::AlignCenter);
	}
	{
		KDReports::Cell& cell = tableElement.cell(0, 3);
		KDReports::TextElement cTextElement("Cost of Return");
		cTextElement.setPointSize(11);
		cTextElement.setBold(true);
		cell.addElement(cTextElement, Qt::AlignCenter);
	}
// 	{
// 		KDReports::Cell& cell = tableElement.cell(0, 4);
// 		KDReports::TextElement cTextElement("P/C Income");
// 		cTextElement.setPointSize(11);
// 		cTextElement.setBold(true);
// 		cell.addElement(cTextElement, Qt::AlignCenter);
// 	}
// 	{
// 		KDReports::Cell& cell = tableElement.cell(0, 5);
// 		KDReports::TextElement cTextElement("P/C Expenses");
// 		cTextElement.setPointSize(11);
// 		cTextElement.setBold(true);
// 		cell.addElement(cTextElement, Qt::AlignCenter);
// 	}
// 	{
// 		KDReports::Cell& cell = tableElement.cell(0, 4);
// 		KDReports::TextElement cTextElement("Interest Earnings");
// 		cTextElement.setPointSize(11);
// 		cTextElement.setBold(true);
// 		cell.addElement(cTextElement, Qt::AlignCenter);
// 	}
	{
		KDReports::Cell& cell = tableElement.cell(0, 4);
		KDReports::TextElement cTextElement("Approx. Profit");
		cTextElement.setPointSize(11);
		cTextElement.setBold(true);
		cell.addElement(cTextElement, Qt::AlignCenter);
	}

	float allBillTotal = 0, allBillProfit = 0, allReturnTotal = 0, allSoldItemCost = 0, allReturnedItemCost = 0, allBillInterest = 0;
	QSqlQuery userQry("SELECT * FROM user JOIN usertype ON user.usertype_id = usertype.usertype_id WHERE user.active = 1");
	while (userQry.next())
	{
		QString uId = userQry.value("user_id").toString();
		QString uName = userQry.value("display_name").toString();
		QSqlQuery queryUserType("SELECT * FROM user JOIN usertype ON user.usertype_id = usertype.usertype_id WHERE user.user_id = " + uId + " AND usertype.usertype_name <> 'DEV'");
		if (queryUserType.next())
		{
			float cashSales = 0, creditSales = 0, chequeSales = 0, cardSales = 0;

			QString qUserStr;
			if (ES::Session::getInstance()->getUser()->getType() == ES::User::SENIOR_MANAGER ||
				ES::Session::getInstance()->getUser()->getType() == ES::User::DEV)
			{
				qUserStr = "SELECT * FROM bill WHERE status = 1 AND bill.user_id = " + uId + " AND  DATE(date) BETWEEN '" + stardDateStr + "' AND '" + endDateStr + "'";
			}
			else
			{
				qUserStr = "SELECT * FROM bill WHERE status = 1 AND visible = 1 AND bill.user_id = " + uId + " AND  DATE(date) BETWEEN '" + stardDateStr + "' AND '" + endDateStr + "'";
			}

			QSqlQuery userBillQry(qUserStr);
			while (userBillQry.next())
			{
				float billTotal = 0, billProfit = 0;
				QString billId = userBillQry.value("bill_id").toString();
				QSqlQuery paymentQry("SELECT * FROM payment WHERE valid = 1 AND bill_id = " + billId);
				while (paymentQry.next())
				{
					QString paymentType = paymentQry.value("payment_type").toString();
					QString paymentId = paymentQry.value("payment_id").toString();
					double tot = paymentQry.value("total_amount").toDouble();

					if (paymentType == "CASH")
					{
						cashSales += tot;
						billTotal += tot;
					}
					else if (paymentType == "CREDIT")
					{
						QSqlQuery creditSaleQry("SELECT * FROM credit WHERE payment_id = " + paymentId);
						while (creditSaleQry.next())
						{
							double amount = creditSaleQry.value("amount").toDouble();
							double interest = creditSaleQry.value("interest").toDouble();
							amount = (amount * (100 + interest) / 100);
							allBillInterest += (amount*interest / 100);
							creditSales += amount;
							billTotal += amount;
						}
					}
					else if (paymentType == "CHEQUE")
					{
						QSqlQuery chequeSaleQry("SELECT * FROM cheque WHERE payment_id = " + paymentId);
						while (chequeSaleQry.next())
						{
							double amount = chequeSaleQry.value("amount").toDouble();
							double interest = chequeSaleQry.value("interest").toDouble();
							allBillInterest += (amount*interest / 100);
							amount = (amount * (100 + interest) / 100);
							chequeSales += amount;
							billTotal += amount;
						}
					}
					else if (paymentType == "CARD")
					{
						QSqlQuery cardSaleQry("SELECT * FROM card WHERE payment_id = " + paymentId);
						while (cardSaleQry.next())
						{
							double amount = cardSaleQry.value("amount").toDouble();
							double interest = cardSaleQry.value("interest").toDouble();
							allBillInterest += (amount*interest / 100);
							amount = (amount * (100 + interest) / 100);
							cardSales += amount;
							billTotal += amount;
						}
					}
					else if (paymentType == "LOYALTY")
					{
					}
				}

				//item wise start
				double totalProfit = 0;

				float totalIncomeOfItems = 0, totalCostOfItems = 0, totalReturned = 0, totalCostOfReturnedItems = 0;
				QString salesQryStr = "SELECT quantity, discount, total,stock_id, item_price from sale where deleted = 0 AND bill_id = " + billId + " AND DATE(date) BETWEEN '" + stardDateStr + "' AND '" + endDateStr + "'";
				QSqlQuery salesQry(salesQryStr);
				while (salesQry.next())
				{
					QString stockId = salesQry.value("stock_id").toString();
					QSqlQuery qryStock("SELECT * FROM stock_purchase_order_item JOIN stock ON stock_purchase_order_item.stock_id = stock.stock_id WHERE stock.deleted = 0 AND stock.stock_id = " + stockId);
					if (qryStock.next())
					{
						QString itemName = "";
						float sellingPrice = qryStock.value("selling_price").toFloat();
						float currentDiscount = qryStock.value("discount").toFloat();
						float purchasingPrice = qryStock.value("purchasing_price").toFloat();
						QString itemId = qryStock.value("item_id").toString();

						float itemPrice = salesQry.value("item_price").toFloat();
						float soldQty = salesQry.value("quantity").toFloat();
						float discount = salesQry.value("discount").toFloat();

						QSqlQuery queryItem("SELECT * FROM item WHERE item_id = " + itemId);
						if (queryItem.next())
						{
							itemName = queryItem.value("item_name").toString();
						}
						QSqlQuery q1("SELECT * FROM return_bill WHERE return_bill_id = " + billId);
						if (q1.next())
						{
							QSqlQuery q2("SELECT * FROM return_item WHERE item_id = " + itemId + " AND bill_id = " + billId + " AND deleted = 0 ");
							float retQty = q2.value("qty").toFloat();
							totalCostOfItems -= (purchasingPrice * retQty);
						}
						else
						{
							totalCostOfItems += (purchasingPrice * soldQty);
						}
// 						QSqlQuery queryReturn("SELECT * FROM return_item WHERE item_id = " + itemId + " AND bill_id = " + billId + " AND deleted = 0 AND DATE(date) BETWEEN '" + stardDateStr + "' AND '" + endDateStr + "'");
// 						float retQty = 0;
// 
// 						while (queryReturn.next())
// 						{
// 							retQty = queryReturn.value("qty").toFloat();
// 							float retTotal = queryReturn.value("return_total").toFloat();
// 							totalReturned += retTotal;
// 							float costOfReturned = (purchasingPrice*retQty);
// 							totalCostOfReturnedItems += costOfReturned;
// 							soldQty -= retQty;
// 						}
// 						float totalAmountPerItem = itemPrice*((100 - discount) / 100)*soldQty;
// 						float costOfSoldItems = (purchasingPrice*soldQty);
// 						totalCostOfItems += costOfSoldItems;
// 						totalIncomeOfItems += totalAmountPerItem;
					}
				}
				allSoldItemCost += totalCostOfItems;
				allBillTotal += billTotal;
				allReturnedItemCost += totalCostOfReturnedItems;
				allReturnTotal += totalReturned;
				totalCostOfItems = 0;
				billTotal = 0;
				totalReturned = 0;
				totalCostOfReturnedItems = 0;
				//item wise end
			}

		}
	}

	float totalIncome = allBillTotal/* + allReturnedItemCost*/;
	float totalExpenses = allSoldItemCost/*+allReturnTotal*/;
	float profit = totalIncome - totalExpenses;

	int row = 1;
	printRow(tableElement, row, 0, QString::number(allBillTotal,'f',2));
	printRow(tableElement, row, 1, QString::number(allSoldItemCost, 'f', 2));
	printRow(tableElement, row, 2, QString::number(allReturnTotal, 'f', 2));
	printRow(tableElement, row, 3, QString::number(allReturnedItemCost, 'f', 2));
	printRow(tableElement, row, 4, QString::number(profit, 'f', 2));

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
