#include "esreturnitems.h"
#include "QSqlQuery"
#include "utility\utility.h"
#include "QMessageBox"
#include "utility\esdbconnection.h"
#include "easylogging++.h"
#include "utility\session.h"
#include "KDReportsTextElement.h"
#include "KDReportsTableElement.h"
#include "KDReportsCell.h"
#include "KDReportsReport.h"
#include "KDReportsHtmlElement.h"
#include "QDateTime"
#include "QPrintPreviewDialog"
#include "utility\esmainwindowholder.h"
#include "QMainWindow"
#include "esmainwindow.h"
#include "entities\tabletextwidget.h"

QString convertToQuantityFormat(QString text, int row, int col, QTableWidget* table)
{
	// 		double val = text.toDouble();
	// 		return QString::number(val, 'f', 3);
	return text;
}

ESReturnItems::ESReturnItems(QWidget *parent /*= 0*/) : QWidget(parent), m_total(0), m_hasInterest(false)
{
	ui.setupUi(this);

	m_removeButtonSignalMapper = new QSignalMapper(this);
	m_idGenerator = 0;
	m_oldBillId = -1;

	QObject::connect(ui.selectBtn, SIGNAL(clicked()), this, SLOT(slotSelect()));
	//QObject::connect(ui.printBtn, SIGNAL(clicked()), this, SLOT(slotPrintReturnBill()));
	QObject::connect(ui.tableWidget, SIGNAL(cellClicked(int, int)), this, SLOT(slotItemDoubleClicked(int, int)));
	QObject::connect(m_removeButtonSignalMapper, SIGNAL(mapped(QString)), this, SLOT(slotRemove(QString)));
	QObject::connect(ui.interestText, SIGNAL(textChanged(QString)), this, SLOT(slotInterestChanged()));

	if (!ES::DbConnection::instance()->open())
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Critical);
		mbox.setText(QString("Cannot connect to the database : ESReturnItems"));
		mbox.exec();
	}

	QStringList headerLabels;
	headerLabels.append("Item Code");
	headerLabels.append("Item Name");
	headerLabels.append("Quantity");
	headerLabels.append("Billed Price");
	headerLabels.append("Paid Price");
	headerLabels.append("Return Price");
	headerLabels.append("Date");
	headerLabels.append("Actions");
	headerLabels.append("rowID");
	headerLabels.append("billQty");

	ui.tableWidget->setHorizontalHeaderLabels(headerLabels);
	ui.tableWidget->horizontalHeader()->setStretchLastSection(true);
	ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.tableWidget->hideColumn(8);
	ui.tableWidget->hideColumn(9);

	QStringList headerLabels2;
	headerLabels2.append("Code");
	headerLabels2.append("Item");
	headerLabels2.append("Price");
	headerLabels2.append("Qty");
	headerLabels2.append("Discount");
	headerLabels2.append("Sub Total");
	headerLabels2.append("Actions");
	headerLabels2.append("Sale_ID");

	ui.billTableWidget->setHorizontalHeaderLabels(headerLabels2);
	ui.billTableWidget->horizontalHeader()->setStretchLastSection(true);
	ui.billTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.billTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.billTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.billTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.billTableWidget->hideColumn(7);

	ui.billIdLbl->setText("N/A");
	ui.subTotalLbl->setText("0.00");

	ui.billIdSearchText->setFocus();
}

ESReturnItems::~ESReturnItems()
{

}

Ui::ReturnItems& ESReturnItems::getUI()
{
	return ui;
}

void ESReturnItems::slotPrint(QPrinter* printer)
{
	report.print(printer);
	this->close();
}

