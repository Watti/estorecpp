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

OverallSalesSummary::OverallSalesSummary(QWidget *parent /*= 0*/) : QWidget(parent)
{
	ui.setupUi(this);
	m_generateReportSignalMapper = new QSignalMapper(this);
	ui.fromDate->setDate(QDate::currentDate());
	ui.toDate->setDate(QDate::currentDate().addDays(1));

	QStringList headerLabels;
	headerLabels.append("CASH");
	headerLabels.append("CREDIT");
	headerLabels.append("CHEQUE");
	headerLabels.append("CARD");
	headerLabels.append("RETURN");
	headerLabels.append("PETTY CASH INCOME");
	headerLabels.append("PETTY CASH EXPENSES");
	headerLabels.append("Actions");

	QFont font = this->font();
	font.setBold(true);
	font.setPointSize(12);

	ui.tableWidgetByUser->setHorizontalHeaderLabels(headerLabels);
	ui.tableWidgetByUser->horizontalHeader()->setStretchLastSection(true);
	ui.tableWidgetByUser->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tableWidgetByUser->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableWidgetByUser->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableWidgetByUser->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.tableWidgetByUser->verticalHeader()->setMinimumWidth(200);
	QObject::connect(ui.fromDate, SIGNAL(dateChanged(const QDate &)), this, SLOT(slotSearch()));
	QObject::connect(ui.toDate, SIGNAL(dateChanged(const QDate &)), this, SLOT(slotSearch()));
	slotSearch();
}

OverallSalesSummary::~OverallSalesSummary()
{

}

void OverallSalesSummary::slotPrint(QPrinter* printer)
{

}

void OverallSalesSummary::slotSearch()
{
	while (ui.tableWidgetByUser->rowCount() > 0)
	{
		ui.tableWidgetByUser->removeRow(0);
	}

	QString stardDateStr = ui.fromDate->date().toString("yyyy-MM-dd");
	QString endDateStr = ui.toDate->date().toString("yyyy-MM-dd");

	double cashSales = 0, creditSales = 0, chequeSales = 0, cardSales = 0;
	QSqlQuery userQry("SELECT * FROM user JOIN usertype ON user.usertype_id = usertype.usertype_id WHERE user.active = 1");
	while (userQry.next())
	{
		QString uId = userQry.value("user_id").toString();
		QString uName = userQry.value("display_name").toString();
		QSqlQuery queryUserType("SELECT * FROM user JOIN usertype ON user.usertype_id = usertype.usertype_id WHERE user.user_id = " + uId + " AND usertype.usertype_name <> 'DEV'");
		if (queryUserType.next())
		{

			int row = ui.tableWidgetByUser->rowCount();
			ui.tableWidgetByUser->insertRow(row);

			QTableWidgetItem* nameItem = new QTableWidgetItem(uName);
			ui.tableWidgetByUser->setVerticalHeaderItem(row, nameItem);

			QSqlQuery userBillQry("SELECT * FROM bill WHERE status = 1 AND bill.user_id = " + uId + " AND  DATE(date) BETWEEN '" + stardDateStr + "' AND '" + endDateStr + "'");
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

			QSqlQuery queryReturn("SELECT SUM(return_total) as rTotal FROM return_item WHERE user_id=" + uId + " AND  DATE(date) BETWEEN '" + stardDateStr + "' AND '" + endDateStr + "'");
			if (queryReturn.next())
			{
				double returnTotal = queryReturn.value("rTotal").toDouble();
				QTableWidgetItem *cardReturnWidget = new QTableWidgetItem(QString::number(returnTotal, 'f', 2));
				cardReturnWidget->setTextAlignment(Qt::AlignRight);
				ui.tableWidgetByUser->setItem(row, 4, cardReturnWidget);
			}

			double income = 0, expense = 0;
			QSqlQuery queryPettyCash("SELECT * FROM petty_cash WHERE user_id = " + uId + " AND  DATE(date) BETWEEN '" + stardDateStr + "' AND '" + endDateStr + "'");
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

			QWidget* base = new QWidget(ui.tableWidgetByUser);

			QPushButton* detailBtn = new QPushButton(base);
			detailBtn->setIcon(QIcon("icons/pdf.png"));
			detailBtn->setIconSize(QSize(24, 24));
			detailBtn->setMaximumWidth(100);

			m_generateReportSignalMapper->setMapping(detailBtn, uId);
			QObject::connect(detailBtn, SIGNAL(clicked()), m_generateReportSignalMapper, SLOT(map()));

			QHBoxLayout *layout = new QHBoxLayout;
			layout->setContentsMargins(0, 0, 0, 0);
			layout->addWidget(detailBtn);
			layout->insertStretch(2);
			base->setLayout(layout);
			ui.tableWidgetByUser->setCellWidget(row, 7, base);
			base->show();

		}
	}
}

void OverallSalesSummary::printRow(KDReports::TableElement& tableElement, int row, int col, QString elementStr, Qt::AlignmentFlag alignment /*= Qt::AlignLeft*/)
{

}

Ui::OverallSalesSummary& OverallSalesSummary::getUI()
{
	return ui;
}
