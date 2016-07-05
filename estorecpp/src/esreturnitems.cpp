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

ESReturnItems::ESReturnItems(QWidget *parent /*= 0*/) : QWidget(parent)
{
	ui.setupUi(this);
	QObject::connect(ui.addButton, SIGNAL(clicked()), this, SLOT(slotAddReturnedItem()));

	if (!ES::DbConnection::instance()->open())
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Critical);
		mbox.setText(QString("Cannot connect to the database : ESReturnItems"));
		mbox.exec();
	}

	ui.qtyText->setText("1.0");
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
	QString iName = ui.itemName->text();
	QString iCode = ui.itemCode->text();
	QString remarks = ui.remarks->toPlainText();
	//QString catId = ui.itemCategoryComboBox->itemData(ui.itemCategoryComboBox->currentIndex()).toString();
	QString iPrice = ui.itemPrice->text();
	QString qty = ui.qtyText->text();

	if (iCode == nullptr || iCode.isEmpty() ||
		qty == nullptr || qty.isEmpty() ||/* catId == "-1" ||*/ iPrice == nullptr || iPrice.isEmpty())
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
	this->close();
}


void ESReturnItems::printReturnItemInfo()
{
	KDReports::Report report;

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
}

void ESReturnItems::slotPrint(QPrinter* printer)
{
	//report.print(printer);
	this->close();
}