void ESReturnItems::slotPrintReturnBill()
{
	QString billedUser = "Billed Cashier : ";
	QSqlQuery q("SELECT display_name FROM user JOIN bill ON user.user_id = bill.user_id WHERE bill.bill_id = " + QString::number(m_oldBillId));
	if (q.next())
	{
		billedUser.append(q.value("display_name").toString());
	}
	QString titleStr = "Return Bill";
	KDReports::TextElement titleElement(titleStr);
	titleElement.setPointSize(14);
	titleElement.setBold(true);
	report.addElement(titleElement, Qt::AlignHCenter);

	QString billIdStr = "Bill ID : ";
	billIdStr.append(QString::number(m_oldBillId));
	KDReports::TextElement addressElement(billIdStr);
	addressElement.setPointSize(11);
	addressElement.setBold(false);
	report.addElement(addressElement, Qt::AlignLeft);

	QString dateStr = "Date : ";
	dateStr.append(QDateTime::currentDateTime().toString("yyyy-MM-dd"));
	QString timeStr = "Time : ";
	timeStr.append(QDateTime::currentDateTime().toString("hh : mm"));

	KDReports::TableElement infoTableElement;
	infoTableElement.setHeaderRowCount(2);
	infoTableElement.setHeaderColumnCount(2);
	infoTableElement.setBorder(0);
	infoTableElement.setWidth(100, KDReports::Percent);
	{
		KDReports::Cell& userNameCell = infoTableElement.cell(0, 0);
		KDReports::TextElement t("Cashier : " + ES::Session::getInstance()->getUser()->getName());
		t.setPointSize(10);
		userNameCell.addElement(t, Qt::AlignLeft);
	}{
		KDReports::Cell& userNameCell = infoTableElement.cell(1, 0);
		KDReports::TextElement t(billedUser);
		t.setPointSize(10);
		userNameCell.addElement(t, Qt::AlignLeft);
	}{
		KDReports::Cell& dateCell = infoTableElement.cell(0, 1);
		KDReports::TextElement t(dateStr);
		t.setPointSize(10);
		dateCell.addElement(t, Qt::AlignRight);
	}{
		KDReports::Cell& timeCell = infoTableElement.cell(1, 1);
		KDReports::TextElement t(timeStr);
		t.setPointSize(10);
		timeCell.addElement(t, Qt::AlignRight);
	}
	report.addElement(infoTableElement);

	//report.addVerticalSpacing(1);
	KDReports::HtmlElement htmlElem1;
	QString htm1("<div><hr/></div>");
	htmlElem1.setHtml(htm1);
	report.addElement(htmlElem1);
	//report.addVerticalSpacing(1);

	//////////////////////////////////////////////////////////////////////////
	KDReports::TableElement dataTableElement;
	dataTableElement.setHeaderRowCount(2);
	dataTableElement.setHeaderColumnCount(5);
	dataTableElement.setBorder(0);
	dataTableElement.setWidth(100, KDReports::Percent);
	double unitPrice, qty = 0;
	int row = 0;
	for (int i = 0; i < ui.tableWidget->rowCount(); ++i)
	{
		row = i;
		{
			KDReports::Cell& cell = dataTableElement.cell(i, 0);
			QString code = ui.tableWidget->item(i, 0)->text();
			KDReports::TextElement t(code);
			t.setPointSize(10);
			cell.addElement(t, Qt::AlignLeft);
		}{
			KDReports::Cell& cell = dataTableElement.cell(i, 1);
			QString name = ui.tableWidget->item(i, 1)->text();
			KDReports::TextElement t(name);
			t.setPointSize(10);
			cell.addElement(t, Qt::AlignLeft);
		}{
			QString qtyStr = ui.tableWidget->item(i, 2)->text();
			qty = qtyStr.toDouble();
			KDReports::Cell& cell = dataTableElement.cell(i, 2);
			KDReports::TextElement t(qtyStr);
			t.setPointSize(10);
			cell.addElement(t, Qt::AlignRight);
		}{
			KDReports::Cell& cell = dataTableElement.cell(i, 3);
			QString billedPrice = ui.tableWidget->item(i, 3)->text();
			KDReports::TextElement t(billedPrice);
			t.setPointSize(10);
			cell.addElement(t, Qt::AlignRight);
		}{
			QString paidPrice = ui.tableWidget->item(i, 4)->text();
			unitPrice = paidPrice.toDouble();
			double total = qty * unitPrice;
			KDReports::Cell& cell = dataTableElement.cell(i, 4);
			KDReports::TextElement t(QString::number(total, 'f', 2));
			t.setPointSize(10);
			cell.addElement(t, Qt::AlignRight);
		}
		m_total += unitPrice*qty;
	}

	float returnedTotal = 0;
	float billOutstanding = ES::Utility::getOutstandingForBill(m_oldBillId);

	row++; // sub total
	{
		KDReports::Cell& total = dataTableElement.cell(row, 3);
		//total.setColumnSpan(5);
		KDReports::TextElement totalTxt("Sub Total ");
		totalTxt.setPointSize(11);
		totalTxt.setBold(true);
		total.addElement(totalTxt, Qt::AlignRight);
	}
	{
		KDReports::Cell& total = dataTableElement.cell(row, 4);
		//total.setColumnSpan(5);
		QString totalStr = QString::number(m_total, 'f', 2);
		KDReports::TextElement totalValue(totalStr);
		totalValue.setPointSize(11);
		totalValue.setBold(true);
		total.addElement(totalValue, Qt::AlignRight);
	}
	{
		KDReports::Cell& total = dataTableElement.cell(row, 0);
		//total.setColumnSpan(5);
		KDReports::TextElement totalTxt("Bill Outstanding");
		totalTxt.setPointSize(11);
		totalTxt.setBold(false);
		total.addElement(totalTxt, Qt::AlignRight);
	}
	{
		returnedTotal = ui.totLbl->text().toFloat();
		KDReports::Cell& total = dataTableElement.cell(row, 1);
		//total.setColumnSpan(5);
		KDReports::TextElement totalValue(QString::number(billOutstanding, 'f', 2));
		totalValue.setPointSize(11);
		totalValue.setBold(false);
		total.addElement(totalValue, Qt::AlignRight);
	}
	row++; // interest
	{
		KDReports::Cell& total = dataTableElement.cell(row, 3);
		//total.setColumnSpan(5);
		KDReports::TextElement totalTxt("Interest ");
		totalTxt.setPointSize(11);
		totalTxt.setBold(true);
		total.addElement(totalTxt, Qt::AlignRight);
	}
	{
		KDReports::Cell& total = dataTableElement.cell(row, 4);
		//total.setColumnSpan(5);
		double interest = ui.interestText->text().toDouble();		
		KDReports::TextElement totalValue(QString::number(interest, 'f', 2));
		totalValue.setPointSize(11);
		totalValue.setBold(true);
		total.addElement(totalValue, Qt::AlignRight);
	}
	{
		KDReports::Cell& total = dataTableElement.cell(row, 0);
		//total.setColumnSpan(5);
		KDReports::TextElement totalTxt("Return Total");
		totalTxt.setPointSize(11);
		totalTxt.setBold(false);
		total.addElement(totalTxt, Qt::AlignRight);
	}
	{
		returnedTotal = ui.totLbl->text().toFloat();
		KDReports::Cell& total = dataTableElement.cell(row, 1);
		//total.setColumnSpan(5);
		KDReports::TextElement totalValue("-" + QString::number(returnedTotal, 'f', 2));
		totalValue.setPointSize(11);
		totalValue.setBold(false);
		total.addElement(totalValue, Qt::AlignRight);
	}
	row++; // return total
	{
		KDReports::Cell& total = dataTableElement.cell(row, 3);
		//total.setColumnSpan(5);
		KDReports::TextElement totalTxt("Return Total");
		totalTxt.setPointSize(11);
		totalTxt.setBold(true);
		total.addElement(totalTxt, Qt::AlignRight);
	}
	{
		returnedTotal  = ui.totLbl->text().toFloat();
		KDReports::Cell& total = dataTableElement.cell(row, 4);
		//total.setColumnSpan(5);
		KDReports::TextElement totalValue(ui.totLbl->text());
		totalValue.setPointSize(11);
		totalValue.setBold(true);
		total.addElement(totalValue, Qt::AlignRight);
	}	
	{
		KDReports::Cell& total = dataTableElement.cell(row, 0);
		//total.setColumnSpan(5);
		KDReports::TextElement totalTxt("Current Due");
		totalTxt.setPointSize(11);
		totalTxt.setBold(false);
		total.addElement(totalTxt, Qt::AlignRight);
	}
	{
		returnedTotal = ui.totLbl->text().toFloat();
		KDReports::Cell& total = dataTableElement.cell(row, 1);
		//total.setColumnSpan(5);
		KDReports::TextElement totalValue(QString::number(billOutstanding - returnedTotal, 'f', 2));
		totalValue.setPointSize(11);
		totalValue.setBold(false);
		total.addElement(totalValue, Qt::AlignRight);
	}
	report.addElement(dataTableElement);
	//report.addElement(htmlElem1);
	report.addVerticalSpacing(1);
	//////////////////////////////////////////////////////////////////////////

	// Update database
	int uId = ES::Session::getInstance()->getUser()->getId();
	QString itemId = "";
	float returnQty = 0;
	for (int i = 0; i < ui.tableWidget->rowCount(); ++i)
	{
		QTableWidgetItem* item = ui.tableWidget->item(i, 0);
		QSqlQuery itemQuery("SELECT item_id FROM item WHERE item_code = '" + item->text() + "'");
		if (itemQuery.next())
		{
			returnQty = ui.tableWidget->item(i, 2)->text().toFloat();
			float paidPrice = ui.tableWidget->item(i, 4)->text().toFloat();
			float interest = ui.interestText->text().toFloat();
			itemId = itemQuery.value("item_id").toString();
			double total = returnQty * paidPrice * (100 + interest) / 100;
			// bill_id, item_id, qty, paid_price, return_total, user_id
			QSqlQuery q("INSERT INTO return_item (bill_id, item_id, qty, paid_price, return_total, user_id) VALUES (" +	
				QString::number(m_oldBillId) + "," +
				itemQuery.value("item_id").toString() + "," +
				QString::number(returnQty) + "," +
				ui.tableWidget->item(i, 4)->text() + "," +
				QString::number(total) + "," +
				QString::number(uId) + ")"
				);

			//inserting to stock

			QSqlQuery qStock("SELECT * FROM stock WHERE item_id = "+itemId);
			if (qStock.next())
			{
				float currentQty = qStock.value("qty").toFloat();
				float newQty = currentQty + returnQty;
				QString stockId = qStock.value("stock_id").toString();
				QSqlQuery qStockUpdate("UPDATE stock SET qty = "+QString::number(newQty)+" WHERE stock_id = "+stockId);
			}
		}
	}

	QPrinter printer;
	printer.setPaperSize(QPrinter::A4);

	printer.setFullPage(false);
	printer.setOrientation(QPrinter::Portrait);

	QPrintPreviewDialog *dialog = new QPrintPreviewDialog(&printer, this);
	QObject::connect(dialog, SIGNAL(paintRequested(QPrinter*)), this, SLOT(slotPrint(QPrinter*)));
	dialog->setWindowTitle(tr("Print Document"));
	ES::MainWindowHolder::instance()->getMainWindow()->setCentralWidget(dialog);
	dialog->exec();

	//report.print(&printer);
}

