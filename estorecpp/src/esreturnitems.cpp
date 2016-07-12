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

ESReturnItems::ESReturnItems(QWidget *parent /*= 0*/) : QWidget(parent), m_total(0)
{
	ui.setupUi(this);

	m_removeButtonSignalMapper = new QSignalMapper(this);
	m_idGenerator = 0;
	m_billId = -1;

	QObject::connect(ui.selectBtn, SIGNAL(clicked()), this, SLOT(slotSelect()));
	QObject::connect(ui.printBtn, SIGNAL(clicked()), this, SLOT(slotPrintReturnBill()));
	QObject::connect(ui.tableWidget, SIGNAL(cellClicked(int, int)), this, SLOT(slotItemDoubleClicked(int, int)));
	QObject::connect(m_removeButtonSignalMapper, SIGNAL(mapped(QString)), this, SLOT(slotRemove(QString)));

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

	ui.billIdLbl->setText("N/A");
	ui.totalLbl->setText("0.00");

	ui.itemCode->setFocus();
}

ESReturnItems::~ESReturnItems()
{

}

Ui::ReturnItems& ESReturnItems::getUI()
{
	return ui;
}

void ESReturnItems::slotAddReturnedItem()
{
	/*	QString iName = ui.itemName->text();
		QString iCode = ui.itemCode->text();
		QString remarks = ui.remarks->toPlainText();
		//QString catId = ui.itemCategoryComboBox->itemData(ui.itemCategoryComboBox->currentIndex()).toString();
		QString iPrice = ui.itemPrice->text();
		QString qty = ui.qtyText->text();

		if (iCode == nullptr || iCode.isEmpty() ||
		qty == nullptr || qty.isEmpty() || iPrice == nullptr || iPrice.isEmpty())
		{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Warning);
		mbox.setText(QString("Following fields should not be empty : Item Code, Item Name, Unit, Category, Item Price"));
		mbox.exec();
		return;
		}
		if (qty.toDouble() <= 0)
		{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Critical);
		mbox.setText(QString("Invalid value for Quantity"));
		mbox.exec();
		return;
		}
		QString q("SELECT * FROM item WHERE item_code = '" + iCode + "'");
		QSqlQuery query(q);
		if (query.next())
		{
		QString itemId = query.value("item_id").toString();
		q = "SELECT * FROM stock WHERE item_id = " + itemId;
		query.exec(q);
		if (query.first())
		{
		double currentQty = query.value("qty").toDouble();
		double currentPrice = query.value("selling_price").toDouble();

		bool isValid = false;
		double returnedItemPrice = iPrice.toDouble(&isValid);
		if (!isValid)
		{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Critical);
		mbox.setText(QString("Price should be a number"));
		mbox.exec();
		return;
		}
		// 			if (returnedItemPrice != currentPrice)
		// 			{
		// 				ES::Utility::verifyUsingMessageBox(this, "Return Item", "Price of this item doesn't match with the stock. Do you want to proceed with this price? ");
		// 				QMessageBox mbox;
		// 				mbox.setIcon(QMessageBox::Warning);
		// 				mbox.setText(QString("Price of the returned Item does not match with the prices of the stock"));
		// 				mbox.exec();
		// 			}
		isValid = false;
		double retunedQty = qty.toDouble(&isValid);
		if (!isValid)
		{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Critical);
		mbox.setText(QString("Quantity should be a number"));
		mbox.exec();
		return;
		}
		//currentQty += retunedQty;
		// 				q = "UPDATE stock set qty = "+QString::number(currentQty)+" WHERE item_id = "+itemId;
		// 				if (!query.exec(q))
		// 				{
		// 					QMessageBox mbox;
		// 					mbox.setIcon(QMessageBox::Critical);
		// 					mbox.setText(QString("Error has been occurred while updating the stock quantity"));
		// 					mbox.exec();
		// 					LOG(ERROR) << "Failed update stock when return item handling query = "<<q.toLatin1().data();
		// 				}
		int uId = ES::Session::getInstance()->getUser()->getId();
		q = "INSERT INTO return_item (item_id, item_price, user_id, remarks) VALUES (" + itemId + ", " + iPrice + ", " + QString::number(uId) + ",'" + remarks + "')";
		if (!query.exec(q))
		{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Critical);
		mbox.setText(QString("Error has been occurred while updating the stock quantity"));
		mbox.exec();
		LOG(ERROR) << "Failed to insert in to return table. query = " << q.toLatin1().data();
		}
		//TODO print the bill
		if (ui.doPrintCB->isChecked())
		{
		printReturnItemInfo();
		}

		}

		}
		else
		{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Critical);
		mbox.setText(QString("Invalid item code."));
		mbox.exec();
		return;
		}
		this->close();*/
}


