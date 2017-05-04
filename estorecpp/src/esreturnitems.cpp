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
#include "QApplication"
#include "QDesktopWidget"
#include "esaddbillitem2.h"
#include "QShortcut"
#include "essinglepayment2.h"

QString convertToQuantityFormat(QString text, int row, int col, QTableWidget* table)
{
	// 		double val = text.toDouble();
	// 		return QString::number(val, 'f', 3);
	return text;
}

ESReturnItems::ESReturnItems(QWidget *parent /*= 0*/) : QWidget(parent), m_hasInterest(false)
{
	ui.setupUi(this);

	m_removeButtonSignalMapper = new QSignal Mapper(this);
	m_removeNewItemButtonSignalMapper = new QSignalMapper(this);
	m_idGenerator = 0;
	m_oldBillId = -1;

	QObject::connect(ui.selectBtn, SIGNAL(clicked()), this, SLOT(slotSelect()));
	//QObject::connect(ui.printBtn, SIGNAL(clicked()), this, SLOT(slotPrintReturnBill()));
	QObject::connect(ui.tableWidget, SIGNAL(cellClicked(int, int)), this, SLOT(slotItemDoubleClicked(int, int)));
	QObject::connect(m_removeButtonSignalMapper, SIGNAL(mapped(QString)), this, SLOT(slotRemove(QString)));
	QObject::connect(m_removeNewItemButtonSignalMapper, SIGNAL(mapped(int)), this, SLOT(slotRemoveNewItem(int)));
	QObject::connect(ui.returnInterest, SIGNAL(textChanged(QString)), this, SLOT(slotInterestChanged()));
	QObject::connect(ui.newInterest, SIGNAL(textChanged(QString)), this, SLOT(slotNewInterestChanged()));
	QObject::connect(ui.addItemBtn, SIGNAL(clicked()), this, SLOT(slotShowAddItem()));
	QObject::connect(ui.startBillBtn, SIGNAL(clicked()), this, SLOT(slotStartBill()));
	QObject::connect(ui.commitBtn, SIGNAL(clicked()), this, SLOT(slotCommit()));
	QObject::connect(ui.billTableWidget, SIGNAL(cellClicked(int, int)), this, SLOT(slotItemDoubleClickedOnNewItems(int, int)));

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
	//ui.subTotalLbl->setText("0.00");

	new QShortcut(QKeySequence(Qt::Key_F4), this, SLOT(slotShowAddItem()));
	new QShortcut(QKeySequence(Qt::Key_F3), this, SLOT(slotStartBill()));

	ui.commitBtn->setDisabled(true);
	ui.label_10->setHidden(true);
	//ui.billIdSearchText->setFocus();

	setEnabled(false);
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
	QString qStrUserQry("SELECT display_name FROM user JOIN bill ON user.user_id = bill.user_id WHERE bill.bill_id = " + QString::number(m_bill.getOldBillId()));
	QSqlQuery q(qStrUserQry);
	if (q.next())
	{
		billedUser.append(q.value("display_name").toString());
	}
	QString titleStr = "Return Bill";
	KDReports::TextElement titleElement(titleStr);
	titleElement.setPointSize(14);
	titleElement.setBold(true);
	report.addElement(titleElement, Qt::AlignHCenter);

	QString dateStr = "Date : ";
	dateStr.append(QDateTime::currentDateTime().toString("yyyy-MM-dd"));
	QString timeStr = "Time : ";
	timeStr.append(QDateTime::currentDateTime().toString("hh : mm"));

	KDReports::TableElement infoTableElement;
	infoTableElement.setHeaderRowCount(3);
	infoTableElement.setHeaderColumnCount(2);
	infoTableElement.setBorder(0);
	infoTableElement.setWidth(100, KDReports::Percent);
	{
		KDReports::Cell& oldBillNoCell = infoTableElement.cell(0, 0);
		KDReports::TextElement t("Orig.Bill No : " + m_bill.getOldBillId());
		t.setPointSize(10);
		oldBillNoCell.addElement(t, Qt::AlignLeft);
	}{
		KDReports::Cell& userNameCell = infoTableElement.cell(1, 0);
		KDReports::TextElement t("Cashier : " + ES::Session::getInstance()->getUser()->getName());
		t.setPointSize(10);
		userNameCell.addElement(t, Qt::AlignLeft);
	}{
		KDReports::Cell& userNameCell = infoTableElement.cell(2, 0);
		KDReports::TextElement t(billedUser);
		t.setPointSize(10);
		userNameCell.addElement(t, Qt::AlignLeft);
	}{
		KDReports::Cell& returnBillNoCell = infoTableElement.cell(0, 1);
		KDReports::TextElement t("Bill No : "+ m_bill.getBillId());
		t.setPointSize(10);
		returnBillNoCell.addElement(t, Qt::AlignRight);
	}{
		KDReports::Cell& dateCell = infoTableElement.cell(1, 1);
		KDReports::TextElement t(dateStr);
		t.setPointSize(10);
		dateCell.addElement(t, Qt::AlignRight);
	}{
		KDReports::Cell& timeCell = infoTableElement.cell(2, 1);
		KDReports::TextElement t(timeStr);
		t.setPointSize(10);
		timeCell.addElement(t, Qt::AlignRight);
	}
	
	report.addElement(infoTableElement);

	KDReports::TextElement retItemsElem("Return Items");
	retItemsElem.setPointSize(11);
	retItemsElem.setBold(false);
	report.addElement(retItemsElem, Qt::AlignLeft);

	report.addVerticalSpacing(1);
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
	double unitPrice = 0, qty = 0, retTotal= 0;
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
		
		retTotal += unitPrice*qty;
	}

	float returnedTotal = 0;

	row++; // sub total
	{
		KDReports::Cell& total = dataTableElement.cell(row, 3);
		//total.setColumnSpan(5);
		KDReports::TextElement totalTxt("Sub Total (RTN) : ");
		totalTxt.setPointSize(11);
		totalTxt.setBold(true);
		total.addElement(totalTxt, Qt::AlignRight);
	}
	{
		KDReports::Cell& total = dataTableElement.cell(row, 4);
		//total.setColumnSpan(5);
		QString totalStr = QString::number(retTotal, 'f', 2);
		KDReports::TextElement totalValue(totalStr);
		totalValue.setPointSize(11);
		totalValue.setBold(true);
		total.addElement(totalValue, Qt::AlignRight);
	}

	row++; // interest
	{
		KDReports::Cell& total = dataTableElement.cell(row, 3);
		//total.setColumnSpan(5);
		KDReports::TextElement totalTxt("Interest : ");
		totalTxt.setPointSize(11);
		totalTxt.setBold(true);
		total.addElement(totalTxt, Qt::AlignRight);
	}
	{
		KDReports::Cell& total = dataTableElement.cell(row, 4);
		//total.setColumnSpan(5);
		double interest = ui.returnInterest->text().toDouble();		
		KDReports::TextElement totalValue(QString::number(interest, 'f', 2));
		totalValue.setPointSize(11);
		totalValue.setBold(true);
		total.addElement(totalValue, Qt::AlignRight);
	}

	report.addElement(dataTableElement);
	report.addVerticalSpacing(2);

	KDReports::TextElement newItemsElem("New Items");
	newItemsElem.setPointSize(11);
	newItemsElem.setBold(false);
	report.addElement(newItemsElem, Qt::AlignLeft);
	report.addElement(htmlElem1);

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
			float interest = ui.returnInterest->text().toFloat();
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

	if (m_bill.addReturnItem(billId, itemCode))
	{
		updateReturnItemTable();
		showTotal();
	}
	else
	{
		QString oldBill;
		oldBill.setNum(m_bill.getOldBillId());
				
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Critical);
		mbox.setText(QString("Invalid bill id. Current Bill is : " + oldBill));
		mbox.exec();
		ui.billIdSearchText->setText("");
	}
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
	QTableWidgetItem* rowIdItem = ui.tableWidget->item(row, 8);
	long rowId = rowIdItem->text().toLong();
	
	if (m_bill.isStarted())
	{
		double billedQty = 0;
		double returnPrice = 0;
		if (!m_bill.updateItemQuantity(rowId, qtyStr, billedQty,returnPrice))
		{
			QMessageBox mbox;
			mbox.setIcon(QMessageBox::Critical);
			mbox.setText(QString("Quantity is larger than billed quantity or invalid quantity"));
			mbox.exec();

			// reset & return
			QTableWidgetItem* retPriceItem = new QTableWidgetItem(QString::number(billedQty));
			retPriceItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
			ui.tableWidget->setItem(row, col, retPriceItem);

			return;
		}

		QTableWidgetItem* retPriceItem = new QTableWidgetItem(QString::number(returnPrice, 'f', 2));
		retPriceItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
		ui.tableWidget->setItem(row, 5, retPriceItem);
	}

	showTotal();
}

