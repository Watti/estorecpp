#include "esoveralllsalessummary.h"
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
#include <iostream>
#include <fstream>
#include "QMessageBox"

OverallSalesSummary::OverallSalesSummary(QWidget *parent /*= 0*/) : QWidget(parent)
{
	ui.setupUi(this);
	m_generateReportSignalMapper = new QSignalMapper(this);
	ui.fromDate->setDate(QDate::currentDate());
	ui.toDate->setDate(QDate::currentDate().addDays(1));
	ui.pdf->hide();

	QStringList headerLabels;
	headerLabels.append("Sales Cash Total");
	headerLabels.append("Credit");
	headerLabels.append("Sales Cheque Total");
	headerLabels.append("Card");
	headerLabels.append("Return");
	headerLabels.append("P/C Income");
	headerLabels.append("P/C Expenses");
	headerLabels.append("Outstanding Collection Cash");
	headerLabels.append("Outstanding Collection Cheque");
	headerLabels.append("Actions");

	QFont font = this->font();
	font.setBold(true);
	font.setPointSize(12);

	ui.tableWidgetByUser->setHorizontalHeaderLabels(headerLabels);
	ui.tableWidgetByUser->horizontalHeader()->setStretchLastSection(true);
	ui.tableWidgetByUser->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	ui.tableWidgetByUser->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableWidgetByUser->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableWidgetByUser->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.tableWidgetByUser->verticalHeader()->setMinimumWidth(200);

	QStringList headerLabels2;
	headerLabels2.append("Cash");
	headerLabels2.append("Credit");
	headerLabels2.append("Cheque");
	headerLabels2.append("Card");
	headerLabels2.append("Return");
	headerLabels2.append("P/C Income");
	headerLabels2.append("P/C Expenses");
	headerLabels2.append("Outstanding Collection Cash");
	headerLabels2.append("Outstanding Collection Cheque");

	ui.tableWidgetTotal->setHorizontalHeaderLabels(headerLabels2);
	ui.tableWidgetTotal->horizontalHeader()->setStretchLastSection(true);
	ui.tableWidgetTotal->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	ui.tableWidgetTotal->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableWidgetTotal->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableWidgetTotal->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.tableWidgetTotal->verticalHeader()->setMinimumWidth(200);

	QObject::connect(ui.generateButton, SIGNAL(clicked()), this, SLOT(slotGenerateReport()));
	QObject::connect(ui.fromDate, SIGNAL(dateChanged(const QDate &)), this, SLOT(slotSearch()));
	QObject::connect(ui.toDate, SIGNAL(dateChanged(const QDate &)), this, SLOT(slotSearch()));
	QObject::connect(m_generateReportSignalMapper, SIGNAL(mapped(QString)), this, SLOT(slotGenerateReportForGivenUser(QString)));
	//slotSearch();
}

OverallSalesSummary::~OverallSalesSummary()
{

}

void OverallSalesSummary::slotPrint(QPrinter* printer)
{
	report.print(printer);
	this->close();
}

