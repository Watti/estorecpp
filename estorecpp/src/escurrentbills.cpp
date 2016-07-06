#include "escurrentbills.h"
#include "utility/esmainwindowholder.h"
#include "utility/esdbconnection.h"
#include <QMessageBox>
#include "QPushButton"
#include "esaddbill.h"
#include "esmainwindow.h"
#include "utility/session.h"
#include "entities/user.h"
#include "utility/utility.h"
#include "KDReportsReport.h"
#include "KDReportsVariableType.h"
#include "KDReportsCell.h"
#include "KDReportsTextElement.h"
#include "KDReportsTableElement.h"
#include "KDReportsHeader.h"
#include "KDReportsHtmlElement.h"
#include "QPrintPreviewDialog"

ESCurrentBills::ESCurrentBills(QWidget *parent)
: QWidget(parent)
{
	ui.setupUi(this);
	m_proceedButtonSignalMapper = new QSignalMapper(this);
	m_voidBillButtonSignalMapper = new QSignalMapper(this);
	m_reprintBillButtonSignalMapper = new QSignalMapper(this);
	QObject::connect(m_proceedButtonSignalMapper, SIGNAL(mapped(QString)), this, SLOT(slotProceed(QString)));
	QObject::connect(m_voidBillButtonSignalMapper, SIGNAL(mapped(QString)), this, SLOT(slotVoidBill(QString)));
	QObject::connect(m_reprintBillButtonSignalMapper, SIGNAL(mapped(QString)), this, SLOT(slotReprint(QString)));

	ui.startDate->setDisplayFormat("yyyy-MM-dd");
	ui.endDate->setDisplayFormat("yyyy-MM-dd");

	QStringList headerLabels;
	headerLabels.append("Bill ID");
	headerLabels.append("Date");
	//headerLabels.append("Payment Method");
	headerLabels.append("Amount");
	headerLabels.append("User");
	headerLabels.append("Status");
	headerLabels.append("Actions");

	ui.tableWidget->setHorizontalHeaderLabels(headerLabels);
	ui.tableWidget->horizontalHeader()->setStretchLastSection(true);
	ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);

	QObject::connect(ui.userComboBox, SIGNAL(activated(QString)), this, SLOT(slotSearch()));
	QObject::connect(ui.statusComboBox, SIGNAL(activated(QString)), this, SLOT(slotSearch()));
	QObject::connect(ui.startDate, SIGNAL(dateChanged(const QDate&)), this, SLOT(slotSearch()));
	QObject::connect(ui.endDate, SIGNAL(dateChanged(const QDate&)), this, SLOT(slotSearch()));

	if (!ES::DbConnection::instance()->open())
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Critical);
		mbox.setText(QString("Cannot connect to the database : ESManageStockItems::displayStockItems "));
		mbox.exec();
	}
	else
	{
		QSqlQuery queryUser("SELECT * FROM user");
		ui.userComboBox->addItem("ALL", 0);
		while (queryUser.next())
		{
			ui.userComboBox->addItem(queryUser.value(1).toString(), queryUser.value(0).toInt());
		}

		ui.statusComboBox->addItem("ALL", 0);
		ui.statusComboBox->addItem("COMMITTED", 1);
		ui.statusComboBox->addItem("PENDING", 2);
		ui.statusComboBox->addItem("CANCELED", 3);

		ui.startDate->setDate(QDate::currentDate().addMonths(-1));
		ui.endDate->setDate(QDate::currentDate());

	}

	slotSearch();
}

ESCurrentBills::~ESCurrentBills()
{

}