void ESReturnItems::slotNewItemQuantityCellUpdated(QString txt, int row, int col)
{
	QTableWidgetItem* item = ui.billTableWidget->item(row, 7);
	if (item)
	{
		QString rowIdStr = item->text();
		std::string s = rowIdStr.toStdString();
		long rowId = rowIdStr.toLong();
		bool success = m_bill.updateNewItemQuantity(rowId, txt);
			const std::map<int, ES::ReturnBill::NewItemInfo>& newItems = m_bill.getNewItemTable();
			std::map<int, ES::ReturnBill::NewItemInfo>::const_iterator iter = newItems.find(rowId);
			if (iter != newItems.end())
			{
				const ES::ReturnBill::NewItemInfo& ni = iter->second;

				double subtotal = ni.itemPrice * ni.quantity;
				QTableWidgetItem* subtotalItem = new QTableWidgetItem(QString::number(subtotal, 'f', 2));
				subtotalItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
				ui.billTableWidget->setItem(row, 5, subtotalItem);
				//if (!success)
				{
					//QSqlQuery query("SELECT qty FROM stock WHERE stock_id = " + QString::number(ni.stockId));
					//if (query.first())
					{
						//QString currentQty = query.value("qty").toString();
						QTableWidgetItem* qtyItem = new QTableWidgetItem(QString::number(ni.quantity));
						qtyItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
						ui.billTableWidget->setItem(row, 3, qtyItem);
					}
				}
			}
	}
	
	showTotal();
}