void ESReturnItems::slotSelect()
{
	QString itemCode = ui.itemCodeSearchText->text();
	QString billId = ui.billIdSearchText->text();

	m_bill.addReturnItem(billId, itemCode);
	updateReturnItemTable();

	showTotal();
}

void ESReturnItems::slotItemDoubleClicked(int row, int col)
{
	if (col == 2) /* Quantity */
	{
		QString quantity = "";
		QTableWidgetItem* item = ui.tableWidget->item(row, 2);
		if (item)
		{
			quantity = item->text();
		}

		TableTextWidget* textWidget = new TableTextWidget(ui.tableWidget, row, 2, ui.tableWidget);
		QObject::connect(textWidget, SIGNAL(notifyEnterPressed(QString, int, int)), this, SLOT(slotQuantityCellUpdated(QString, int, int)));
		textWidget->setTextFormatterFunc(convertToQuantityFormat);
		textWidget->setText(quantity);
		textWidget->selectAll();
		ui.tableWidget->setCellWidget(row, 2, textWidget);
		textWidget->setFocus();
	}
}

void ESReturnItems::slotRemove(QString rowId)
{
	m_bill.removeReturnItem(rowId);
	updateReturnItemTable();
	showTotal();
}

void ESReturnItems::slotQuantityCellUpdated(QString qtyStr, int row, int col)
{
	QTableWidgetItem* billQtyItem = ui.tableWidget->item(row, 9);
	double billQty = billQtyItem->text().toDouble();
	double qty = qtyStr.toDouble();

	if (billQty < qty || qty <= 0.0)
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Critical);
		mbox.setText(QString("Quantity is larger than billed quantity"));
		mbox.exec();

		// reset & return
		QTableWidgetItem* retPriceItem = new QTableWidgetItem(QString::number(billQty));
		retPriceItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
		ui.tableWidget->setItem(row, col, retPriceItem);
		return;
	}

	QTableWidgetItem* paidPriceItem = ui.tableWidget->item(row, 4);
	double paidPrice = paidPriceItem->text().toDouble();

	double returnPrice = paidPrice * qty;
	QTableWidgetItem* retPriceItem = new QTableWidgetItem(QString::number(returnPrice, 'f', 2));
	retPriceItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
	ui.tableWidget->setItem(row, 5, retPriceItem);

	showTotal();
}