void ESCurrentBills::slotSearch()
{
	while (ui.tableWidget->rowCount() > 0)
	{
		ui.tableWidget->removeRow(0);
	}

	int selectedUser = ui.userComboBox->currentData().toInt();
	int selectedStatus = ui.statusComboBox->currentData().toInt();

	QDateTime startDate = QDateTime::fromString(ui.startDate->text(), Qt::ISODate);
	QDateTime endDate = QDateTime::fromString(ui.endDate->text(), Qt::ISODate);
	endDate.setTime(QTime(23,59, 59));

	int row = 0;
	QSqlQuery allBillQuery("SELECT * FROM bill WHERE deleted = 0 AND DATE(date) = DATE(CURDATE())");
	while (allBillQuery.next())
	{		
		if (selectedUser > 0)
		{
			if (selectedUser != allBillQuery.value("user_id").toInt())
			{
				continue;
			}
		}
		QDateTime billedDate = QDateTime::fromString(allBillQuery.value("date").toString(), Qt::ISODate);
		if (billedDate < startDate || billedDate > endDate)
		{
			continue;
		}
		int statusId = allBillQuery.value("status").toInt();
		if (selectedStatus > 0)
		{
			if (selectedStatus != statusId)
			{
				continue;
			}
		}

		QTableWidgetItem* tableItem = NULL;
		row = ui.tableWidget->rowCount();
		ui.tableWidget->insertRow(row);		
		QColor rowColor;
		QString billId = allBillQuery.value("bill_id").toString();
		switch (statusId)
		{
		case 1:
		{
			rowColor.setRgb(169, 245, 208);
			tableItem = new QTableWidgetItem("COMMITTED");
			tableItem->setTextAlignment(Qt::AlignHCenter);
			tableItem->setBackgroundColor(rowColor);
			ui.tableWidget->setItem(row, 4, tableItem);

			QWidget* base = new QWidget(ui.tableWidget);
			QHBoxLayout *layout = new QHBoxLayout;
			layout->setContentsMargins(0, 0, 0, 0);
			
			ES::User::UserType uType = ES::Session::getInstance()->getUser()->getType();
			if (uType == ES::User::SENIOR_MANAGER || uType == ES::User::DEV || uType == ES::User::MANAGER)
			{
				QPushButton* voidBtn = new QPushButton("Cancel", base);
				voidBtn->setMaximumWidth(100);
				m_voidBillButtonSignalMapper->setMapping(voidBtn, billId);
				QObject::connect(voidBtn, SIGNAL(clicked()), m_voidBillButtonSignalMapper, SLOT(map()));
				layout->addWidget(voidBtn);
			}

 			QPushButton* reprintBtn = new QPushButton("Reprint", base);
 			reprintBtn->setMaximumWidth(100);
 			m_reprintBillButtonSignalMapper->setMapping(reprintBtn, billId);
 			QObject::connect(reprintBtn, SIGNAL(clicked()), m_reprintBillButtonSignalMapper, SLOT(map()));
 			layout->addWidget(reprintBtn);
						
			layout->insertStretch(2);
			base->setLayout(layout);
			ui.tableWidget->setCellWidget(row, 5, base);
			base->show();
		}
			break;
		case 2:
		{
			rowColor.setRgb(254, 239, 179);
			tableItem = new QTableWidgetItem("PENDING");
			tableItem->setTextAlignment(Qt::AlignHCenter);
			tableItem->setBackgroundColor(rowColor);
			ui.tableWidget->setItem(row, 4, tableItem);

			QWidget* base = new QWidget(ui.tableWidget);
			QPushButton* proceedBtn = new QPushButton("Proceed", base);
			proceedBtn->setMaximumWidth(100);

			m_proceedButtonSignalMapper->setMapping(proceedBtn, billId);
			QObject::connect(proceedBtn, SIGNAL(clicked()), m_proceedButtonSignalMapper, SLOT(map()));
			
			QHBoxLayout *layout = new QHBoxLayout;
			layout->setContentsMargins(0, 0, 0, 0);
			layout->addWidget(proceedBtn);
			layout->insertStretch(2);
			base->setLayout(layout);
			ui.tableWidget->setCellWidget(row, 5, base);
			base->show();
		}
			break;
		case 3:
		{
			rowColor.setRgb(245, 169, 169);
			tableItem = new QTableWidgetItem("CANCELED");
			tableItem->setTextAlignment(Qt::AlignHCenter);
			tableItem->setBackgroundColor(rowColor);
			ui.tableWidget->setItem(row, 4, tableItem);
		}
			break;
		default:
		{
			rowColor.setRgb(255, 255, 255);
			tableItem = new QTableWidgetItem("UNSPECIFIED");
			tableItem->setTextAlignment(Qt::AlignHCenter);
			tableItem->setBackgroundColor(rowColor);
			ui.tableWidget->setItem(row, 4, tableItem);
		}
			
			break;
		}

		tableItem = new QTableWidgetItem(billId);
		tableItem->setBackgroundColor(rowColor);
		ui.tableWidget->setItem(row, 0, tableItem);
		QDateTime datetime = QDateTime::fromString(allBillQuery.value("date").toString(), Qt::ISODate);
		tableItem = new QTableWidgetItem(datetime.toString(Qt::SystemLocaleShortDate));
		tableItem->setBackgroundColor(rowColor);
		ui.tableWidget->setItem(row, 1, tableItem);


		//QSqlQuery qSales("SELECT * FROM payment WHERE bill_id = " + billId);
		//if (qSales.next())
		{
			tableItem = new QTableWidgetItem(QString::number(allBillQuery.value("amount").toDouble(), 'f', 2));
			tableItem->setTextAlignment(Qt::AlignRight);
			tableItem->setBackgroundColor(rowColor);
			ui.tableWidget->setItem(row, 2, tableItem);
		}

		QString userQueryStr("SELECT * FROM user WHERE user_id=");
		userQueryStr.append(allBillQuery.value("user_id").toString());
		QSqlQuery userQuery(userQueryStr);
		if (userQuery.first())
		{
			tableItem = new QTableWidgetItem(userQuery.value("display_name").toString());
			tableItem->setTextAlignment(Qt::AlignHCenter);
			tableItem->setBackgroundColor(rowColor);
			ui.tableWidget->setItem(row, 3, tableItem);
		}
	}

}

