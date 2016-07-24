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
#include "QString"
#include <memory>

ESCurrentBills::ESCurrentBills(QWidget *parent)
: QWidget(parent), m_startingLimit(0), m_pageOffset(15), m_nextCounter(0), m_maxNextCount(0)
{
	ui.setupUi(this);
	m_proceedButtonSignalMapper = new QSignalMapper(this);
	m_voidBillButtonSignalMapper = new QSignalMapper(this);
	m_reprintBillButtonSignalMapper = new QSignalMapper(this);
	m_invisibleButtonSignalMapper = new QSignalMapper(this);

	QObject::connect(m_proceedButtonSignalMapper, SIGNAL(mapped(QString)), this, SLOT(slotProceed(QString)));
	QObject::connect(m_voidBillButtonSignalMapper, SIGNAL(mapped(QString)), this, SLOT(slotVoidBill(QString)));
	QObject::connect(m_reprintBillButtonSignalMapper, SIGNAL(mapped(QString)), this, SLOT(slotReprint(QString)));
	QObject::connect(m_invisibleButtonSignalMapper, SIGNAL(mapped(QString)), this, SLOT(slotInvisible(QString)));
	QObject::connect(ui.nextBtn, SIGNAL(clicked()), this, SLOT(slotNext()));
	QObject::connect(ui.prevBtn, SIGNAL(clicked()), this, SLOT(slotPrev()));

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
	ui.prevBtn->setDisabled(true);
	ui.nextBtn->setDisabled(true);
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
	endDate.setTime(QTime(23, 59, 59));

	int row = 0;
	QString qStr, qRecordCountStr;
	qStr = "SELECT * FROM bill WHERE deleted = 0";
	qRecordCountStr = "SELECT COUNT(*) as c FROM bill WHERE deleted = 0";
	if (selectedUser > 0)
	{
		qStr.append(" AND user_id = ").append(QString::number(selectedUser));
		qRecordCountStr.append(" AND user_id = ").append(QString::number(selectedUser));
	}
	if (selectedStatus > 0)
	{
		qStr.append(" AND status = ").append(QString::number(selectedStatus));
		qRecordCountStr.append(" AND status = ").append(QString::number(selectedStatus));
	}
	if (!(ES::Session::getInstance()->getUser()->getType() == ES::User::SENIOR_MANAGER ||
		ES::Session::getInstance()->getUser()->getType() == ES::User::DEV))
	{

		qStr.append(" AND visible = 1");
		qRecordCountStr.append(" AND visible = 1");
	}
	qStr.append(" ORDER BY date DESC");
	QSqlQuery queryRecordCount(qRecordCountStr);
	if (queryRecordCount.next())
	{
		m_totalRecords = queryRecordCount.value("c").toInt();
	}
	//pagination start
	qStr.append(" LIMIT ").append(QString::number(m_startingLimit));
	qStr.append(" , ").append(QString::number(m_pageOffset));
	//pagination end
	QSqlQuery allBillQuery;
	if (allBillQuery.exec(qStr))
	{
		//pagination start
		m_maxNextCount = m_totalRecords / m_pageOffset;
		if (m_maxNextCount > m_nextCounter)
		{
			ui.nextBtn->setEnabled(true);
		}
		int currentlyShowdItemCount = (m_nextCounter + 1)*m_pageOffset;
		if (currentlyShowdItemCount >= m_totalRecords)
		{
			ui.nextBtn->setDisabled(true);
		}
		//pagination end
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

					  //////////////////////////////////////////////////////////////////////////

					  if (ES::Session::getInstance()->getUser()->getType() == ES::User::DEV ||
						  ES::Session::getInstance()->getUser()->getType() == ES::User::SENIOR_MANAGER)
					  {
						  QPushButton* hideBtn = new QPushButton("Hide", base);
						  hideBtn->setMaximumWidth(100);
						  m_invisibleButtonSignalMapper->setMapping(hideBtn, billId);
						  QObject::connect(hideBtn, SIGNAL(clicked()), m_invisibleButtonSignalMapper, SLOT(map()));
						  layout->addWidget(hideBtn);
					  }

					  //////////////////////////////////////////////////////////////////////////

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


	double total = 0.0;
	QString customerId = -1;
	if (q.next())
	{

		customerId = q.value("customer_id").toString();
		QString qStrUser("SELECT display_name FROM user WHERE user_id = " + q.value("user_id").toString());
		QSqlQuery queryUser(qStrUser);
		if (queryUser.next())
		{
			userName = queryUser.value("display_name").toString();
		}

		bool secondDisplayOn = ES::Session::getInstance()->isSecondDisplayOn();

		QString dateStr = "Date : ";
		dateStr.append(QDateTime::currentDateTime().toString("yyyy-MM-dd"));
		QString timeStr = "Time : ";
		timeStr.append(QDateTime::currentDateTime().toString("hh : mm"));
		QString billIdStr("Bill No : " + QString::number(billId));
		if (secondDisplayOn)
		{
			billIdStr = "Bill No : " + QString::number(billId % 10000);
		}

		struct PaymentSummaryElement
		{
			QString type, amount, no, date, interest, netAmount;
		};
		QVector<std::shared_ptr<PaymentSummaryElement>> payamentSummaryTableInfo;

		QSqlQuery queryPaymentType("SELECT * FROM payment WHERE bill_id = " + QString::number(billId));
		QString paymentTypes = "";
		QString payamentStr = "";
		float totalPayingAmount = 0;
		while (queryPaymentType.next())
		{
			QString paymentType = queryPaymentType.value("payment_type").toString();
			QString paymentId = queryPaymentType.value("payment_id").toString();

			if (paymentType == "CARD")
			{
				QSqlQuery queryCard("SELECT * FROM card WHERE payment_id = " + paymentId);
				if (queryCard.next())
				{
					float interest = queryCard.value("interest").toFloat();
					float amount = queryCard.value("amount").toFloat();
					float netAmount = amount;
					amount = amount + (amount * interest) / 100;
					totalPayingAmount += amount;

					std::shared_ptr<PaymentSummaryElement> pse = std::make_shared<PaymentSummaryElement>();
					pse->type = paymentType;
					pse->no = queryCard.value("card_no").toString();
					pse->date = "-";
					pse->amount = QString::number(amount, 'f', 2);
					pse->interest = QString::number(interest, 'f', 2) + "%";
					pse->netAmount = QString::number(netAmount, 'f', 2);
					payamentSummaryTableInfo.push_back(pse);
				}
			}
			else if (paymentType == "CHEQUE")
			{
				QSqlQuery query("SELECT * FROM cheque WHERE payment_id = " + paymentId);
				if (query.next())
				{
					float interest = query.value("interest").toFloat();
					float amount = query.value("amount").toFloat();
					float netAmount = amount;
					amount = amount + (amount * interest) / 100;
					totalPayingAmount += amount;

					std::shared_ptr<PaymentSummaryElement> pse = std::make_shared<PaymentSummaryElement>();
					pse->type = paymentType;
					pse->no = query.value("cheque_number").toString();
					pse->amount = QString::number(amount, 'f', 2);
					pse->date = query.value("due_date").toString();
					pse->interest = QString::number(interest, 'f', 2) + "%";
					pse->netAmount = QString::number(netAmount, 'f', 2);
					payamentSummaryTableInfo.push_back(pse);
				}
			}
			else if (paymentType == "CREDIT")
			{
				QSqlQuery query("SELECT * FROM credit WHERE payment_id = " + paymentId);
				if (query.next())
				{
					float interest = query.value("interest").toFloat();
					float amount = query.value("amount").toFloat();
					float netAmount = amount;
					amount = amount + (amount * interest) / 100;
					totalPayingAmount += amount;

					std::shared_ptr<PaymentSummaryElement> pse = std::make_shared<PaymentSummaryElement>();
					pse->type = paymentType;
					pse->date = query.value("due_date").toString();
					pse->no = "-";
					pse->amount = QString::number(amount, 'f', 2);
					payamentSummaryTableInfo.push_back(pse);
					pse->interest = QString::number(interest, 'f', 2) + "%";
					pse->netAmount = QString::number(netAmount, 'f', 2);
				}
			}
			else if (paymentType == "CASH")
			{
				QSqlQuery query("SELECT * FROM cash WHERE payment_id = " + paymentId);
				if (query.next())
				{
					float amount = query.value("amount").toFloat();
					totalPayingAmount += amount;

					std::shared_ptr<PaymentSummaryElement> pse = std::make_shared<PaymentSummaryElement>();
					pse->type = paymentType;
					pse->date = "-";
					pse->no = "-";
					pse->amount = QString::number(amount, 'f', 2);
					pse->interest = "0%";
					pse->netAmount = QString::number(amount, 'f', 2);
					payamentSummaryTableInfo.push_back(pse);
				}
			}
			else if (paymentType == "LOYALTY")
			{
			}
		}

		KDReports::TableElement infoTableElement;
		infoTableElement.setHeaderRowCount(3);
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
			QSqlQuery queryCustomer("SELECT * FROM customer WHERE customer_id = " + customerId);
			if (queryCustomer.next())
			{
				KDReports::Cell& userNameCell = infoTableElement.cell(1, 0);
				KDReports::TextElement t("Customer : " + queryCustomer.value("name").toString());
				t.setPointSize(10);
				userNameCell.addElement(t, Qt::AlignLeft);
			}
		}
		{
			KDReports::Cell& userNameCell = infoTableElement.cell(2, 0);
			KDReports::TextElement t("Cashier : " + userName);
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

		KDReports::HtmlElement htmlElem;
		QString htm("<div><hr/></div>");
		htmlElem.setHtml(htm);
		report.addElement(htmlElem);

		QString querySaleStr("SELECT * FROM sale WHERE bill_id = " + QString::number(billId) + " AND deleted = 0");
		QSqlQuery querySale(querySaleStr);

		KDReports::TableElement tableElement;
		tableElement.setHeaderColumnCount(6);
		if (secondDisplayOn)
		{
			tableElement.setHeaderColumnCount(5);
		}
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

		if (secondDisplayOn)
		{
			KDReports::Cell& cQty = tableElement.cell(0, 3);
			KDReports::TextElement tEQty("Qty");
			tEQty.setPointSize(11);
			tEQty.setBold(true);
			cQty.addElement(tEQty, Qt::AlignRight);

			KDReports::Cell& cTotal = tableElement.cell(0, 4);
			KDReports::TextElement tETotal("Line Total");
			tETotal.setPointSize(11);
			tETotal.setBold(true);
			cTotal.addElement(tETotal, Qt::AlignRight);
		}
		else
		{
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
		}

		int row = 1;
		int noOfPcs = 0, noOfItems = 0;
		while (querySale.next())
		{
			QString stockId = querySale.value("stock_id").toString();
			QString discount = QString::number(querySale.value("discount").toDouble(), 'f', 2);
			QString qty = querySale.value("quantity").toString();
			noOfPcs += qty.toInt();
			noOfItems++;
			QString subTotal = QString::number(querySale.value("total").toDouble(), 'f', 2);
			QString itemName = "";
			QString unitPrice = "";
			QString itemCode = "";

			//get the item name from the item table
			QString qItemStr("SELECT it.item_code, it.item_name , st.selling_price FROM stock st JOIN item it ON st.item_id = it.item_id AND st.stock_id = " + stockId);
			QSqlQuery queryItem(qItemStr);
			if (queryItem.next())
			{
				itemName = queryItem.value("item_name").toString();
				unitPrice = QString::number(querySale.value("item_price").toDouble(), 'f', 2);
				itemCode = queryItem.value("item_code").toString();
			}
			//columns (item_code, Description, UnitPrice, Discount, Qty, Sub Total)
			printRow(tableElement, row, 0, itemCode);
			printRow(tableElement, row, 1, itemName);
			printRow(tableElement, row, 2, unitPrice, Qt::AlignRight);
			if (secondDisplayOn)
			{
				printRow(tableElement, row, 3, qty, Qt::AlignRight);
				printRow(tableElement, row, 4, subTotal, Qt::AlignRight);
			}
			else
			{
				printRow(tableElement, row, 3, discount, Qt::AlignRight);
				printRow(tableElement, row, 4, qty, Qt::AlignRight);
				printRow(tableElement, row, 5, subTotal, Qt::AlignRight);
			}
			row++;
		}

		{
			KDReports::Cell& emptyCell = tableElement.cell(row, 0);
			KDReports::HtmlElement htmlElem;
			QString html("<div><hr/></div>");
			htmlElem.setHtml(html);
			emptyCell.setColumnSpan(6);
			emptyCell.addElement(htmlElem);
			row++;
		}

		if (customerId == "-1")
		{
			QString customer = "Customer Id";

			KDReports::Cell& billToCell = tableElement.cell(row, 0);
			KDReports::TextElement billTo(customer);
			billTo.setPointSize(11);
			billToCell.addElement(billTo, Qt::AlignLeft);
		}
		else
		{
			QString customer = "Customer Id : ";
			QSqlQuery q("SELECT * FROM customer WHERE customer_id = " + customerId);

			KDReports::Cell& billToCell = tableElement.cell(row, 0);
			KDReports::TextElement billTo(customer);
			billTo.setPointSize(10);
			billToCell.addElement(billTo, Qt::AlignLeft);
			if (q.next())
			{
				KDReports::Cell& cell = tableElement.cell(row, 1);
				KDReports::TextElement te(q.value("customer_id").toString());
				te.setPointSize(10);
				te.setBold(false);
				cell.addElement(te, Qt::AlignLeft);
			}
		}

		if (secondDisplayOn)
		{
			KDReports::Cell& payableTextC = tableElement.cell(row, 3);
			KDReports::TextElement payableTxt("Total ");
			payableTxt.setPointSize(11);
			payableTxt.setBold(true);
			payableTextC.addElement(payableTxt, Qt::AlignRight);

			KDReports::Cell& payableCell = tableElement.cell(row, 4);
			KDReports::TextElement payableValue(QString::number(totalPayingAmount, 'f', 2));
			payableValue.setPointSize(11);
			payableValue.setBold(true);
			payableCell.addElement(payableValue, Qt::AlignRight);
		}
		else
		{

			KDReports::Cell& payableTextC = tableElement.cell(row, 4);
			KDReports::TextElement payableTxt("Total ");
			payableTxt.setPointSize(11);
			payableTxt.setBold(true);
			payableTextC.addElement(payableTxt, Qt::AlignRight);

			KDReports::Cell& payableCell = tableElement.cell(row, 5);
			KDReports::TextElement payableValue(QString::number(totalPayingAmount, 'f', 2));
			payableValue.setPointSize(11);
			payableValue.setBold(true);
			payableCell.addElement(payableValue, Qt::AlignRight);
		}

		row++;

		QString prevOutstandingText = "Prev. Outstanding : ";
		double totalOutstanding = getTotalOutstanding(customerId);
		double billOutstanding = getOutstandingForBill(billId);
		double prevOutstanding = totalOutstanding - billOutstanding;
		{
			KDReports::Cell& cell = tableElement.cell(row, 0);
			KDReports::TextElement te("Prev. Due");
			te.setPointSize(10);
			te.setBold(false);
			cell.addElement(te, Qt::AlignLeft);
		}
		{
			KDReports::Cell& cell = tableElement.cell(row, 1);
			KDReports::TextElement te(QString::number(prevOutstanding, 'f', 2));
			te.setPointSize(10);
			te.setBold(false);
			cell.addElement(te, Qt::AlignLeft);
		}



		if (secondDisplayOn)
		{
			KDReports::Cell& countText = tableElement.cell(row, 3);
			KDReports::TextElement noOfItemsTxt("# of Items ");
			noOfItemsTxt.setPointSize(11);
			noOfItemsTxt.setBold(true);
			countText.addElement(noOfItemsTxt, Qt::AlignRight);

			KDReports::Cell& countItemCell = tableElement.cell(row, 4);
			KDReports::TextElement itemCountValue(QString::number(noOfItems));
			itemCountValue.setPointSize(11);
			itemCountValue.setBold(true);
			countItemCell.addElement(itemCountValue, Qt::AlignRight);
		}
		else
		{
			KDReports::Cell& countText = tableElement.cell(row, 4);
			KDReports::TextElement noOfItemsTxt("# of Items ");
			noOfItemsTxt.setPointSize(11);
			noOfItemsTxt.setBold(true);
			countText.addElement(noOfItemsTxt, Qt::AlignRight);

			KDReports::Cell& countItemCell = tableElement.cell(row, 5);
			KDReports::TextElement itemCountValue(QString::number(noOfItems));
			itemCountValue.setPointSize(11);
			itemCountValue.setBold(true);
			countItemCell.addElement(itemCountValue, Qt::AlignRight);
		}

		row++;
		QString outstandingText = "Total Outstanding : ";
		outstandingText.append(QString::number(totalOutstanding, 'f', 2));
		{
			KDReports::Cell& cell = tableElement.cell(row, 0);
			KDReports::TextElement te("Total Due");
			te.setPointSize(10);
			te.setBold(false);
			cell.addElement(te, Qt::AlignLeft);
		}

		{
			KDReports::Cell& cell = tableElement.cell(row, 1);
			KDReports::TextElement te(QString::number(totalOutstanding, 'f', 2));
			te.setPointSize(10);
			te.setBold(false);
			cell.addElement(te, Qt::AlignLeft);
		}


		if (secondDisplayOn)
		{
			KDReports::Cell& pcsText = tableElement.cell(row, 3);
			KDReports::TextElement noOfPcsTxt("# of Pieces ");
			noOfPcsTxt.setPointSize(11);
			noOfPcsTxt.setBold(true);
			pcsText.addElement(noOfPcsTxt, Qt::AlignRight);

			KDReports::Cell& pcsItemCell = tableElement.cell(row, 4);
			KDReports::TextElement itemPcsValue(QString::number(noOfPcs));
			itemPcsValue.setPointSize(11);
			itemPcsValue.setBold(true);
			pcsItemCell.addElement(itemPcsValue, Qt::AlignRight);
		}
		else
		{
			KDReports::Cell& pcsText = tableElement.cell(row, 4);
			KDReports::TextElement noOfPcsTxt("# of Pieces ");
			noOfPcsTxt.setPointSize(11);
			noOfPcsTxt.setBold(true);
			pcsText.addElement(noOfPcsTxt, Qt::AlignRight);

			KDReports::Cell& pcsItemCell = tableElement.cell(row, 5);
			KDReports::TextElement itemPcsValue(QString::number(noOfPcs));
			itemPcsValue.setPointSize(11);
			itemPcsValue.setBold(true);
			pcsItemCell.addElement(itemPcsValue, Qt::AlignRight);

		}

		report.addElement(tableElement);
		report.addVerticalSpacing(1);


		if (!secondDisplayOn)
		{
			KDReports::TableElement paymentSummaryElement;
			paymentSummaryElement.setHeaderRowCount(payamentSummaryTableInfo.size());
			paymentSummaryElement.setHeaderColumnCount(6);
			paymentSummaryElement.setBorder(1);
			paymentSummaryElement.setWidth(60, KDReports::Percent);
			int pointSizeForPayement = 7;
			{
				KDReports::Cell& cell = paymentSummaryElement.cell(0, 0);
				KDReports::TextElement textElm("Type");
				textElm.setPointSize(pointSizeForPayement);
				textElm.setBold(true);
				cell.addElement(textElm, Qt::AlignCenter);
			}
			{
				KDReports::Cell& cell = paymentSummaryElement.cell(0, 1);
				KDReports::TextElement textElm("Net Amount");
				textElm.setPointSize(pointSizeForPayement);
				textElm.setBold(true);
				cell.addElement(textElm, Qt::AlignCenter);
			}
			{
				KDReports::Cell& cell = paymentSummaryElement.cell(0, 2);
				KDReports::TextElement textElm("Interest");
				textElm.setPointSize(pointSizeForPayement);
				textElm.setBold(true);
				cell.addElement(textElm, Qt::AlignCenter);
			}
			{
				KDReports::Cell& cell = paymentSummaryElement.cell(0, 3);
				KDReports::TextElement textElm("Line Total");
				textElm.setPointSize(pointSizeForPayement);
				textElm.setBold(true);
				cell.addElement(textElm, Qt::AlignCenter);
			}
			{
				KDReports::Cell& cell = paymentSummaryElement.cell(0, 4);
				KDReports::TextElement textElm("Cheque/Card No");
				textElm.setPointSize(pointSizeForPayement);
				textElm.setBold(true);
				cell.addElement(textElm, Qt::AlignCenter);
			}
			{
				KDReports::Cell& cell = paymentSummaryElement.cell(0, 5);
				KDReports::TextElement textElm("Payment Date");
				textElm.setPointSize(pointSizeForPayement);
				textElm.setBold(true);
				cell.addElement(textElm, Qt::AlignCenter);
			}
			int count = 1;
			for (std::shared_ptr<PaymentSummaryElement> pse : payamentSummaryTableInfo)
			{
				{
					KDReports::Cell& cell = paymentSummaryElement.cell(count, 0);
					KDReports::TextElement textElm(pse->type);
					textElm.setPointSize(pointSizeForPayement);
					textElm.setBold(false);
					cell.addElement(textElm, Qt::AlignLeft);
				}
				{
				KDReports::Cell& cell = paymentSummaryElement.cell(count, 1);
				KDReports::TextElement textElm(pse->netAmount);
				textElm.setPointSize(pointSizeForPayement);
				textElm.setBold(false);
				cell.addElement(textElm, Qt::AlignRight);
			}
				{
					KDReports::Cell& cell = paymentSummaryElement.cell(count, 2);
					KDReports::TextElement textElm(pse->interest);
					textElm.setPointSize(pointSizeForPayement);
					textElm.setBold(false);
					cell.addElement(textElm, Qt::AlignRight);
				}
				{
					KDReports::Cell& cell = paymentSummaryElement.cell(count, 3);
					KDReports::TextElement textElm(pse->amount);
					textElm.setPointSize(pointSizeForPayement);
					textElm.setBold(false);
					cell.addElement(textElm, Qt::AlignRight);
				}
				{
					KDReports::Cell& cell = paymentSummaryElement.cell(count, 4);
					KDReports::TextElement textElm(pse->no);
					textElm.setPointSize(pointSizeForPayement);
					textElm.setBold(false);
					cell.addElement(textElm, Qt::AlignLeft);
				}
				{
					KDReports::Cell& cell = paymentSummaryElement.cell(count, 5);
					KDReports::TextElement textElm(pse->date);
					textElm.setPointSize(pointSizeForPayement);
					textElm.setBold(false);
					cell.addElement(textElm, Qt::AlignLeft);
				}
				count++;
			}
			report.addElement(paymentSummaryElement);
		}

		report.addVerticalSpacing(1);

		KDReports::TextElement customerInfo2("Thank You!");
		customerInfo2.setPointSize(11);
		report.addElement(customerInfo2, Qt::AlignCenter);

		QPrinter printer;
		printer.setPaperSize(QPrinter::Custom);

		printer.setFullPage(false);
		printer.setOrientation(QPrinter::Portrait);

		//preview start
		QPrintPreviewDialog *dialog = new QPrintPreviewDialog(&printer, this);
		QObject::connect(dialog, SIGNAL(paintRequested(QPrinter*)), this, SLOT(slotPrint(QPrinter*)));
		dialog->setWindowTitle(tr("Print Document"));
		ES::MainWindowHolder::instance()->getMainWindow()->setCentralWidget(dialog);
		dialog->exec();
		//preview end

		KDReports::Header& header2 = report.header(KDReports::FirstPage);

		QString titleStr = ES::Session::getInstance()->getBillTitle();
		KDReports::TextElement titleElement(titleStr);
		titleElement.setPointSize(14);
		titleElement.setBold(true);
		header2.addElement(titleElement, Qt::AlignCenter);

		QString addressStr = ES::Session::getInstance()->getBillAddress();
		KDReports::TextElement addressElement(addressStr);
		addressElement.setPointSize(10);
		addressElement.setBold(false);
		header2.addElement(addressElement, Qt::AlignCenter);

		QString phoneStr = ES::Session::getInstance()->getBillPhone();
		KDReports::TextElement telElement(phoneStr);
		telElement.setPointSize(10);
		telElement.setBold(false);
		header2.addElement(telElement, Qt::AlignCenter);

		QString emailStr = ES::Session::getInstance()->getBillEmail();
		if (emailStr != "")
		{
			KDReports::TextElement emailElement(emailStr);
			emailElement.setPointSize(10);
			emailElement.setBold(false);
			header2.addElement(emailElement, Qt::AlignCenter);
		}

		KDReports::Header& header1 = report.header(KDReports::AllPages);

		KDReports::TextElement billIdHead("Bill No : " + QString::number(billId));
		billIdHead.setPointSize(11);
		billIdHead.setBold(true);
		header1.addElement(billIdHead);

		report.setMargins(10, 15, 10, 15);

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

void ESCurrentBills::slotInvisible(QString billId)
{
	if (ES::Utility::verifyUsingMessageBox(this, "EStore", "Do you really want hide this ?"))
	{
		QString q("UPDATE bill SET visible = 0 WHERE bill_id = " + billId);
		QSqlQuery query;
		if (query.exec(q))
		{
			QMessageBox mbox;
			mbox.setIcon(QMessageBox::Information);
			mbox.setText("Operation is success");
			mbox.exec();
		}
	}
}

void ESCurrentBills::slotPrev()
{
	if (m_nextCounter == 1)
	{
		ui.prevBtn->setDisabled(true);
	}
	if (m_nextCounter > 0)
	{
		m_nextCounter--;
		m_startingLimit -= m_pageOffset;
		ui.nextBtn->setEnabled(true);
	}
	slotSearch();
}

void ESCurrentBills::slotNext()
{
	if (m_nextCounter < m_maxNextCount)
	{
		m_nextCounter++;
		ui.prevBtn->setEnabled(true);
		m_startingLimit += m_pageOffset;
	}
	slotSearch();
}

float ESCurrentBills::getTotalOutstanding(QString customerId)
{
	float totalAmount;
	QString query;
	query.append("SELECT * FROM customer_outstanding WHERE customer_id = ");
	query.append(customerId);
	query.append(" AND settled = 0");

	QSqlQuery q(query);
	while (q.next())
	{
		QString paymentId = q.value("payment_id").toString();
		QSqlQuery qry("SELECT * FROM payment WHERE payment_id = " + paymentId);
		QString pm = q.value("payment_method").toString();
		float interest = 0;
		if (pm == "CREDIT")
		{
			QSqlQuery qq("SELECT * FROM credit WHERE credit_id = " + q.value("table_id").toString());
			if (qq.next())
			{
				interest = qq.value("interest").toFloat();
				float amount = qq.value("amount").toFloat();
				totalAmount += (amount * (100 + interest) / 100);
			}
		}
		else if (pm == "CHEQUE")
		{
			QSqlQuery qq("SELECT * FROM cheque WHERE cheque_id = " + q.value("table_id").toString());
			if (qq.next())
			{
				interest = qq.value("interest").toFloat();
				float amount = qq.value("amount").toFloat();
				totalAmount += (amount * (100 + interest) / 100);
			}
		}
	}
	return totalAmount;
}

float ESCurrentBills::getOutstandingForBill(int billId)
{
	float totalOutstanding = 0;
	QSqlQuery queryPayment("SELECT * FROM payment WHERE bill_id = " + QString::number(billId) + " AND valid = 1");
	while (queryPayment.next())
	{
		QString pId = queryPayment.value("payment_id").toString();
		QString type = queryPayment.value("payment_type").toString();
		if (type == "CHEQUE")
		{
			QSqlQuery queryCheque("SELECT * FROM cheque WHERE payment_id = " + pId);
			while (queryCheque.next())
			{
				float amount = queryCheque.value("amount").toFloat();
				float interest = queryCheque.value("interest").toFloat();
				totalOutstanding += (amount * (100 + interest) / 100);
			}
		}
		else if (type == "CREDIT")
		{
			QSqlQuery queryCheque("SELECT * FROM credit WHERE payment_id = " + pId);
			while (queryCheque.next())
			{
				float amount = queryCheque.value("amount").toFloat();
				float interest = queryCheque.value("interest").toFloat();
				totalOutstanding += (amount * (100 + interest) / 100);
			}
		}
	}
	return totalOutstanding;
}