void ESReturnItems::showTotal()
{
	ui.subTotalLbl->setText(QString::number(m_bill.getSubTotal(), 'f', 2));
	ui.totLbl->setText(QString::number(m_bill.getTotal(), 'f', 2));
}

void ESReturnItems::slotInterestChanged()
{
	m_bill.setInterest(ui.interestText->text());
}

void ESReturnItems::updateReturnItemTable()
{
	while (ui.tableWidget->rowCount() > 0)
	{
		ui.tableWidget->removeRow(0);
	}
	const std::map<int, QStringList>& returnItems = m_bill.getReturnItemTable();
	for (std::map<int, QStringList>::const_iterator it = returnItems.begin(), ite = returnItems.end(); it != ite; ++it)
	{
		int row = ui.tableWidget->rowCount();
		ui.tableWidget->insertRow(row);

		const QStringList& sl = it->second;

		ui.tableWidget->setItem(row, 0, new QTableWidgetItem(sl[0]));
		ui.tableWidget->setItem(row, 1, new QTableWidgetItem(sl[1]));

		QTableWidgetItem* qtyItem = new QTableWidgetItem(sl[2]);
		qtyItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
		ui.tableWidget->setItem(row, 2, qtyItem);

		QTableWidgetItem* itemPriceItem = new QTableWidgetItem(sl[3]);
		itemPriceItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
		ui.tableWidget->setItem(row, 3, itemPriceItem);

		QTableWidgetItem* paidPriceItem = new QTableWidgetItem(sl[4]);
		paidPriceItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
		ui.tableWidget->setItem(row, 4, paidPriceItem);

		QTableWidgetItem* retPriceItem = new QTableWidgetItem(sl[5]);
		retPriceItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
		ui.tableWidget->setItem(row, 5, retPriceItem);

		QTableWidgetItem* dateItem = new QTableWidgetItem(sl[6]);
		dateItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
		ui.tableWidget->setItem(row, 6, dateItem);
		// 7 - add/remove buttons
		QWidget* base = new QWidget(ui.tableWidget);

		QPushButton* removeBtn = new QPushButton(base);
		removeBtn->setIcon(QIcon("icons/delete.png"));
		removeBtn->setIconSize(QSize(24, 24));
		removeBtn->setMaximumWidth(100);
		
		m_removeButtonSignalMapper->setMapping(removeBtn, QString::number(it->first));
		QObject::connect(removeBtn, SIGNAL(clicked()), m_removeButtonSignalMapper, SLOT(map()));

		QHBoxLayout *layout = new QHBoxLayout;
		layout->setContentsMargins(0, 0, 0, 0);
		layout->addWidget(removeBtn);
		layout->insertStretch(2);
		base->setLayout(layout);
		ui.tableWidget->setCellWidget(row, 7, base);
		base->show();

		ui.tableWidget->setItem(row, 8, new QTableWidgetItem(QString::number(it->first)));
		ui.tableWidget->setItem(row, 9, new QTableWidgetItem(sl[7]));
	}
}