void ESReturnItems::showTotal()
{
	ui.returnSubTotal->setText(QString::number(m_bill.getSubTotal(), 'f', 2));
	ui.returnTotal->setText(QString::number(m_bill.getTotal(), 'f', 2));

	ui.newSubTotal->setText(QString::number(m_bill.getNewSubTotal(), 'f', 2));
	ui.newTotal->setText(QString::number(m_bill.getNewTotal(), 'f', 2));
// 	if (m_bill.getNewTotal() >= m_bill.getTotal())
// 	{
// 		ui.commitBtn->setEnabled(true);
// 	}
// 	else
// 	{
// 		ui.commitBtn->setDisabled(true);
// 	}
}

void ESReturnItems::slotInterestChanged()
{
	m_bill.setInterest(ui.returnInterest->text());
	showTotal();
}

void ESReturnItems::updateReturnItemTable()
{
	while (ui.tableWidget->rowCount() > 0)
	{
		ui.tableWidget->removeRow(0);
	}
	const std::map<int, ES::ReturnBill::ReturnItemInfo>& returnItems = m_bill.getReturnItemTable();
	for (std::map<int, ES::ReturnBill::ReturnItemInfo>::const_iterator it = returnItems.begin(), ite = returnItems.end(); it != ite; ++it)
	{
		int row = ui.tableWidget->rowCount();
		ui.tableWidget->insertRow(row);

		const ES::ReturnBill::ReturnItemInfo& bi = it->second;

		ui.tableWidget->setItem(row, 0, new QTableWidgetItem(bi.itemCode));
		ui.tableWidget->setItem(row, 1, new QTableWidgetItem(bi.itemName));

		QTableWidgetItem* qtyItem = new QTableWidgetItem(QString::number(bi.quantity));
		qtyItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
		ui.tableWidget->setItem(row, 2, qtyItem);

		QTableWidgetItem* itemPriceItem = new QTableWidgetItem(QString::number(bi.itemPrice, 'f', 2));
		itemPriceItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
		ui.tableWidget->setItem(row, 3, itemPriceItem);

		QTableWidgetItem* paidPriceItem = new QTableWidgetItem(QString::number(bi.paidPrice, 'f', 2));
		paidPriceItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
		ui.tableWidget->setItem(row, 4, paidPriceItem);

		QTableWidgetItem* retPriceItem = new QTableWidgetItem(QString::number(bi.returnPrice, 'f', 2));
		retPriceItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
		ui.tableWidget->setItem(row, 5, retPriceItem);

		QTableWidgetItem* dateItem = new QTableWidgetItem(bi.date);
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
	}
}