void OverallSalesSummary::slotSearch()
{
	while (ui.tableWidgetByUser->rowCount() > 0)
	{
		ui.tableWidgetByUser->removeRow(0);
	}

	QString stardDateStr = ui.fromDate->date().toString("yyyy-MM-dd");
	QString endDateStr = ui.toDate->date().toString("yyyy-MM-dd");

	QString fromDateStr = stardDateStr;
	int dayscount = ui.fromDate->date().daysTo(QDate::currentDate());
	bool hasPermission = (ES::Session::getInstance()->getUser()->getType() == ES::User::SENIOR_MANAGER ||
		ES::Session::getInstance()->getUser()->getType() == ES::User::DEV);
	int maxBackDays = ES::Session::getInstance()->getMaximumDaysToShowRecords();
	if (ES::Session::getInstance()->isEnableTaxSupport() && !hasPermission && dayscount > maxBackDays)
	{
		maxBackDays = maxBackDays*-1;
		QString maxBackDateStr = QDate::currentDate().addDays(maxBackDays).toString("yyyy-MM-dd");
		fromDateStr = maxBackDateStr;
	}

	double cashSales = 0, creditSales = 0, chequeSales = 0, cardSales = 0;
	QSqlQuery userQry("SELECT * FROM user JOIN usertype ON user.usertype_id = usertype.usertype_id WHERE user.active = 1");
	while (userQry.next())
	{
		cashSales = 0, creditSales = 0, chequeSales = 0, cardSales = 0;
		QString uId = userQry.value("user_id").toString();
		QString uName = userQry.value("display_name").toString();
		QSqlQuery queryUserType("SELECT * FROM user JOIN usertype ON user.usertype_id = usertype.usertype_id WHERE user.user_id = " + uId + " AND usertype.usertype_name <> 'DEV'");
		if (queryUserType.next())
		{

			int row = ui.tableWidgetByUser->rowCount();
			ui.tableWidgetByUser->insertRow(row);

			QTableWidgetItem* nameItem = new QTableWidgetItem(uName);
			ui.tableWidgetByUser->setVerticalHeaderItem(row, nameItem);
			QString userBillQryStr;
			if (ES::Session::getInstance()->getUser()->getType() == ES::User::SENIOR_MANAGER ||
				ES::Session::getInstance()->getUser()->getType() == ES::User::DEV)
			{
				userBillQryStr = "SELECT * FROM bill WHERE status = 1 AND bill.user_id = " + uId + " AND  DATE(date) BETWEEN '" + fromDateStr + "' AND '" + endDateStr + "'";
			}
			else
			{
				userBillQryStr = "SELECT * FROM bill WHERE status = 1 AND visible = 1 AND user_id = " + uId + " AND  DATE(date) BETWEEN '" + fromDateStr + "' AND '" + endDateStr + "'";
			}
			QSqlQuery userBillQry(userBillQryStr);
			while (userBillQry.next())
			{
				QSqlQuery paymentQry("SELECT * FROM payment WHERE valid = 1 AND bill_id = " + userBillQry.value("bill_id").toString());
				while (paymentQry.next())
				{
					QString paymentType = paymentQry.value("payment_type").toString();
					QString paymentId = paymentQry.value("payment_id").toString();
					double tot = paymentQry.value("total_amount").toDouble();

					if (paymentType == "CASH")
					{
						cashSales += tot;
					}
					else if (paymentType == "CREDIT")
					{
						QSqlQuery creditSaleQry("SELECT * FROM credit WHERE payment_id = " + paymentId);
						while (creditSaleQry.next())
						{
							double amount = creditSaleQry.value("amount").toDouble();
							double interest = creditSaleQry.value("interest").toDouble();
							amount = (amount * (100 + interest) / 100);
							creditSales += amount;
						}
					}
					else if (paymentType == "CHEQUE")
					{
						QSqlQuery chequeSaleQry("SELECT * FROM cheque WHERE payment_id = " + paymentId);
						while (chequeSaleQry.next())
						{
							double amount = chequeSaleQry.value("amount").toDouble();
							double interest = chequeSaleQry.value("interest").toDouble();
							amount = (amount * (100 + interest) / 100);
							chequeSales += amount;
						}
					}
					else if (paymentType == "CARD")
					{
						QSqlQuery cardSaleQry("SELECT * FROM card WHERE payment_id = " + paymentId);
						while (cardSaleQry.next())
						{
							double amount = cardSaleQry.value("amount").toDouble();
							double interest = cardSaleQry.value("interest").toDouble();
							amount = (amount * (100 + interest) / 100);
							cardSales += amount;
						}
					}
					else if (paymentType == "LOYALTY")
					{
					}
				}
			}

			QTableWidgetItem *cashSalesWidget = new QTableWidgetItem(QString::number(cashSales, 'f', 2));
			cashSalesWidget->setTextAlignment(Qt::AlignRight);
			ui.tableWidgetByUser->setItem(row, 0, cashSalesWidget);

			QTableWidgetItem *creditSalesWidget = new QTableWidgetItem(QString::number(creditSales, 'f', 2));
			creditSalesWidget->setTextAlignment(Qt::AlignRight);
			ui.tableWidgetByUser->setItem(row, 1, creditSalesWidget);

			QTableWidgetItem *chequeSalesWidget = new QTableWidgetItem(QString::number(chequeSales, 'f', 2));
			chequeSalesWidget->setTextAlignment(Qt::AlignRight);
			ui.tableWidgetByUser->setItem(row, 2, chequeSalesWidget);

			QTableWidgetItem *cardSalesWidget = new QTableWidgetItem(QString::number(cardSales, 'f', 2));
			cardSalesWidget->setTextAlignment(Qt::AlignRight);
			ui.tableWidgetByUser->setItem(row, 3, cardSalesWidget);

			QSqlQuery queryReturn("SELECT SUM(return_total) as rTotal FROM return_item WHERE user_id=" + uId + " AND  DATE(date) BETWEEN '" + fromDateStr + "' AND '" + endDateStr + "' AND deleted = 0");
			if (queryReturn.next())
			{
				double returnTotal = queryReturn.value("rTotal").toDouble();
				QTableWidgetItem *cardReturnWidget = new QTableWidgetItem(QString::number(returnTotal, 'f', 2));
				cardReturnWidget->setTextAlignment(Qt::AlignRight);
				ui.tableWidgetByUser->setItem(row, 4, cardReturnWidget);
			}
			double income = 0, expense = 0;
			QSqlQuery queryPettyCash("SELECT * FROM petty_cash WHERE user_id = " + uId + " AND  DATE(date) BETWEEN '" + fromDateStr + "' AND '" + endDateStr + "'");
			while (queryPettyCash.next())
			{
				QString uId = queryPettyCash.value("user_id").toString();
				int type = queryPettyCash.value("type").toInt();
				double amount = queryPettyCash.value("amount").toDouble();
				if (type == 1)
				{
					//income
					income += amount;
				}
				else if (type == 0)
				{
					//expense
					expense += amount;
				}
			}
			QTableWidgetItem* incomeWidgetItem = new QTableWidgetItem(QString::number(income, 'f', 2));
			incomeWidgetItem->setTextAlignment(Qt::AlignRight);
			ui.tableWidgetByUser->setItem(row, 5, incomeWidgetItem);

			QTableWidgetItem* expenseWidgetItem = new QTableWidgetItem(QString::number(expense, 'f', 2));
			expenseWidgetItem->setTextAlignment(Qt::AlignRight);
			ui.tableWidgetByUser->setItem(row, 6, expenseWidgetItem);
			// todo :loyalty

			//outstanding settlement related stuff
			QSqlQuery outstandingCashCollectionQry("SELECT * FROM customer_outstanding_settlement WHERE user_id = " + uId + " AND  DATE(date) BETWEEN '" + fromDateStr + "' AND '" + endDateStr + "'");
			double cashSettlementTotal = 0, chequeSettlementTotal = 0;
			while (outstandingCashCollectionQry.next())
			{
				double amount = outstandingCashCollectionQry.value("amount").toDouble();
				int cashFlag = outstandingCashCollectionQry.value("settled_by_cash").toInt();
				if (cashFlag == 1)
				{
					cashSettlementTotal += amount;
				}
				else if (cashFlag == 0)
				{
					//cheque
					chequeSettlementTotal += amount;
				}
			}

			QTableWidgetItem* settlementCashWidgetItem = new QTableWidgetItem(QString::number(cashSettlementTotal, 'f', 2));
			settlementCashWidgetItem->setTextAlignment(Qt::AlignRight);
			ui.tableWidgetByUser->setItem(row, 7, settlementCashWidgetItem);

			QTableWidgetItem* settlementChequeWidgetItem = new QTableWidgetItem(QString::number(chequeSettlementTotal, 'f', 2));
			settlementChequeWidgetItem->setTextAlignment(Qt::AlignRight);
			ui.tableWidgetByUser->setItem(row, 8, settlementChequeWidgetItem);

			QWidget* base = new QWidget(ui.tableWidgetByUser);

			QPushButton* generateReportBtn = new QPushButton(base);
			generateReportBtn->setIcon(QIcon("icons/pdf.png"));
			generateReportBtn->setIconSize(QSize(24, 24));
			generateReportBtn->setMaximumWidth(100);

			m_generateReportSignalMapper->setMapping(generateReportBtn, uId);
			QObject::connect(generateReportBtn, SIGNAL(clicked()), m_generateReportSignalMapper, SLOT(map()));

			QHBoxLayout *layout = new QHBoxLayout;
			layout->setContentsMargins(0, 0, 0, 0);
			layout->addWidget(generateReportBtn);
			layout->insertStretch(2);
			base->setLayout(layout);
			ui.tableWidgetByUser->setCellWidget(row, 9, base);
			base->show();
		}
	}


	while (ui.tableWidgetTotal->rowCount() > 0)
	{
		ui.tableWidgetTotal->removeRow(0);
	}

	int row = ui.tableWidgetTotal->rowCount();
	ui.tableWidgetTotal->insertRow(row);
	ui.tableWidgetTotal->setVerticalHeaderItem(row, new QTableWidgetItem("Total"));
	cashSales = 0, creditSales = 0, chequeSales = 0, cardSales = 0;
	QString qBillQryStr;
	if (ES::Session::getInstance()->getUser()->getType() == ES::User::SENIOR_MANAGER ||
		ES::Session::getInstance()->getUser()->getType() == ES::User::DEV)
	{
		qBillQryStr = "SELECT* FROM bill WHERE deleted = 0 AND status = 1 AND  DATE(date) BETWEEN '" + fromDateStr + "' AND '" + endDateStr + "'";
	}
	else
	{
		qBillQryStr = "SELECT* FROM bill WHERE deleted = 0 AND visible = 1 AND status = 1 AND  DATE(date) BETWEEN '" + fromDateStr + "' AND '" + endDateStr + "'";
	}
	QSqlQuery totalBillQry(qBillQryStr);
	while (totalBillQry.next())
	{
		QSqlQuery queryUserType("SELECT * FROM user JOIN usertype ON user.usertype_id = usertype.usertype_id WHERE user.active = 1 AND user.user_id = " + totalBillQry.value("user_id").toString() + " AND usertype.usertype_name <> 'DEV'");
		if (queryUserType.next())
		{
			QSqlQuery paymentQry("SELECT * FROM payment WHERE valid = 1 AND bill_id = " + totalBillQry.value("bill_id").toString());
			while (paymentQry.next())
			{
				QString paymentType = paymentQry.value("payment_type").toString();
				QString paymentId = paymentQry.value("payment_id").toString();
				double tot = paymentQry.value("total_amount").toDouble();
				if (paymentType == "CASH")
				{
					cashSales += tot;
				}
				else if (paymentType == "CREDIT")
				{
					QSqlQuery creditSaleQry("SELECT * FROM credit WHERE payment_id = " + paymentId);
					while (creditSaleQry.next())
					{
						double amount = creditSaleQry.value("amount").toDouble();
						double interest = creditSaleQry.value("interest").toDouble();
						amount = (amount * (100 + interest) / 100);
						creditSales += amount;
					}
				}
				else if (paymentType == "CHEQUE")
				{
					QSqlQuery chequeSaleQry("SELECT * FROM cheque WHERE payment_id = " + paymentId);
					while (chequeSaleQry.next())
					{
						double amount = chequeSaleQry.value("amount").toDouble();
						double interest = chequeSaleQry.value("interest").toDouble();
						amount = (amount * (100 + interest) / 100);
						chequeSales += amount;
					}
				}
				else if (paymentType == "CARD")
				{
					QSqlQuery cardSaleQry("SELECT * FROM card WHERE payment_id = " + paymentId);
					while (cardSaleQry.next())
					{
						double amount = cardSaleQry.value("amount").toDouble();
						double interest = cardSaleQry.value("interest").toDouble();
						amount = (amount * (100 + interest) / 100);
						cardSales += amount;
					}
				}
				else if (paymentType == "LOYALTY")
				{
				}
			}
		}
	}

	QTableWidgetItem* cashSalesWidget = new QTableWidgetItem(QString::number(cashSales, 'f', 2));
	cashSalesWidget->setTextAlignment(Qt::AlignRight);
	ui.tableWidgetTotal->setItem(row, 0, cashSalesWidget);

	QTableWidgetItem* creditSalesWidget = new QTableWidgetItem(QString::number(creditSales, 'f', 2));
	creditSalesWidget->setTextAlignment(Qt::AlignRight);
	ui.tableWidgetTotal->setItem(row, 1, creditSalesWidget);

	QTableWidgetItem* chequeSalesWidget = new QTableWidgetItem(QString::number(chequeSales, 'f', 2));
	chequeSalesWidget->setTextAlignment(Qt::AlignRight);
	ui.tableWidgetTotal->setItem(row, 2, chequeSalesWidget);

	QTableWidgetItem* cardSalesWidget = new QTableWidgetItem(QString::number(cardSales, 'f', 2));
	cardSalesWidget->setTextAlignment(Qt::AlignRight);
	ui.tableWidgetTotal->setItem(row, 3, cardSalesWidget);

	QSqlQuery queryReturn("SELECT SUM(return_total) as rTotal FROM return_item WHERE deleted=0 AND DATE(date) BETWEEN '" + stardDateStr + "' AND '" + endDateStr + "'");
	double returnTotal = 0;
	if (queryReturn.next())
	{
		// 		QString uId = queryReturn.value("user_id").toString();
		// 		QSqlQuery queryUserType("SELECT * FROM user JOIN usertype ON user.usertype_id = usertype.usertype_id WHERE user.active = 1 AND user.user_id = " + uId + " AND usertype.usertype_name <> 'DEV'");
		// 		if (queryUserType.next())
		// 		{
		returnTotal = queryReturn.value("rTotal").toDouble();
		//		}
	}
	QTableWidgetItem *cardReturnWidget = new QTableWidgetItem(QString::number(returnTotal, 'f', 2));
	cardReturnWidget->setTextAlignment(Qt::AlignRight);
	ui.tableWidgetTotal->setItem(row, 4, cardReturnWidget);

	double income = 0, expense = 0;
	QSqlQuery queryPettyCash("SELECT * FROM petty_cash WHERE DATE(date) BETWEEN '" + stardDateStr + "' AND '" + endDateStr + "'");
	while (queryPettyCash.next())
	{
		QString uId = queryPettyCash.value("user_id").toString();
		QSqlQuery queryUserType("SELECT * FROM user JOIN usertype ON user.usertype_id = usertype.usertype_id WHERE user.active = 1 AND user.user_id = " + uId + " AND usertype.usertype_name <> 'DEV'");
		if (queryUserType.next())
		{
			int type = queryPettyCash.value("type").toInt();
			double amount = queryPettyCash.value("amount").toDouble();
			if (type == 1)
			{
				//income
				income += amount;
			}
			else if (type == 0)
			{
				//expense
				expense += amount;
			}
		}
	}
	QTableWidgetItem* incomeWidgetItem = new QTableWidgetItem(QString::number(income, 'f', 2));
	incomeWidgetItem->setTextAlignment(Qt::AlignRight);
	ui.tableWidgetTotal->setItem(row, 5, incomeWidgetItem);

	QTableWidgetItem* expenseWidgetItem = new QTableWidgetItem(QString::number(expense, 'f', 2));
	expenseWidgetItem->setTextAlignment(Qt::AlignRight);
	ui.tableWidgetTotal->setItem(row, 6, expenseWidgetItem);

	QSqlQuery outstandingCashCollectionQry("SELECT * FROM customer_outstanding_settlement WHERE DATE(date) BETWEEN '" + stardDateStr + "' AND '" + endDateStr + "'");
	double cashSettlementGrandTotal = 0, chequeSettlementGrandTotal = 0;
	while (outstandingCashCollectionQry.next())
	{
		double amount = outstandingCashCollectionQry.value("amount").toDouble();
		int cashFlag = outstandingCashCollectionQry.value("settled_by_cash").toInt();
		if (cashFlag == 1)
		{
			cashSettlementGrandTotal += amount;
		}
		else if (cashFlag == 0)
		{
			//cheque
			chequeSettlementGrandTotal += amount;
		}
	}

	QTableWidgetItem* settlementCashTotalWidgetItem = new QTableWidgetItem(QString::number(cashSettlementGrandTotal, 'f', 2));
	settlementCashTotalWidgetItem->setTextAlignment(Qt::AlignRight);
	ui.tableWidgetTotal->setItem(row, 7, settlementCashTotalWidgetItem);

	QTableWidgetItem* settlementChequeTotalWidgetItem = new QTableWidgetItem(QString::number(chequeSettlementGrandTotal, 'f', 2));
	settlementChequeTotalWidgetItem->setTextAlignment(Qt::AlignRight);
	ui.tableWidgetTotal->setItem(row, 8, settlementChequeTotalWidgetItem);

}