void ESReturnItems::printReturnItemInfo()
{
	/*	KDReports::Report report;

		QString titleStr = ES::Session::getInstance()->getBillTitle();
		KDReports::TextElement titleElement(titleStr);
		titleElement.setPointSize(14);
		titleElement.setBold(true);
		report.addElement(titleElement, Qt::AlignHCenter);

		QString addressStr = ES::Session::getInstance()->getBillAddress();
		KDReports::TextElement addressElement(addressStr);
		addressElement.setPointSize(10);
		addressElement.setBold(false);
		report.addElement(addressElement, Qt::AlignHCenter);


		QString phoneStr = ES::Session::getInstance()->getBillPhone();
		KDReports::TextElement telElement(phoneStr);
		telElement.setPointSize(10);
		telElement.setBold(false);
		report.addElement(telElement, Qt::AlignHCenter);

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
		KDReports::Cell& userNameCell = infoTableElement.cell(1, 0);
		KDReports::TextElement t("Cashier : " + ES::Session::getInstance()->getUser()->getName());
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

		KDReports::TextElement retTitleElement("Return Bill Summary");
		retTitleElement.setPointSize(10);
		retTitleElement.setBold(true);
		report.addElement(retTitleElement, Qt::AlignHCenter);

		report.addVerticalSpacing(5);


		KDReports::HtmlElement htmlElem1;
		QString htm1("<div><hr/></div>");
		htmlElem1.setHtml(htm1);
		report.addElement(htmlElem1);

		{// Item Code
		KDReports::TextElement telElement("Item Code : " + ui.itemCode->text());
		telElement.setPointSize(12);
		telElement.setBold(false);
		report.addElement(telElement, Qt::AlignLeft);
		}{// Item Name
		QString itemCode = ui.itemCode->text();
		QSqlQuery queryItem("SELECT item_name FROM item WHERE deleted = 0 AND item_code = '" + ui.itemCode->text()+"'");
		if (queryItem.next())
		{
		QString itemName = queryItem.value("item_name").toString();
		KDReports::TextElement telElement("Item Name : " + queryItem.value("item_name").toString());
		telElement.setPointSize(12);
		telElement.setBold(false);
		report.addElement(telElement, Qt::AlignLeft);
		}
		}{// Quantity
		KDReports::TextElement telElement("Quantity : " + ui.qtyText->text());
		telElement.setPointSize(12);
		telElement.setBold(false);
		report.addElement(telElement, Qt::AlignLeft);
		}{// Remarks
		KDReports::TextElement telElement("Remarks : " + ui.remarks->toPlainText());
		telElement.setPointSize(12);
		telElement.setBold(false);
		report.addElement(telElement, Qt::AlignLeft);
		}{// Item Price
		QString itemp = "-" + QString::number(ui.itemPrice->text().toDouble(), 'f', 2);
		KDReports::TextElement telElement("Item Price : " + itemp);
		telElement.setPointSize(12);
		telElement.setBold(true);
		report.addElement(telElement, Qt::AlignLeft);
		}


		QPrinter printer;
		printer.setPaperSize(QPrinter::A4);

		printer.setFullPage(false);
		printer.setOrientation(QPrinter::Portrait);

		// 	QPrintPreviewDialog *dialog = new QPrintPreviewDialog(&printer, this);
		// 	QObject::connect(dialog, SIGNAL(paintRequested(QPrinter*)), this, SLOT(slotPrint(QPrinter*)));
		// 	dialog->setWindowTitle(tr("Print Document"));
		// 	ES::MainWindowHolder::instance()->getMainWindow()->setCentralWidget(dialog);
		// 	dialog->exec();

		report.print(&printer);
		*/
}