void ESCurrentBills::slotProceed(QString billId)
{
	ESAddBill* addBill = new ESAddBill(this);
	ES::MainWindowHolder::instance()->getMainWindow()->setCentralWidget(addBill);
	addBill->proceedPendingBill(billId);
	addBill->show();
}

void ESCurrentBills::slotVoidBill(QString billId)
{
	if (ES::Utility::verifyUsingMessageBox(this, "ProgexPOS", "Do you really want to cancel this?"))
	{
		QString billQryStr = "SELECT * FROM bill WHERE bill_id  = " + billId;
		QSqlQuery BillQry(billQryStr);
		if (BillQry.next())
		{
			QString queryUpdateStr("UPDATE bill set status = 3 WHERE bill_id = " + billId);
			QSqlQuery query(queryUpdateStr);

			QString saleQryStr("SELECT * FROM sale WHERE deleted = 0 AND bill_id = " + billId);
			QSqlQuery querySale(saleQryStr);
			while (querySale.next())
			{
				QString stockId = querySale.value("stock_id").toString();
				double qty = querySale.value("quantity").toDouble();


				QString stockQryStr("SELECT * FROM stock WHERE stock_id = " + stockId);
				QSqlQuery queryStock(stockQryStr);
				if (queryStock.next())
				{
					double currentQty = queryStock.value("qty").toDouble();
					double newQty = currentQty + qty;

					QSqlQuery updateStock("UPDATE stock set qty = " + QString::number(newQty) + " WHERE stock_id = " + stockId);
				}
			}
		}
		slotSearch();
	}
	
}

void ESCurrentBills::slotPrint(QPrinter* printer)
{
	report.print(printer);
	//this->close();
}