void ESReturnItems::slotShowAddItem()
{
	QRect rec = QApplication::desktop()->screenGeometry();
	int width = rec.width();
	int height = rec.height();

	width -= 200;
	height -= 200;

	ESAddBillItem2* addBillItem = new ESAddBillItem2(m_bill, this);
	addBillItem->resize(QSize(width, height));
	addBillItem->setWindowState(Qt::WindowActive);
	addBillItem->setWindowModality(Qt::ApplicationModal);
	addBillItem->setAttribute(Qt::WA_DeleteOnClose);
	addBillItem->setWindowFlags(Qt::CustomizeWindowHint | Qt::Window);
	addBillItem->show();
	addBillItem->setFocus();
	addBillItem->focus();
}

void ESReturnItems::setEnabled(bool enable)
{
	ui.billIdSearchText->setEnabled(enable);
	ui.itemCodeSearchText->setEnabled(enable);
	ui.selectBtn->setEnabled(enable);
	ui.tableWidget->setEnabled(enable);
	ui.billTableWidget->setEnabled(enable);
	ui.returnInterest->setEnabled(enable);
	ui.newInterest->setEnabled(enable);
	//ui.startBillBtn->setEnabled(enable);
	ui.addItemBtn->setEnabled(enable);
	ui.commitBtn->setEnabled(enable);
	ui.cancelBtn->setEnabled(enable);

	if (enable)
	{
		ui.billIdSearchText->setFocus();
	}
}

void ESReturnItems::slotStartBill()
{
	if (m_bill.isStarted())
		return;

	if (m_bill.start())
	{
		QString billid;
		billid.setNum(m_bill.getBillId());
		ui.billIdLbl->setText(billid);

		setEnabled(true);
	}
	else
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Critical);
		mbox.setText(QString("Bill cannot be started"));
		mbox.exec();
	}
}