void ESReturnItems::slotPrint(QPrinter* printer)
{
	report.print(printer);
	this->close();
}

void ESReturnItems::slotPrintReturnBill()
{
	QString billedUser = "Billed Cashier : ";
	QSqlQuery q("SELECT display_name FROM user JOIN bill ON user.user_id = bill.user_id WHERE bill.bill_id = " + QString::number(m_billId));
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
	billIdStr.append(QString::number(m_billId));
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
			KDReports::TextElement t(ui.tableWidget->item(i, 0)->text());
			t.setPointSize(10);
			cell.addElement(t, Qt::AlignLeft);
		}{
			KDReports::Cell& cell = dataTableElement.cell(i, 1);
			KDReports::TextElement t(ui.tableWidget->item(i, 1)->text());
			t.setPointSize(10);
			cell.addElement(t, Qt::AlignLeft);
		}{
			unitPrice = ui.tableWidget->item(i, 2)->text().toDouble();
			KDReports::Cell& cell = dataTableElement.cell(i, 2);
			KDReports::TextElement t(ui.tableWidget->item(i, 2)->text());
			t.setPointSize(10);
			cell.addElement(t, Qt::AlignRight);
		}{
			KDReports::Cell& cell = dataTableElement.cell(i, 3);
			KDReports::TextElement t(ui.tableWidget->item(i, 3)->text());
			t.setPointSize(10);
			cell.addElement(t, Qt::AlignRight);
		}{
			qty = ui.tableWidget->item(i, 4)->text().toDouble();
			KDReports::Cell& cell = dataTableElement.cell(i, 4);
			KDReports::TextElement t(ui.tableWidget->item(i, 4)->text());
			t.setPointSize(10);
			cell.addElement(t, Qt::AlignRight);
		}
		m_total += unitPrice*qty;
	}

	row++;
	{
		KDReports::Cell& total = dataTableElement.cell(row, 3);
		//total.setColumnSpan(5);
		KDReports::TextElement totalTxt("Total ");
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
	report.addElement(dataTableElement);
	//report.addElement(htmlElem1);
	report.addVerticalSpacing(1);
	//////////////////////////////////////////////////////////////////////////

	// Update database
	int uId = ES::Session::getInstance()->getUser()->getId();
	for (int i = 0; i < ui.tableWidget->rowCount(); ++i)
	{
		QTableWidgetItem* item = ui.tableWidget->item(i, 0);
		QSqlQuery itemQuery("SELECT item_id FROM item WHERE item_code = '" + item->text() + "'");
		if (itemQuery.next())
		{
			// bill_id, item_id, qty, paid_price, return_total, user_id
			QSqlQuery q("INSERT INTO return_item (bill_id, item_id, qty, paid_price, return_total, user_id) VALUES (" +	
				QString::number(m_billId) + "," +
				itemQuery.value("item_id").toString() + "," +
				ui.tableWidget->item(i, 2)->text() + "," +
				ui.tableWidget->item(i, 4)->text() + "," +
				ui.tableWidget->item(i, 5)->text() + "," + 
				QString::number(uId) + ")"
				);
		}

	}

	//////////////////////////////////////////////////////////////////////////

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
	QString itemCode = ui.itemCode->text();
	QString billId = ui.billId->text();

	if (m_billId == -1)
	{
		m_billId = billId.toInt();
		ui.billIdLbl->setText(billId);
	}
	else
	{
		if (m_billId != billId.toInt())
		{
			QMessageBox mbox;
			mbox.setIcon(QMessageBox::Critical);
			mbox.setText(QString("A Return bill cannot have multiple bills. Please use separate return bill for this"));
			mbox.exec();
			return;
		}
	}

	int itemId = -1;
	QString itemName = "-1";
	QSqlQuery q("SELECT item_id, item_name FROM item WHERE item_code = '" + itemCode + "'");
	if (q.next())
	{
		itemId = q.value("item_id").toInt();
		itemName = q.value("item_name").toString();
	}
	else
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Critical);
		mbox.setText(QString("Invalid Item Code"));
		mbox.exec();
		return;
	}

	QSqlQuery q2("SELECT stock_id FROM stock WHERE item_id = " + QString::number(itemId));
	if (q2.next())
	{
		QString str("SELECT b.* FROM sale b JOIN stock s ON b.stock_id = s.stock_id AND s.stock_id = ");
		str.append(q2.value("stock_id").toString());
		str.append(" WHERE b.bill_id = ");
		str.append(billId);
		str.append(" AND b.deleted = 0");

		QSqlQuery q3(str);
		if (q3.next())
		{
			int row = ui.tableWidget->rowCount();
			ui.tableWidget->insertRow(row);
			ui.tableWidget->setItem(row, 0, new QTableWidgetItem(itemCode));
			ui.tableWidget->setItem(row, 1, new QTableWidgetItem(itemName));

			double quantity = q3.value("quantity").toDouble();
			QTableWidgetItem* qtyItem = new QTableWidgetItem(QString::number(quantity));
			qtyItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
			ui.tableWidget->setItem(row, 2, qtyItem);

			double itemPrice = q3.value("item_price").toDouble();
			QTableWidgetItem* itemPriceItem = new QTableWidgetItem(QString::number(itemPrice, 'f', 2));
			itemPriceItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
			ui.tableWidget->setItem(row, 3, itemPriceItem);

			double discount = q3.value("discount").toDouble();
			double paidPrice = itemPrice - (itemPrice * discount / 100.0);
			QTableWidgetItem* paidPriceItem = new QTableWidgetItem(QString::number(paidPrice, 'f', 2));
			paidPriceItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
			ui.tableWidget->setItem(row, 4, paidPriceItem);

			double returnPrice = paidPrice * quantity;
			QTableWidgetItem* retPriceItem = new QTableWidgetItem(QString::number(returnPrice, 'f', 2));
			retPriceItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
			ui.tableWidget->setItem(row, 5, retPriceItem);

			QDate d = q3.value("date").toDate();
			QTableWidgetItem* dateItem = new QTableWidgetItem(d.toString("yyyy-MM-dd"));
			dateItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
			ui.tableWidget->setItem(row, 6, dateItem);

			QWidget* base = new QWidget(ui.tableWidget);

			QPushButton* removeBtn = new QPushButton(base);
			removeBtn->setIcon(QIcon("icons/delete.png"));
			removeBtn->setIconSize(QSize(24, 24));
			removeBtn->setMaximumWidth(100);

			int rowId = m_idGenerator++;

			m_removeButtonSignalMapper->setMapping(removeBtn, QString::number(rowId));
			QObject::connect(removeBtn, SIGNAL(clicked()), m_removeButtonSignalMapper, SLOT(map()));

			QHBoxLayout *layout = new QHBoxLayout;
			layout->setContentsMargins(0, 0, 0, 0);
			layout->addWidget(removeBtn);
			layout->insertStretch(2);
			base->setLayout(layout);
			ui.tableWidget->setCellWidget(row, 7, base);
			base->show();

			ui.tableWidget->setItem(row, 8, new QTableWidgetItem(QString::number(rowId)));
			ui.tableWidget->setItem(row, 9, new QTableWidgetItem(QString::number(quantity)));
		}
	}

	calculateTotal();
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
	for (int i = 0; i < ui.tableWidget->rowCount(); ++i)
	{
		QTableWidgetItem* item = ui.tableWidget->item(i, 8);
		if (item)
		{
			if (item->text() == rowId)
			{
				ui.tableWidget->removeRow(i);
				break;
			}
		}
	}

	calculateTotal();
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

	calculateTotal();
}

void ESReturnItems::calculateTotal()
{
	double total = 0.0;

	for (int i = 0; i < ui.tableWidget->rowCount(); ++i)
	{
		QTableWidgetItem* item = ui.tableWidget->item(i, 5);
		if (item)
		{
			total += item->text().toDouble();
		}
	}

	ui.totalLbl->setText(QString::number(total, 'f', 2));
}