void ESCurrentBills::slotReprint(QString billIdStr)
{
	int billId = billIdStr.toInt();
	QSqlQuery q("SELECT * FROM bill WHERE bill_id = " + billIdStr);

	QString userName = "";
	QString customerId = q.value("customer_id").toString();
	QString qStrUser("SELECT display_name FROM user WHERE user_id = " + q.value("user_id").toString());
	QSqlQuery queryUser(qStrUser);
	if (queryUser.next())
	{
		userName = queryUser.value("display_name").toString();
	}

	double total = 0.0;

	if (q.next())
	{
		//KDReports::Report report;

		QString dateStr = "Date : ";
		dateStr.append(QDateTime::currentDateTime().toString("yyyy-MM-dd"));
		QString timeStr = "Time : ";
		timeStr.append(QDateTime::currentDateTime().toString("hh : mm"));
		QString billIdStr("Bill No : " + QString::number(billId));
		/*
		PUJITHA ENTERPRISES (PVT) LTD
		No. 154, Kurugala, Padukka
		Phone :  077-4784430 / 077-4784437
		email :rapprasanna4@gmail.com


		HIRUNA MARKETING (PVT) LTD
		No.374, High level Road, Meegoda
		*/
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

		QString emailStr = ES::Session::getInstance()->getBillEmail();
		if (emailStr != "")
		{
			KDReports::TextElement emailElement(emailStr);
			emailElement.setPointSize(10);
			emailElement.setBold(false);
			report.addElement(emailElement, Qt::AlignHCenter);
		}

		KDReports::TableElement infoTableElement;
		infoTableElement.setHeaderRowCount(2);
		infoTableElement.setHeaderColumnCount(2);
		infoTableElement.setBorder(0);
		infoTableElement.setWidth(100, KDReports::Percent);
		{
			KDReports::Cell& billIdCell = infoTableElement.cell(0, 0);
			KDReports::TextElement t(billIdStr);
			t.setPointSize(10);
			billIdCell.addElement(t, Qt::AlignLeft);
		}
		{
			KDReports::Cell& userNameCell = infoTableElement.cell(1, 0);
			KDReports::TextElement t("Cashier : " + userName);
			t.setPointSize(10);
			userNameCell.addElement(t, Qt::AlignLeft);
		}
		{
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

		QString querySaleStr("SELECT * FROM sale WHERE bill_id = " + QString::number(billId) + " AND deleted = 0");
		QSqlQuery querySale(querySaleStr);

		//columns (item_code, Description, UnitPrice, Discount, Qty, Sub Total)


		QStringList srtList;
		QSqlQuery queryPaymentType("SELECT * FROM payment WHERE bill_id = " + QString::number(billId));
		QString paymentTypes = "";
		float totalPayingAmount = 0;
		while (queryPaymentType.next())
		{
			//paymentTypes.append(queryPaymentType.value("payment_type").toString());
			QString paymentType = queryPaymentType.value("payment_type").toString();
			QString paymentId = queryPaymentType.value("payment_id").toString();

			if (paymentType == "CARD")
			{
				QSqlQuery queryCard("SELECT * FROM card WHERE payment_id = " + paymentId);
				if (queryCard.next())
				{
					float interest = queryCard.value("interest").toFloat();
					float amount = queryCard.value("amount").toFloat();
					//total += amount;
					float netAmount = amount;
					amount = amount + (amount * interest) / 100;
					totalPayingAmount += amount;
					srtList.append(paymentType + "(" + QString::number(netAmount, 'f', 2) + " +" + QString::number(interest, 'f', 2) + "%): " + QString::number(amount, 'f', 2));
				}
			}
			else if (paymentType == "CHEQUE")
			{
				QSqlQuery query("SELECT * FROM cheque WHERE payment_id = " + paymentId);
				if (query.next())
				{
					float interest = query.value("interest").toFloat();
					float amount = query.value("amount").toFloat();
					//total += amount;
					float netAmount = amount;
					amount = amount + (amount * interest) / 100;
					totalPayingAmount += amount;
					srtList.append(paymentType + "(" + QString::number(netAmount, 'f', 2) + " +" + QString::number(interest, 'f', 2) + "%): " + QString::number(amount, 'f', 2));
				}
			}
			else if (paymentType == "CREDIT")
			{
				QSqlQuery query("SELECT * FROM credit WHERE payment_id = " + paymentId);
				if (query.next())
				{
					float interest = query.value("interest").toFloat();
					float amount = query.value("amount").toFloat();
					//total += amount;
					float netAmount = amount;
					amount = amount + (amount * interest) / 100;
					totalPayingAmount += amount;
					srtList.append(paymentType + "(" + QString::number(netAmount, 'f', 2) + " +" + QString::number(interest, 'f', 2) + "%): " + QString::number(amount, 'f', 2));
				}
			}
			else if (paymentType == "CASH")
			{
				QSqlQuery query("SELECT * FROM cash WHERE payment_id = " + paymentId);
				if (query.next())
				{
					float amount = query.value("amount").toFloat();
					//total += amount;
					totalPayingAmount += amount;
					srtList.append(paymentType + " : " + QString::number(amount, 'f', 2));
				}
			}
			else if (paymentType == "LOYALTY")
			{
			}
		}

		KDReports::TableElement tableElement;
		//tableElement.setHeaderRowCount(5);
		tableElement.setHeaderColumnCount(6);
		tableElement.setBorder(0);
		tableElement.setWidth(100, KDReports::Percent);

		KDReports::Cell& cICode = tableElement.cell(0, 0);
		KDReports::TextElement tICode("Item Code");
		tICode.setPointSize(11);
		tICode.setBold(true);
		cICode.addElement(tICode, Qt::AlignLeft);

		KDReports::Cell& cIName = tableElement.cell(0, 1);
		KDReports::TextElement tEItem("Item");
		tEItem.setBold(true);
		tEItem.setPointSize(11);
		cIName.addElement(tEItem, Qt::AlignLeft);

		KDReports::Cell& cPrice = tableElement.cell(0, 2);
		KDReports::TextElement tEPrice("Unit Price");
		tEPrice.setPointSize(11);
		tEPrice.setBold(true);
		cPrice.addElement(tEPrice, Qt::AlignRight);

		KDReports::Cell& cDiscount = tableElement.cell(0, 3);
		KDReports::TextElement tEDiscount("Discount");
		tEDiscount.setPointSize(11);
		tEDiscount.setBold(true);
		cDiscount.addElement(tEDiscount, Qt::AlignRight);

		KDReports::Cell& cQty = tableElement.cell(0, 4);
		KDReports::TextElement tEQty("Qty");
		tEQty.setPointSize(11);
		tEQty.setBold(true);
		cQty.addElement(tEQty, Qt::AlignRight);

		KDReports::Cell& cTotal = tableElement.cell(0, 5);
		KDReports::TextElement tETotal("Line Total");
		tETotal.setPointSize(11);
		tETotal.setBold(true);
		cTotal.addElement(tETotal, Qt::AlignRight);

		//	report.addVerticalSpacing(6);
		KDReports::HtmlElement htmlElem;
		QString htm("<div><hr/></div>");
		htmlElem.setHtml(htm);
		report.addElement(htmlElem);

		int row = 1;
		int noOfPcs = 0, noOfItems = 0;
		while (querySale.next())
		{
			QString stockId = querySale.value("stock_id").toString();
			QString discount = QString::number(querySale.value("discount").toDouble(), 'f', 2);
			QString qty = querySale.value("quantity").toString();
			noOfPcs += qty.toInt();
			noOfItems++;
			//QString subTotal = QString::number(querySale.value("total").toDouble(), 'f', 2);
			double subTotal = (querySale.value("total").toDouble() *(100 - querySale.value("discount").toDouble()) / 100);
			QString subTotalStr = QString::number(subTotal, 'f', 2);
			QString itemName = "";
			QString unitPrice = "";
			QString itemCode = "";

			//get the item name from the item table
			QString qItemStr("SELECT it.item_code, it.item_name , st.selling_price FROM stock st JOIN item it ON st.item_id = it.item_id AND st.stock_id = " + stockId);
			QSqlQuery queryItem(qItemStr);
			if (queryItem.next())
			{
				itemName = queryItem.value("item_name").toString();
				unitPrice = QString::number(queryItem.value("selling_price").toDouble(), 'f', 2);
				itemCode = queryItem.value("item_code").toString();
			}
			//columns (item_code, Description, UnitPrice, Discount, Qty, Sub Total)
			printRow(tableElement, row, 0, itemCode);
			printRow(tableElement, row, 1, itemName);
			printRow(tableElement, row, 2, unitPrice, Qt::AlignRight);
			printRow(tableElement, row, 3, discount, Qt::AlignRight);
			printRow(tableElement, row, 4, qty, Qt::AlignRight);
			printRow(tableElement, row, 5, subTotalStr, Qt::AlignRight);
			row++;
		}

		//report.addVerticalSpacing(5);
		{
			KDReports::Cell& emptyCell = tableElement.cell(row, 0);
			KDReports::HtmlElement htmlElem;
			QString html("<div><hr/></div>");
			htmlElem.setHtml(html);
			emptyCell.setColumnSpan(6);
			emptyCell.addElement(htmlElem);
			row++;
		}

		//
		KDReports::Cell& totalTextC = tableElement.cell(row, 0);
		totalTextC.setColumnSpan(5);
		KDReports::TextElement totalTxt("Sub Total ");
		totalTxt.setPointSize(11);
		totalTxt.setBold(true);
		totalTextC.addElement(totalTxt, Qt::AlignRight);

		KDReports::Cell& totalCell = tableElement.cell(row, 5);
		KDReports::TextElement totalValue(QString::number(total, 'f', 2));
		totalValue.setPointSize(11);
		totalValue.setBold(true);
		totalCell.addElement(totalValue, Qt::AlignRight);

		row++;
		KDReports::Cell& payableTextC = tableElement.cell(row, 0);
		payableTextC.setColumnSpan(5);
		KDReports::TextElement payableTxt("Total ");
		payableTxt.setPointSize(11);
		payableTxt.setBold(true);
		payableTextC.addElement(payableTxt, Qt::AlignRight);

		KDReports::Cell& payableCell = tableElement.cell(row, 5);
		KDReports::TextElement payableValue(QString::number(totalPayingAmount, 'f', 2));
		payableValue.setPointSize(11);
		payableValue.setBold(true);
		payableCell.addElement(payableValue, Qt::AlignRight);

		//
		row++;
		KDReports::Cell& countText = tableElement.cell(row, 0);
		countText.setColumnSpan(5);
		KDReports::TextElement noOfItemsTxt("# of Items ");
		noOfItemsTxt.setPointSize(11);
		noOfItemsTxt.setBold(true);
		countText.addElement(noOfItemsTxt, Qt::AlignRight);

		KDReports::Cell& countItemCell = tableElement.cell(row, 5);
		KDReports::TextElement itemCountValue(QString::number(noOfItems));
		itemCountValue.setPointSize(11);
		itemCountValue.setBold(true);
		countItemCell.addElement(itemCountValue, Qt::AlignRight);

		//
		row++;
		KDReports::Cell& pcsText = tableElement.cell(row, 0);
		pcsText.setColumnSpan(5);
		KDReports::TextElement noOfPcsTxt("# of Pieces ");
		noOfPcsTxt.setPointSize(11);
		noOfPcsTxt.setBold(true);
		pcsText.addElement(noOfPcsTxt, Qt::AlignRight);

		KDReports::Cell& pcsItemCell = tableElement.cell(row, 5);
		KDReports::TextElement itemPcsValue(QString::number(noOfPcs));
		itemPcsValue.setPointSize(11);
		itemPcsValue.setBold(true);
		pcsItemCell.addElement(itemPcsValue, Qt::AlignRight);
		//
		report.addElement(tableElement);

		report.addVerticalSpacing(5);

		KDReports::HtmlElement htmlElem1;
		QString htm1("<div><hr/></div>");
		htmlElem1.setHtml(htm1);
		report.addElement(htmlElem1);

		KDReports::TextElement payementInfo(" Payment Summary ");
		payementInfo.setPointSize(11);
		report.addElement(payementInfo, Qt::AlignCenter);
		for (QString s : srtList)
		{
			KDReports::TextElement paymentTE(s);
			paymentTE.setPointSize(11);
			report.addElement(paymentTE, Qt::AlignLeft);
		}

		report.addVerticalSpacing(1);

		KDReports::HtmlElement htmlElem2;
		QString htm2("<div><hr/></div>");
		htmlElem2.setHtml(htm2);
		report.addElement(htmlElem2);
		// customer info	
		if (customerId == "-1")
		{
			QString customer = "Bill To : N/A";

			KDReports::TextElement customerInfo(customer);
			customerInfo.setPointSize(11);
			report.addElement(customerInfo, Qt::AlignLeft);
		}
		else
		{
			QString customer = "Bill To : ";
			QSqlQuery q("SELECT * FROM customer WHERE customer_id = " + customerId);
			if (q.next())
			{
				customer.append(q.value("customer_id").toString());
				customer.append(" / ");
				customer.append(q.value("name").toString());
			}
			KDReports::TextElement customerInfo(customer);
			customerInfo.setPointSize(11);
			report.addElement(customerInfo, Qt::AlignLeft);
		}

		report.addVerticalSpacing(5);

		KDReports::TextElement customerInfo("Thank You!");
		customerInfo.setPointSize(11);
		report.addElement(customerInfo, Qt::AlignCenter);

		report.addVerticalSpacing(5);

		// 	KDReports::Footer& foter = report.footer();
		// 	KDReports::TextElement info("Powered by PROGEX Technologies.");
		// 	KDReports::TextElement web("www.progextech.com  T.P.: 072-6430268/071-1308531");
		// 	foter.addElement(info, Qt::AlignCenter);
		// 	foter.addElement(web, Qt::AlignCenter);
// 		KDReports::TextElement poweredBy("Powered by PROGEX Technologies.");
// 		KDReports::TextElement web("www.progextech.com  T.P.: 072-6430268/071-1308531");
// 		report.addElement(poweredBy, Qt::AlignCenter);
// 		report.addElement(web, Qt::AlignCenter);

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
}

void ESCurrentBills::printRow(KDReports::TableElement& tableElement, int row, int col, QString elementStr, Qt::AlignmentFlag alignment)
{
	KDReports::Cell& cell = tableElement.cell(row, col);
	KDReports::TextElement te(elementStr);
	te.setPointSize(ES::Session::getInstance()->getBillItemFontSize());
	cell.addElement(te, alignment);
}