void ESReturnItems::updateNewItemTable()
{
	while (ui.billTableWidget->rowCount() > 0)
	{
		ui.billTableWidget->removeRow(0);
	}
	int row = ui.billTableWidget->rowCount();

	const std::map<int, ES::ReturnBill::NewItemInfo>& newItems = m_bill.getNewItemTable();
	for (std::map<int, ES::ReturnBill::NewItemInfo>::const_iterator it = newItems.begin(), ite = newItems.end(); it != ite; ++it)
	{
		row = ui.billTableWidget->rowCount();
		ui.billTableWidget->insertRow(row);

		const ES::ReturnBill::NewItemInfo& ni = it->second;

		QString ss;
		ss.setNum(ni.itemPrice);
		ni.itemCode + " : " + ss;

		ui.billTableWidget->setItem(row, 0, new QTableWidgetItem(ni.itemCode));
		ui.billTableWidget->setItem(row, 1, new QTableWidgetItem(ni.itemName));

		QTableWidgetItem* priceItem = new QTableWidgetItem(QString::number(ni.itemPrice, 'f', 2));
		priceItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
		ui.billTableWidget->setItem(row, 2, priceItem);
		
		QTableWidgetItem* qtyItem = new QTableWidgetItem(QString::number(ni.quantity));
		qtyItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
		ui.billTableWidget->setItem(row, 3, qtyItem);

		QTableWidgetItem* discountItem = new QTableWidgetItem(QString::number(ni.discount, 'f', 2));
		discountItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
		ui.billTableWidget->setItem(row, 4, discountItem);

		double total = ni.itemPrice * ni.quantity;
		double subtotal = total + total * ni.discount * 0.01;
		QTableWidgetItem* subtotalItem = new QTableWidgetItem(QString::number(subtotal, 'f', 2));
		subtotalItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
		ui.billTableWidget->setItem(row, 5, subtotalItem);

		QWidget* base = new QWidget(ui.billTableWidget);
		QPushButton* removeBtn = new QPushButton(base);
		removeBtn->setIcon(QIcon("icons/delete.png"));
		removeBtn->setIconSize(QSize(24, 24));
		removeBtn->setMaximumWidth(100);

		QObject::connect(removeBtn, SIGNAL(clicked()), m_removeNewItemButtonSignalMapper, SLOT(map()));
		m_removeNewItemButtonSignalMapper->setMapping(removeBtn, it->first);

		QHBoxLayout *layout = new QHBoxLayout;
		layout->setContentsMargins(0, 0, 0, 0);
		layout->addWidget(removeBtn);
		layout->insertStretch(2);
		base->setLayout(layout);
		ui.billTableWidget->setCellWidget(row, 6, base);
		base->show();

		ui.billTableWidget->setItem(row, 7, new QTableWidgetItem(QString::number(it->first)));
	}
}

void ESReturnItems::keyPressEvent(QKeyEvent* evt)
{
	switch (evt->key())
	{
	case Qt::Key_Return:
	case Qt::Key_Enter:
	{
						  int row = ui.billTableWidget->currentRow();
						  QString quantity = "";
						  QTableWidgetItem* item = ui.billTableWidget->item(row, 3);
						  if (item)
						  {
							  quantity = item->text();
						  }

						  TableTextWidget* textWidget = new TableTextWidget(ui.billTableWidget, row, 3, ui.billTableWidget);
						  QObject::connect(textWidget, SIGNAL(notifyEnterPressed(QString, int, int)), this, SLOT(slotNewItemQuantityCellUpdated(QString, int, int)));
						  textWidget->setTextFormatterFunc(convertToQuantityFormat);
						  textWidget->setText(quantity);
						  textWidget->selectAll();
						  ui.billTableWidget->setCellWidget(row, 3, textWidget);
						  textWidget->setFocus();
						  break;
	}
	case Qt::Key_Up:
	case Qt::Key_Down:
	case Qt::Key_Left:
	case Qt::Key_Right:
	{
						  ui.tableWidget->setFocus();
						  break;
	}
	default:
		QWidget::keyPressEvent(evt);
	}
	QWidget::keyPressEvent(evt);
}

void ESReturnItems::slotRemoveNewItem(int id)
{
	m_bill.removeNewItem(id);
	updateNewItemTable();
	showTotal();
}

