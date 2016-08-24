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

MDFSalesSummary::MDFSalesSummary(QWidget *parent /*= 0*/) : QWidget(parent), report(NULL)
{
	ui.setupUi(this);
	m_generateReportSignalMapper = new QSignalMapper(this);
	ui.fromDate->setDate(QDate::currentDate());
	ui.toDate->setDate(QDate::currentDate().addDays(1));

	QStringList headerLabels;
	headerLabels.append("Item Cost");
	headerLabels.append("Discount");
	headerLabels.append("Avg. Sold Price");
	headerLabels.append("Sold Qty");
	headerLabels.append("Returned Qty");
	headerLabels.append("Net Qty");
	headerLabels.append("Line Total");
	headerLabels.append("Approx. Profit");
	headerLabels.append("Action");

	QFont font = this->font();
	font.setBold(true);
	font.setPointSize(12);

	ui.tableWidgetByCategory->setHorizontalHeaderLabels(headerLabels);
	ui.tableWidgetByCategory->horizontalHeader()->setStretchLastSection(true);
	ui.tableWidgetByCategory->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tableWidgetByCategory->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableWidgetByCategory->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableWidgetByCategory->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.tableWidgetByCategory->verticalHeader()->setMinimumWidth(200);

	QObject::connect(ui.generateButton, SIGNAL(clicked()), this, SLOT(slotGenerateReport()));
	QObject::connect(ui.fromDate, SIGNAL(dateChanged(const QDate &)), this, SLOT(slotSearch()));
	QObject::connect(ui.toDate, SIGNAL(dateChanged(const QDate &)), this, SLOT(slotSearch()));
	QObject::connect(m_generateReportSignalMapper, SIGNAL(mapped(QString)), this, SLOT(slotGenerateReportForGivenItem(QString)));
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
	while (ui.tableWidgetByCategory->rowCount() > 0)
	{
		ui.tableWidgetByCategory->removeRow(0);
	}

	QString stardDateStr = ui.fromDate->date().toString("yyyy-MM-dd");
	QString endDateStr = ui.toDate->date().toString("yyyy-MM-dd");
	double totalProfit = 0;
	QSqlQuery categoryQry("SELECT * FROM Item JOIN item_category ON item.itemcategory_id = item_category.itemcategory_id WHERE item.deleted = 0 AND item_category.itemcategory_name ='MDF' GROUP BY item.itemcategory_id");
	while (categoryQry.next())
	{
		float grandTotal = 0, averagePrice = 0, totalQty = 0, totalAmount = 0;
		QString itemId = categoryQry.value("item_id").toString();
		QString itemName = categoryQry.value("item_name").toString();
		QSqlQuery qryStock("SELECT * FROM stock_purchase_order_item JOIN stock ON stock_purchase_order_item.stock_id = stock.stock_id WHERE stock.deleted = 0 AND stock.item_id = " + itemId);
		if (qryStock.next())
		{
			QString stockId = qryStock.value("stock_id").toString();
			float sellingPrice = qryStock.value("selling_price").toFloat();
			float currentDiscount = qryStock.value("discount").toFloat();
			float purchasingPrice = qryStock.value("purchasing_price").toFloat();

			int row = ui.tableWidgetByCategory->rowCount();
			ui.tableWidgetByCategory->insertRow(row);

			QTableWidgetItem* nameItem = new QTableWidgetItem(itemName);
			ui.tableWidgetByCategory->setVerticalHeaderItem(row, nameItem);
			float itemPrice = 0, balanceQty = 0, discount = 0, subTotal = 0, soldQty = 0, totalRetunedQty = 0;
			QString salesQryStr = "SELECT discount, item_price, quantity FROM sale WHERE stock_id = " + stockId + " AND  DATE(date) BETWEEN '" + stardDateStr + "' AND '" + endDateStr + "'";
			QSqlQuery salesQry(salesQryStr);
			while (salesQry.next())
			{
				itemPrice = salesQry.value("item_price").toFloat();
				balanceQty = salesQry.value("quantity").toFloat();
				soldQty = salesQry.value("quantity").toFloat();
				discount = salesQry.value("discount").toFloat();

				QSqlQuery queryReturn("SELECT * FROM return_item WHERE item_id = " + itemId + " AND deleted = 0 AND DATE(date) BETWEEN '" + stardDateStr + "' AND '" + endDateStr + "'");
				while (queryReturn.next())
				{
					float retQty = queryReturn.value("qty").toFloat();
					totalRetunedQty += retQty;
				}
				balanceQty -= totalRetunedQty;
				subTotal = itemPrice*((100 - discount) / 100)*balanceQty;
				totalAmount += subTotal;

			}
			if (balanceQty > 0)
			{
				averagePrice = totalAmount / balanceQty;
			}

			QTableWidgetItem *purchasePriceWidget = new QTableWidgetItem(QString::number(purchasingPrice, 'f', 2));
			purchasePriceWidget->setTextAlignment(Qt::AlignRight);
			ui.tableWidgetByCategory->setItem(row, 0, purchasePriceWidget);

			QTableWidgetItem *discountWidget = new QTableWidgetItem(QString::number(currentDiscount, 'f', 2));
			discountWidget->setTextAlignment(Qt::AlignRight);
			ui.tableWidgetByCategory->setItem(row, 1, discountWidget);

			QTableWidgetItem *avgPriceWidget = new QTableWidgetItem(QString::number(averagePrice, 'f', 2));
			avgPriceWidget->setTextAlignment(Qt::AlignRight);
			ui.tableWidgetByCategory->setItem(row, 2, avgPriceWidget);

			QTableWidgetItem *totalQtyWidget = new QTableWidgetItem(QString::number(soldQty));
			totalQtyWidget->setTextAlignment(Qt::AlignRight);
			ui.tableWidgetByCategory->setItem(row, 3, totalQtyWidget);


			QTableWidgetItem *returnedQtyWidget = new QTableWidgetItem(QString::number(totalRetunedQty));
			returnedQtyWidget->setTextAlignment(Qt::AlignRight);
			ui.tableWidgetByCategory->setItem(row, 4, returnedQtyWidget);


			QTableWidgetItem *totalBalanceQtyWidget = new QTableWidgetItem(QString::number(balanceQty));
			totalBalanceQtyWidget->setTextAlignment(Qt::AlignRight);
			ui.tableWidgetByCategory->setItem(row, 5, totalBalanceQtyWidget);

			QTableWidgetItem *totalAmountWidget = new QTableWidgetItem(QString::number(totalAmount, 'f', 2));
			totalAmountWidget->setTextAlignment(Qt::AlignRight);
			ui.tableWidgetByCategory->setItem(row, 6, totalAmountWidget);

			double approximateProfit = totalAmount - (purchasingPrice*((100 - currentDiscount) / 100)*balanceQty);
			totalProfit += approximateProfit;
			QTableWidgetItem *profitWidget = new QTableWidgetItem(QString::number(approximateProfit, 'f', 2));
			profitWidget->setTextAlignment(Qt::AlignRight);
			ui.tableWidgetByCategory->setItem(row, 7, profitWidget);

			QWidget* base = new QWidget(ui.tableWidgetByCategory);

			QPushButton* generateReportBtn = new QPushButton(base);
			generateReportBtn->setIcon(QIcon("icons/pdf.png"));
			generateReportBtn->setIconSize(QSize(24, 24));
			generateReportBtn->setMaximumWidth(100);

			m_generateReportSignalMapper->setMapping(generateReportBtn, itemId);
			QObject::connect(generateReportBtn, SIGNAL(clicked()), m_generateReportSignalMapper, SLOT(map()));

			QHBoxLayout *layout = new QHBoxLayout;
			layout->setContentsMargins(0, 0, 0, 0);
			layout->addWidget(generateReportBtn);
			layout->insertStretch(2);
			base->setLayout(layout);
			ui.tableWidgetByCategory->setCellWidget(row, 8, base);
			base->show();
		}
	}

	ui.totalProfitLbl->setText(QString::number(totalProfit, 'f', 2));
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

void MDFSalesSummary::slotGenerateReportForGivenItem(QString itemId)
{
	report = new KDReports::Report;
	float averagePrice = 0, totalQty = 0, totalAmount = 0, totalProfit = 0;
	QSqlQuery queryItems("SELECT * FROM Item WHERE item_id = " + itemId);
	if (queryItems.next())
	{
		QString itemName = queryItems.value("item_name").toString();

		KDReports::TextElement titleElement("Profit/Loss Summary of " + itemName);
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
		tableElement.setHeaderColumnCount(7);
		tableElement.setBorder(1);
		tableElement.setWidth(100, KDReports::Percent);

		{
			KDReports::Cell& cell = tableElement.cell(0, 0);
			KDReports::TextElement cTextElement("Cost");
			cTextElement.setPointSize(11);
			cTextElement.setBold(true);
			cell.addElement(cTextElement, Qt::AlignCenter);
		}
		{
			KDReports::Cell& cell = tableElement.cell(0, 1);
			KDReports::TextElement cTextElement("Discount");
			cTextElement.setPointSize(11);
			cTextElement.setBold(true);
			cell.addElement(cTextElement, Qt::AlignCenter);
		}
		{
			KDReports::Cell& cell = tableElement.cell(0, 2);
			KDReports::TextElement cTextElement("Avg. Sold Price");
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
			KDReports::TextElement cTextElement("Returned Qty");
			cTextElement.setPointSize(11);
			cTextElement.setBold(true);
			cell.addElement(cTextElement, Qt::AlignCenter);
		}
		{
			KDReports::Cell& cell = tableElement.cell(0, 5);
			KDReports::TextElement cTextElement("Net sold Qty");
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
		{
			KDReports::Cell& cell = tableElement.cell(0, 7);
			KDReports::TextElement cTextElement("Approx. Profit");
			cTextElement.setPointSize(11);
			cTextElement.setBold(true);
			cell.addElement(cTextElement, Qt::AlignCenter);
		}

		int row = 1;
		float sellingPrice = 0, currentDiscount = 0, purchasingPrice = 0;
		float itemPrice = 0, balanceQty = 0, discount = 0, subTotal = 0, soldQty = 0, totalRetunedQty = 0;
		QSqlQuery qryStock("SELECT * FROM stock_purchase_order_item JOIN stock ON stock_purchase_order_item.stock_id = stock.stock_id WHERE stock.deleted = 0 AND stock.item_id = " + itemId);
		if (qryStock.next())
		{
			QString stockId = qryStock.value("stock_id").toString();
			sellingPrice = qryStock.value("selling_price").toFloat();
			currentDiscount = qryStock.value("discount").toFloat();
			purchasingPrice = qryStock.value("purchasing_price").toFloat();
			
			QTableWidgetItem* nameItem = new QTableWidgetItem(itemName);
			ui.tableWidgetByCategory->setVerticalHeaderItem(row, nameItem);
			QString salesQryStr = "SELECT discount, item_price, quantity FROM sale WHERE stock_id = " + stockId + " AND  DATE(date) BETWEEN '" + stardDateStr + "' AND '" + endDateStr + "'";
			QSqlQuery salesQry(salesQryStr);
			while (salesQry.next())
			{
				itemPrice = salesQry.value("item_price").toFloat();
				balanceQty = salesQry.value("quantity").toFloat();
				soldQty = salesQry.value("quantity").toFloat();
				discount = salesQry.value("discount").toFloat();

				QSqlQuery queryReturn("SELECT * FROM return_item WHERE item_id = " + itemId + " AND deleted = 0 AND DATE(date) BETWEEN '" + stardDateStr + "' AND '" + endDateStr + "'");
				while (queryReturn.next())
				{
					float retQty = queryReturn.value("qty").toFloat();
					totalRetunedQty += retQty;
				}
				balanceQty -= totalRetunedQty;
				subTotal = itemPrice*((100 - discount) / 100)*balanceQty;
				totalAmount += subTotal;

			}
			if (balanceQty > 0)
			{
				averagePrice = totalAmount / balanceQty;
			}
		}
		double approximateProfit = totalAmount - (purchasingPrice*((100 - currentDiscount) / 100)*balanceQty);
		totalProfit += approximateProfit;

		printRow(tableElement, row, 0, QString::number(purchasingPrice, 'f', 2));
		printRow(tableElement, row, 1, QString::number(currentDiscount, 'f', 2));
		printRow(tableElement, row, 2, QString::number(averagePrice, 'f', 2));
		printRow(tableElement, row, 3, QString::number(totalQty));
		printRow(tableElement, row, 4, QString::number(totalRetunedQty));
		printRow(tableElement, row, 5, QString::number(balanceQty));
		printRow(tableElement, row, 6, QString::number(totalAmount, 'f', 2));
		printRow(tableElement, row, 7, QString::number(approximateProfit, 'f', 2));

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
}

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
	tableElement.setHeaderColumnCount(8);
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
		KDReports::TextElement cTextElement("Discount");
		cTextElement.setPointSize(11);
		cTextElement.setBold(true);
		cell.addElement(cTextElement, Qt::AlignCenter);
	}
	{
		KDReports::Cell& cell = tableElement.cell(0, 3);
		KDReports::TextElement cTextElement("Avg. Sold Price");
		cTextElement.setPointSize(11);
		cTextElement.setBold(true);
		cell.addElement(cTextElement, Qt::AlignCenter);
	}
	{
		KDReports::Cell& cell = tableElement.cell(0, 4);
		KDReports::TextElement cTextElement("Sold Qty");
		cTextElement.setPointSize(11);
		cTextElement.setBold(true);
		cell.addElement(cTextElement, Qt::AlignCenter);
	}
	{
		KDReports::Cell& cell = tableElement.cell(0, 5);
		KDReports::TextElement cTextElement("Returned Qty");
		cTextElement.setPointSize(11);
		cTextElement.setBold(true);
		cell.addElement(cTextElement, Qt::AlignCenter);
	}
	{
		KDReports::Cell& cell = tableElement.cell(0, 6);
		KDReports::TextElement cTextElement("Net sold Qty");
		cTextElement.setPointSize(11);
		cTextElement.setBold(true);
		cell.addElement(cTextElement, Qt::AlignCenter);
	}
	{
		KDReports::Cell& cell = tableElement.cell(0, 7);
		KDReports::TextElement cTextElement("Line Total");
		cTextElement.setPointSize(11);
		cTextElement.setBold(true);
		cell.addElement(cTextElement, Qt::AlignCenter);
	}
	{
		KDReports::Cell& cell = tableElement.cell(0, 8);
		KDReports::TextElement cTextElement("Approx. Profit");
		cTextElement.setPointSize(11);
		cTextElement.setBold(true);
		cell.addElement(cTextElement, Qt::AlignCenter);
	}

	int row = 1;
	
	float grandTotal = 0, totalQty = 0, totalProfit = 0;
	QSqlQuery categoryQry("SELECT * FROM Item JOIN item_category ON item.itemcategory_id = item_category.itemcategory_id WHERE item.deleted = 0 AND item_category.itemcategory_name ='MDF' GROUP BY item.itemcategory_id");
	while (categoryQry.next())
	{
		float totalAmount = 0, averagePrice = 0;
		QString itemId = categoryQry.value("item_id").toString();
		QString itemName = categoryQry.value("item_name").toString();
		float sellingPrice = 0, currentDiscount = 0, purchasingPrice = 0;
		float itemPrice = 0, balanceQty = 0, discount = 0, subTotal = 0, soldQty = 0, totalRetunedQty = 0;
		QSqlQuery qryStock("SELECT * FROM stock_purchase_order_item JOIN stock ON stock_purchase_order_item.stock_id = stock.stock_id WHERE stock.deleted = 0 AND stock.item_id = " + itemId);
		if (qryStock.next())
		{
			totalAmount = 0;
			QString stockId = qryStock.value("stock_id").toString();
			sellingPrice = qryStock.value("selling_price").toFloat();
			currentDiscount = qryStock.value("discount").toFloat();
			purchasingPrice = qryStock.value("purchasing_price").toFloat();

			QTableWidgetItem* nameItem = new QTableWidgetItem(itemName);
			ui.tableWidgetByCategory->setVerticalHeaderItem(row, nameItem);
			QString salesQryStr = "SELECT discount, item_price, quantity FROM sale WHERE stock_id = " + stockId + " AND  DATE(date) BETWEEN '" + stardDateStr + "' AND '" + endDateStr + "'";
			QSqlQuery salesQry(salesQryStr);
			while (salesQry.next())
			{
				itemPrice = salesQry.value("item_price").toFloat();
				balanceQty = salesQry.value("quantity").toFloat();
				soldQty = salesQry.value("quantity").toFloat();
				discount = salesQry.value("discount").toFloat();

				QSqlQuery queryReturn("SELECT * FROM return_item WHERE item_id = " + itemId + " AND deleted = 0 AND DATE(date) BETWEEN '" + stardDateStr + "' AND '" + endDateStr + "'");
				while (queryReturn.next())
				{
					float retQty = queryReturn.value("qty").toFloat();
					totalRetunedQty += retQty;
				}
				balanceQty -= totalRetunedQty;
				subTotal = itemPrice*((100 - discount) / 100)*balanceQty;
				totalAmount += subTotal;

			}
			if (balanceQty > 0)
			{
				averagePrice = totalAmount / balanceQty;
			}
		}
		double approximateProfit = totalAmount - (purchasingPrice*((100 - currentDiscount) / 100)*balanceQty);
		totalProfit += approximateProfit;

		printRow(tableElement, row, 0, itemName);
		printRow(tableElement, row, 1, QString::number(purchasingPrice, 'f', 2));
		printRow(tableElement, row, 2, QString::number(currentDiscount, 'f', 2));
		printRow(tableElement, row, 3, QString::number(averagePrice, 'f', 2));
		printRow(tableElement, row, 4, QString::number(totalQty));
		printRow(tableElement, row, 5, QString::number(totalRetunedQty));
		printRow(tableElement, row, 6, QString::number(balanceQty));
		printRow(tableElement, row, 7, QString::number(totalAmount, 'f', 2));
		printRow(tableElement, row, 8, QString::number(approximateProfit, 'f', 2));
		row++;
	}

	printRow(tableElement, row, 7, "Total Profit");
	printRow(tableElement, row, 8, QString::number(totalProfit, 'f', 2));

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