void OverallSalesSummary::printRow(KDReports::TableElement& tableElement, int row, int col, QString elementStr, Qt::AlignmentFlag alignment /*= Qt::AlignLeft*/)
{
	KDReports::Cell& cell = tableElement.cell(row, col);
	KDReports::TextElement te(elementStr);
	te.setPointSize(ES::Session::getInstance()->getBillItemFontSize());
	cell.addElement(te, alignment);
}

Ui::OverallSalesSummary& OverallSalesSummary::getUI()
{
	return ui;
}

void OverallSalesSummary::slotGenerateReportForGivenUser(QString userId)
{
	KDReports::TextElement titleElement("OVERALL COLLECTION SUMMARY");
	titleElement.setPointSize(13);
	titleElement.setBold(true);
	report.addElement(titleElement, Qt::AlignHCenter);

	QString stardDateStr = ui.fromDate->date().toString("yyyy-MM-dd");
	QString endDateStr = ui.toDate->date().toString("yyyy-MM-dd");

	QString fromDateStr = stardDateStr;
	int dayscount = ui.fromDate->date().daysTo(QDate::currentDate());
	bool hasPermission = (ES::Session::getInstance()->getUser()->getType() == ES::User::SENIOR_MANAGER ||
		ES::Session::getInstance()->getUser()->getType() == ES::User::DEV);
	int maxBackDays = ES::Session::getInstance()->getMaximumDaysToShowRecords();
	if (ES::Session::getInstance()->isEnableTaxSupport() && !hasPermission && dayscount > maxBackDays)
	{
		maxBackDays = maxBackDays*-1;
		QString maxBackDateStr = QDate::currentDate().addDays(maxBackDays).toString("yyyy-MM-dd");
		fromDateStr = maxBackDateStr;
	}

	QString dateStr = "Date : ";
	dateStr.append(stardDateStr).append(" - ").append(endDateStr);

	QString userStr = "User : ";

	QSqlQuery queryUser("SELECT * FROM user WHERE user_id = " + userId);
	if (queryUser.next())
	{
		userStr.append(queryUser.value("display_name").toString());
	}

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

	{
		KDReports::Cell& userCell = infoTableElement.cell(0, 0);
		KDReports::TextElement t(userStr);
		t.setPointSize(10);
		userCell.addElement(t, Qt::AlignLeft);
	}

	report.addElement(infoTableElement);
	report.addVerticalSpacing(5);

	KDReports::TableElement tableElement;
	tableElement.setHeaderColumnCount(9);
	tableElement.setBorder(1);
	tableElement.setWidth(100, KDReports::Percent);

	{
		KDReports::Cell& cell = tableElement.cell(0, 0);
		KDReports::TextElement cTextElement("Cash");
		cTextElement.setPointSize(11);
		cTextElement.setBold(true);
		cell.addElement(cTextElement, Qt::AlignCenter);
	}
	{
		KDReports::Cell& cell = tableElement.cell(0, 1);
		KDReports::TextElement cTextElement("Credit");
		cTextElement.setPointSize(11);
		cTextElement.setBold(true);
		cell.addElement(cTextElement, Qt::AlignCenter);
	}
	{
		KDReports::Cell& cell = tableElement.cell(0, 2);
		KDReports::TextElement cTextElement("Cheque");
		cTextElement.setPointSize(11);
		cTextElement.setBold(true);
		cell.addElement(cTextElement, Qt::AlignCenter);
	}
	{
		KDReports::Cell& cell = tableElement.cell(0, 3);
		KDReports::TextElement cTextElement("Card");
		cTextElement.setPointSize(11);
		cTextElement.setBold(true);
		cell.addElement(cTextElement, Qt::AlignCenter);
	}
	{
		KDReports::Cell& cell = tableElement.cell(0, 4);
		KDReports::TextElement cTextElement("Return");
		cTextElement.setPointSize(11);
		cTextElement.setBold(true);
		cell.addElement(cTextElement, Qt::AlignCenter);
	}
	{
		KDReports::Cell& cell = tableElement.cell(0, 5);
		KDReports::TextElement cTextElement("P/C Income");
		cTextElement.setPointSize(11);
		cTextElement.setBold(true);
		cell.addElement(cTextElement, Qt::AlignCenter);
	}
	{
		KDReports::Cell& cell = tableElement.cell(0, 6);
		KDReports::TextElement cTextElement("P/C Expenses");
		cTextElement.setPointSize(11);
		cTextElement.setBold(true);
		cell.addElement(cTextElement, Qt::AlignCenter);
	}
	{
		KDReports::Cell& cell = tableElement.cell(0, 7);
		KDReports::TextElement cTextElement("Outstanding Collection Cash");
		cTextElement.setPointSize(11);
		cTextElement.setBold(true);
		cell.addElement(cTextElement, Qt::AlignCenter);
	}{
		KDReports::Cell& cell = tableElement.cell(0, 8);
		KDReports::TextElement cTextElement("Outstanding Collection Cheque");
		cTextElement.setPointSize(11);
		cTextElement.setBold(true);
		cell.addElement(cTextElement, Qt::AlignCenter);
	}
	int row = 1;
	double cashSales = 0, creditSales = 0, chequeSales = 0, cardSales = 0;
	QString userBillQryStr;
	if (ES::Session::getInstance()->getUser()->getType() == ES::User::SENIOR_MANAGER ||
		ES::Session::getInstance()->getUser()->getType() == ES::User::DEV)
	{
		userBillQryStr = "SELECT * FROM bill WHERE status = 1 AND bill.user_id = " + userId + " AND  DATE(date) BETWEEN '" + fromDateStr + "' AND '" + endDateStr + "'";
	}
	else
	{
		userBillQryStr = "SELECT * FROM bill WHERE status = 1 AND visible = 1 AND bill.user_id = " + userId + " AND  DATE(date) BETWEEN '" + fromDateStr + "' AND '" + endDateStr + "'";
	}
	QSqlQuery userBillQry(userBillQryStr);
	while (userBillQry.next())
	{
		QSqlQuery paymentQry("SELECT * FROM payment WHERE valid = 1 AND bill_id = " + userBillQry.value("bill_id").toString());
		while (paymentQry.next())
		{
			QString paymentType = paymentQry.value("payment_type").toString();
			QString paymentId = paymentQry.value("payment_id").toString();
			double tot = paymentQry.value("total_amount").toDouble();

			if (paymentType == "CASH")
			{
				cashSales += tot;
			}
			else if (paymentType == "CREDIT")
			{
				QSqlQuery creditSaleQry("SELECT * FROM credit WHERE payment_id = " + paymentId);
				while (creditSaleQry.next())
				{
					double amount = creditSaleQry.value("amount").toDouble();
					double interest = creditSaleQry.value("interest").toDouble();
					amount = (amount * (100 + interest) / 100);
					creditSales += amount;
				}
			}
			else if (paymentType == "CHEQUE")
			{
				QSqlQuery chequeSaleQry("SELECT * FROM cheque WHERE payment_id = " + paymentId);
				while (chequeSaleQry.next())
				{
					double amount = chequeSaleQry.value("amount").toDouble();
					double interest = chequeSaleQry.value("interest").toDouble();
					amount = (amount * (100 + interest) / 100);
					chequeSales += amount;
				}
			}
			else if (paymentType == "CARD")
			{
				QSqlQuery cardSaleQry("SELECT * FROM card WHERE payment_id = " + paymentId);
				while (cardSaleQry.next())
				{
					double amount = cardSaleQry.value("amount").toDouble();
					double interest = cardSaleQry.value("interest").toDouble();
					amount = (amount * (100 + interest) / 100);
					cardSales += amount;
				}
			}
			else if (paymentType == "LOYALTY")
			{
			}
		}
	}
	double returnTotal = 0;
	QSqlQuery queryReturn("SELECT SUM(return_total) as rTotal FROM return_item WHERE user_id=" + userId + " AND  DATE(date) BETWEEN '" + fromDateStr + "' AND '" + endDateStr + "' AND deleted =0");
	if (queryReturn.next())
	{
		returnTotal = queryReturn.value("rTotal").toDouble();
		QTableWidgetItem *cardReturnWidget = new QTableWidgetItem(QString::number(returnTotal, 'f', 2));
		cardReturnWidget->setTextAlignment(Qt::AlignRight);
		ui.tableWidgetByUser->setItem(row, 4, cardReturnWidget);
	}

	double income = 0, expense = 0;
	QSqlQuery queryPettyCash("SELECT * FROM petty_cash WHERE user_id = " + userId + " AND  DATE(date) BETWEEN '" + fromDateStr + "' AND '" + endDateStr + "'");
	while (queryPettyCash.next())
	{
		QString uId = queryPettyCash.value("user_id").toString();
		int type = queryPettyCash.value("type").toInt();
		double amount = queryPettyCash.value("amount").toDouble();
		if (type == 1)
		{
			//income
			income += amount;
		}
		else if (type == 0)
		{
			//expense
			expense += amount;
		}
	}

	QSqlQuery outstandingCashCollectionQry("SELECT * FROM customer_outstanding_settlement WHERE user_id = " + userId + " AND  DATE(date) BETWEEN '" + fromDateStr + "' AND '" + endDateStr + "'");
	double cashSettlementGrandTotal = 0, chequeSettlementGrandTotal = 0;
	while (outstandingCashCollectionQry.next())
	{
		double amount = outstandingCashCollectionQry.value("amount").toDouble();
		int cashFlag = outstandingCashCollectionQry.value("settled_by_cash").toInt();
		if (cashFlag == 1)
		{
			cashSettlementGrandTotal += amount;
		}
		else if (cashFlag == 0)
		{
			//cheque
			chequeSettlementGrandTotal += amount;
		}
	}
	printRow(tableElement, row, 0, QString::number(cashSales, 'f', 2));
	printRow(tableElement, row, 1, QString::number(creditSales, 'f', 2));
	printRow(tableElement, row, 2, QString::number(chequeSales, 'f', 2));
	printRow(tableElement, row, 3, QString::number(cardSales, 'f', 2));
	printRow(tableElement, row, 4, QString::number(returnTotal, 'f', 2));
	printRow(tableElement, row, 5, QString::number(income, 'f', 2));
	printRow(tableElement, row, 6, QString::number(expense, 'f', 2));
	printRow(tableElement, row, 7, QString::number(cashSettlementGrandTotal, 'f', 2));
	printRow(tableElement, row, 8, QString::number(chequeSettlementGrandTotal, 'f', 2));
	report.addElement(tableElement);

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

void OverallSalesSummary::slotGenerateReport()
{
	KDReports::TextElement titleElement("OVERALL SUMMARY");
	titleElement.setPointSize(13);
	titleElement.setBold(true);
	report.addElement(titleElement, Qt::AlignHCenter);


	QString stardDateStr = ui.fromDate->date().toString("yyyy-MM-dd");
	QString endDateStr = ui.toDate->date().toString("yyyy-MM-dd");

	QString fromDateStr = stardDateStr;
	int dayscount = ui.fromDate->date().daysTo(QDate::currentDate());
	bool hasPermission = (ES::Session::getInstance()->getUser()->getType() == ES::User::SENIOR_MANAGER ||
		ES::Session::getInstance()->getUser()->getType() == ES::User::DEV);
	int maxBackDays = ES::Session::getInstance()->getMaximumDaysToShowRecords();
	if (ES::Session::getInstance()->isEnableTaxSupport() && !hasPermission && dayscount > maxBackDays)
	{
		maxBackDays = maxBackDays*-1;
		QString maxBackDateStr = QDate::currentDate().addDays(maxBackDays).toString("yyyy-MM-dd");
		fromDateStr = maxBackDateStr;
	}

	QString dateStr = "Date : ";
	dateStr.append(stardDateStr).append(" - ").append(endDateStr);

	std::ofstream stream;
	QString filename = "";
	bool generateCSV = ui.csv->isChecked();
	if (generateCSV)
	{
		QString dateTimeStr = QDateTime::currentDateTime().toString(QLatin1String("yyyyMMdd-hhmmss"));
		QString pathToFile = ES::Session::getInstance()->getReportPath();
		filename = pathToFile.append("\\Overall Summary Report-");
		filename.append(dateTimeStr).append(".csv");
		std::string s(filename.toStdString());
		stream.open(s, std::ios::out | std::ios::app);
		stream << "Overall Summary Report" << "\n";
		stream << "Generated TimeStamp : ," << dateTimeStr.toLatin1().toStdString() << "\n";
		QString sDate = stardDateStr;
		QString eDate = endDateStr;
		QString dateRange = sDate.append(" - ").append(eDate);
		stream << "Cash , Credit, Cheque, Card, Return, Income, Expenses, Outstanding Collections Cash, Outstanding Collection Cheque" << "\n";

	}

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

	report.addElement(infoTableElement);
	report.addVerticalSpacing(5);

	KDReports::TableElement tableElement;
	tableElement.setHeaderColumnCount(9);
	tableElement.setBorder(1);
	tableElement.setWidth(100, KDReports::Percent);

	{
		KDReports::Cell& cell = tableElement.cell(0, 0);
		KDReports::TextElement cTextElement("Cash");
		cTextElement.setPointSize(11);
		cTextElement.setBold(true);
		cell.addElement(cTextElement, Qt::AlignCenter);
	}
	{
		KDReports::Cell& cell = tableElement.cell(0, 1);
		KDReports::TextElement cTextElement("Credit");
		cTextElement.setPointSize(11);
		cTextElement.setBold(true);
		cell.addElement(cTextElement, Qt::AlignCenter);
	}
	{
		KDReports::Cell& cell = tableElement.cell(0, 2);
		KDReports::TextElement cTextElement("Cheque");
		cTextElement.setPointSize(11);
		cTextElement.setBold(true);
		cell.addElement(cTextElement, Qt::AlignCenter);
	}
	{
		KDReports::Cell& cell = tableElement.cell(0, 3);
		KDReports::TextElement cTextElement("Card");
		cTextElement.setPointSize(11);
		cTextElement.setBold(true);
		cell.addElement(cTextElement, Qt::AlignCenter);
	}
	{
		KDReports::Cell& cell = tableElement.cell(0, 4);
		KDReports::TextElement cTextElement("Return");
		cTextElement.setPointSize(11);
		cTextElement.setBold(true);
		cell.addElement(cTextElement, Qt::AlignCenter);
	}
	{
		KDReports::Cell& cell = tableElement.cell(0, 5);
		KDReports::TextElement cTextElement("Income");
		cTextElement.setPointSize(11);
		cTextElement.setBold(true);
		cell.addElement(cTextElement, Qt::AlignCenter);
	}
	{
		KDReports::Cell& cell = tableElement.cell(0, 6);
		KDReports::TextElement cTextElement("Expenses");
		cTextElement.setPointSize(11);
		cTextElement.setBold(true);
		cell.addElement(cTextElement, Qt::AlignCenter);
	}

	{
		KDReports::Cell& cell = tableElement.cell(0, 7);
		KDReports::TextElement cTextElement("Outstanding Collection Cash");
		cTextElement.setPointSize(11);
		cTextElement.setBold(true);
		cell.addElement(cTextElement, Qt::AlignCenter);
	}
	{
		KDReports::Cell& cell = tableElement.cell(0, 8);
		KDReports::TextElement cTextElement("Outstanding Collection Cheque");
		cTextElement.setPointSize(11);
		cTextElement.setBold(true);
		cell.addElement(cTextElement, Qt::AlignCenter);
	}
	int row = 1;
	double cashSales = 0, creditSales = 0, chequeSales = 0, cardSales = 0;
	QString qBillQryStr;
	if (ES::Session::getInstance()->getUser()->getType() == ES::User::SENIOR_MANAGER ||
		ES::Session::getInstance()->getUser()->getType() == ES::User::DEV)
	{
		qBillQryStr = "SELECT* FROM bill WHERE deleted = 0 AND status = 1 AND  DATE(date) BETWEEN '" + fromDateStr + "' AND '" + endDateStr + "'";
	}
	else
	{
		qBillQryStr = "SELECT* FROM bill WHERE deleted = 0 AND visible = 1 AND status = 1 AND  DATE(date) BETWEEN '" + fromDateStr + "' AND '" + endDateStr + "'";
	}
	QSqlQuery totalBillQry(qBillQryStr);
	/*QSqlQuery totalBillQry("SELECT* FROM bill WHERE deleted = 0 AND status = 1 AND  DATE(date) BETWEEN '" + stardDateStr + "' AND '" + endDateStr + "'");*/
	while (totalBillQry.next())
	{
		QSqlQuery queryUserType("SELECT * FROM user JOIN usertype ON user.usertype_id = usertype.usertype_id WHERE user.active = 1 AND user.user_id = " + totalBillQry.value("user_id").toString() + " AND usertype.usertype_name <> 'DEV'");
		if (queryUserType.next())
		{
			QSqlQuery paymentQry("SELECT * FROM payment WHERE valid = 1 AND bill_id = " + totalBillQry.value("bill_id").toString());
			while (paymentQry.next())
			{
				QString paymentType = paymentQry.value("payment_type").toString();
				QString paymentId = paymentQry.value("payment_id").toString();
				double tot = paymentQry.value("total_amount").toDouble();
				if (paymentType == "CASH")
				{
					cashSales += tot;
				}
				else if (paymentType == "CREDIT")
				{
					QSqlQuery creditSaleQry("SELECT * FROM credit WHERE payment_id = " + paymentId);
					while (creditSaleQry.next())
					{
						double amount = creditSaleQry.value("amount").toDouble();
						double interest = creditSaleQry.value("interest").toDouble();
						amount = (amount * (100 + interest) / 100);
						creditSales += amount;
					}
				}
				else if (paymentType == "CHEQUE")
				{
					QSqlQuery chequeSaleQry("SELECT * FROM cheque WHERE payment_id = " + paymentId);
					while (chequeSaleQry.next())
					{
						double amount = chequeSaleQry.value("amount").toDouble();
						double interest = chequeSaleQry.value("interest").toDouble();
						amount = (amount * (100 + interest) / 100);
						chequeSales += amount;
					}
				}
				else if (paymentType == "CARD")
				{
					QSqlQuery cardSaleQry("SELECT * FROM card WHERE payment_id = " + paymentId);
					while (cardSaleQry.next())
					{
						double amount = cardSaleQry.value("amount").toDouble();
						double interest = cardSaleQry.value("interest").toDouble();
						amount = (amount * (100 + interest) / 100);
						cardSales += amount;
					}
				}
				else if (paymentType == "LOYALTY")
				{
				}
			}
		}
	}
	double returnTotal = 0;
	QSqlQuery queryReturn("SELECT SUM(return_total) as rTotal FROM return_item WHERE deleted=0 AND DATE(date) BETWEEN '" + fromDateStr + "' AND '" + endDateStr + "'");
	if (queryReturn.next())
	{
		// 		QString uId = queryReturn.value("user_id").toString();
		// 		QSqlQuery queryUserType("SELECT * FROM user JOIN usertype ON user.usertype_id = usertype.usertype_id WHERE user.active = 1 AND user.user_id = " + uId + " AND usertype.usertype_name <> 'DEV'");
		// 		if (queryUserType.next())
		// 		{
		returnTotal = queryReturn.value("rTotal").toDouble();
		//		}
	}
	double income = 0, expense = 0;
	QSqlQuery queryPettyCash("SELECT * FROM petty_cash WHERE DATE(date) BETWEEN '" + fromDateStr + "' AND '" + endDateStr + "'");
	while (queryPettyCash.next())
	{
		QString uId = queryPettyCash.value("user_id").toString();
		QSqlQuery queryUserType("SELECT * FROM user JOIN usertype ON user.usertype_id = usertype.usertype_id WHERE user.active = 1 AND user.user_id = " + uId + " AND usertype.usertype_name <> 'DEV'");
		if (queryUserType.next())
		{
			int type = queryPettyCash.value("type").toInt();
			double amount = queryPettyCash.value("amount").toDouble();
			if (type == 1)
			{
				//income
				income += amount;
			}
			else if (type == 0)
			{
				//expense
				expense += amount;
			}
		}
	}

	QSqlQuery outstandingCashCollectionQry("SELECT * FROM customer_outstanding_settlement WHERE DATE(date) BETWEEN '" + fromDateStr + "' AND '" + endDateStr + "'");
	double cashSettlementGrandTotal = 0, chequeSettlementGrandTotal = 0;
	while (outstandingCashCollectionQry.next())
	{
		double amount = outstandingCashCollectionQry.value("amount").toDouble();
		int cashFlag = outstandingCashCollectionQry.value("settled_by_cash").toInt();
		if (cashFlag == 1)
		{
			cashSettlementGrandTotal += amount;
		}
		else if (cashFlag == 0)
		{
			//cheque
			chequeSettlementGrandTotal += amount;
		}
	}
	if (generateCSV)
	{
		stream << QString::number(cashSales, 'f', 2).toLatin1().data() << ", " << QString::number(creditSales, 'f', 2).toLatin1().data() <<
			"," << QString::number(chequeSales, 'f', 2).toLatin1().data() << "," << QString::number(cardSales, 'f', 2).toLatin1().data() <<
			", " << QString::number(returnTotal, 'f', 2).toLatin1().data() << "," << QString::number(income, 'f', 2).toLatin1().data() <<
			"," << QString::number(expense, 'f', 2).toLatin1().data() << "," << QString::number(cashSettlementGrandTotal, 'f', 2).toLatin1().data() <<
			"," << QString::number(chequeSettlementGrandTotal, 'f', 2).toLatin1().data();
		stream.close();
		ui.csv->setChecked(false);
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Information);
		mbox.setText(QString("Overall Sales Summary Report has been saved in : ").append(filename));
		mbox.exec();
	}
	else
	{
		printRow(tableElement, row, 0, QString::number(cashSales, 'f', 2));
		printRow(tableElement, row, 1, QString::number(creditSales, 'f', 2));
		printRow(tableElement, row, 2, QString::number(chequeSales, 'f', 2));
		printRow(tableElement, row, 3, QString::number(cardSales, 'f', 2));
		printRow(tableElement, row, 4, QString::number(returnTotal, 'f', 2));
		printRow(tableElement, row, 5, QString::number(income, 'f', 2));
		printRow(tableElement, row, 6, QString::number(expense, 'f', 2));
		printRow(tableElement, row, 7, QString::number(cashSettlementGrandTotal, 'f', 2));
		printRow(tableElement, row, 8, QString::number(chequeSettlementGrandTotal, 'f', 2));

		report.addElement(tableElement);

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