void ESReturnItems::slotCommit()
{
	ESSinglePayment2* singlePayment = new ESSinglePayment2(this, 0);
	singlePayment->setWindowState(Qt::WindowActive);
	singlePayment->setWindowModality(Qt::ApplicationModal);
	singlePayment->setAttribute(Qt::WA_DeleteOnClose);

	QSqlQuery queryBill("SELECT * FROM bill WHERE bill_id = " + QString::number(m_bill.getOldBillId()));
	if (queryBill.next())
	{
		m_customerId = queryBill.value("customer_id").toString();
		QSqlQuery queryCustomer("SELECT * FROM customer WHERE deleted = 0 AND customer_id = "+m_customerId);
		if (queryCustomer.next())
		{
			singlePayment->getUI().nameText->setText(queryCustomer.value("name").toString());
			singlePayment->getUI().addressText->setText(queryCustomer.value("address").toString());
			singlePayment->getUI().commentsText->setText(queryCustomer.value("comments").toString());
		}
		singlePayment->setCustomerId(m_customerId);
	}
	float outstanding = ES::Utility::getTotalCreditOutstanding(m_customerId);
	singlePayment->getUI().outstandingText->setText(QString::number(outstanding, 'f', 2));
	//singlePayment->setCustomerId(m_customerId);

	////outstanding start
	//float totalAmount = 0;
	//int customerId = m_customerId.toInt();
	//if (customerId > -1)
	//{
	//	totalAmount = ES::Utility::getTotalCreditOutstanding(m_customerId);
	//}

	//singlePayment->getUI().nameText->setText(m_name);
	//singlePayment->getUI().outstandingText->setText(QString::number(totalAmount, 'f', 2));
	//singlePayment->getUI().addressText->setText(m_address);
	//singlePayment->getUI().commentsText->setText(m_comments);

	//singlePayment->getUI().netAmountLbl->setText(m_netAmount);
	//singlePayment->getUI().totalBillLbl->setText(m_totalAmount);
	//singlePayment->setInitialNetAmount(m_netAmount.toFloat());
	//singlePayment->getUI().noOfItemsLbl->setText(m_noOfItems);
	//singlePayment->getUI().balanceLbl->setText(QString::number(totalAmount, 'f', 2));

	singlePayment->show();

	//slotPrintReturnBill();
}

void ESReturnItems::slotNewInterestChanged()
{
	m_bill.setNewInterest(ui.newInterest->text());
	showTotal();
}

void ESReturnItems::updateDatabase()
{
	int uId = ES::Session::getInstance()->getUser()->getId();
	QString itemId = "";
	float returnQty = 0;

	// update return_bill table
	QString returnInterest = "0", newInterest = "0";
	bool ok = false;
	ui.returnInterest->text().toFloat(&ok);
	if (ok)
	{
		returnInterest = ui.returnInterest->text();
	}
	ui.newInterest->text().toFloat(&ok);
	if (ok)
	{
		newInterest = ui.newInterest->text();
	}
	QSqlQuery rbQuery("INSERT INTO return_bill (return_bill_id, bill_id, ts, return_interest, new_interest) VALUES (" + 
		QString::number(m_bill.getBillId()) + "," + 
		QString::number(m_bill.getOldBillId()) + ", NOW() , " + returnInterest + " , " + newInterest + ")");

	// process return items
	for (int i = 0; i < ui.tableWidget->rowCount(); ++i)
	{
		QTableWidgetItem* item = ui.tableWidget->item(i, 0);
		QSqlQuery itemQuery("SELECT item_id FROM item WHERE item_code = '" + item->text() + "'");
		if (itemQuery.next())
		{
			returnQty = ui.tableWidget->item(i, 2)->text().toFloat();
			float paidPrice = ui.tableWidget->item(i, 4)->text().toFloat();
			float interest = ui.returnInterest->text().toFloat();
			itemId = itemQuery.value("item_id").toString();
			double total = returnQty * paidPrice * (100 + interest) / 100;
			// bill_id, item_id, qty, paid_price, return_total, user_id
			QSqlQuery q("INSERT INTO return_item (bill_id, new_bill_id, item_id, qty, paid_price, return_total, user_id) VALUES (" +
				QString::number(m_bill.getOldBillId()) + "," +
				QString::number(m_bill.getBillId()) + "," +
				itemQuery.value("item_id").toString() + "," +
				QString::number(returnQty) + "," +
				ui.tableWidget->item(i, 4)->text() + "," +
				QString::number(total) + "," +
				QString::number(uId) + ")"
				);

			//inserting to stock
			QSqlQuery qStock("SELECT * FROM stock WHERE item_id = " + itemId);
			if (qStock.next())
			{
				float currentQty = qStock.value("qty").toFloat();
				float newQty = currentQty + returnQty;
				QString stockId = qStock.value("stock_id").toString();
				QSqlQuery qStockUpdate("UPDATE stock SET qty = " + QString::number(newQty) + " WHERE stock_id = " + stockId);
			}
		}
	}

	// process new items
	const std::map<int, ES::ReturnBill::NewItemInfo>& newItems = m_bill.getNewItemTable();
	std::map<int, ES::ReturnBill::NewItemInfo>::const_iterator iter = newItems.begin();
	std::map<int, ES::ReturnBill::NewItemInfo>::const_iterator iterEnd = newItems.end();
	for (; iter != iterEnd; ++iter)
	{
		const ES::ReturnBill::NewItemInfo& ni = iter->second;

		double price = ni.itemPrice - ni.itemPrice * ni.discount * 0.01;
		double netTotal = price * ni.quantity;
		// stock_id 	bill_id 	quantity 	discount 	item_price 	total 	deleted 	date 
		QString q = "INSERT INTO sale (stock_id, bill_id, quantity, discount, item_price, w_cost, total) VALUES(" +
			QString::number(ni.stockId) + ", " +
			QString::number(m_bill.getBillId()) + ", " +
			QString::number(ni.quantity) + ", " +
			QString::number(ni.discount, 'f', 2) + ", " +
			QString::number(ni.itemPrice, 'f', 2) + ", " +
			QString::number(ni.wCost, 'f', 2) + ", " +
			QString::number(netTotal, 'f', 2) + ")";
		QSqlQuery query;
		if (query.exec(q))
		{
			QSqlQuery qStock("SELECT * FROM stock WHERE stock_id = " + QString::number(ni.stockId));
			if (qStock.next())
			{
				float currentQty = qStock.value("qty").toFloat();
				float newQty = currentQty - ni.quantity;
				QSqlQuery qStockUpdate("UPDATE stock SET qty = " + QString::number(newQty) + " WHERE stock_id = " + QString::number(ni.stockId));
			}
		}
	}
}

void ESReturnItems::finishBill()
{
	m_bill.end();
}

void ESReturnItems::slotNewPriceCellUpdated(QString price, int row, int col)
{
	QTableWidgetItem* item = ui.billTableWidget->item(row, col);
	if (item)
	{
		QString rowIdStr = item->text();
		std::string s = rowIdStr.toStdString();
		long rowId = rowIdStr.toLong();
		bool success = m_bill.updateNewItemPrice(rowId, price);
		const std::map<int, ES::ReturnBill::NewItemInfo>& newItems = m_bill.getNewItemTable();
		std::map<int, ES::ReturnBill::NewItemInfo>::const_iterator iter = newItems.find(rowId);
		if (iter != newItems.end())
		{
			const ES::ReturnBill::NewItemInfo& ni = iter->second;

			double subtotal = ni.itemPrice * ni.quantity;
			QTableWidgetItem* subtotalItem = new QTableWidgetItem(QString::number(subtotal, 'f', 2));
			subtotalItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
			ui.billTableWidget->setItem(row, 5, subtotalItem);
			
			QTableWidgetItem* qtyItem = new QTableWidgetItem(QString::number(ni.quantity));
			qtyItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
			ui.billTableWidget->setItem(row, 3, qtyItem);	
		}
	}

	showTotal();
}

void ESReturnItems::slotItemDoubleClickedOnNewItems(int row, int col)
{
	if (col == 2) /* Price */
	{
		QString price = "";
		QTableWidgetItem* item = ui.billTableWidget->item(row, 2);
		if (item)
		{
			bool valid = true;
			item->text().toDouble(&valid);
			if (!valid)
			{
				return;
			}
			price = item->text();
		}

		TableTextWidget* textWidget = new TableTextWidget(ui.billTableWidget, row, 2, ui.billTableWidget);
		QObject::connect(textWidget, SIGNAL(notifyEnterPressed(QString, int, int)), this, SLOT(slotNewPriceCellUpdated(QString, int, int)));
		textWidget->setTextFormatterFunc(convertToQuantityFormat);
		textWidget->setText(price);
		textWidget->selectAll();
		ui.billTableWidget->setCellWidget(row, 2, textWidget);
		textWidget->setFocus();
	}
}
