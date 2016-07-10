#include "essalesummaryreport.h"
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
#include "entities\SaleLineEdit.h"
#include "esmainwindow.h"

ESSalesSummary::ESSalesSummary(QWidget *parent /*= 0*/) : QWidget(parent)
{
	ui.setupUi(this);


	ui.fromDate->setDate(QDate::currentDate());
	ui.toDate->setDate(QDate::currentDate().addDays(1));

	QStringList headerLabels;
	headerLabels.append("CASH");
	headerLabels.append("CREDIT");
	headerLabels.append("CHEQUE");
	headerLabels.append("CARD");
	headerLabels.append("LOYALTY");

	QFont font = this->font();
	font.setBold(true);
	font.setPointSize(12);

	ui.tableWidgetByUser->setHorizontalHeaderLabels(headerLabels);
	ui.tableWidgetByUser->horizontalHeader()->setStretchLastSection(true);
	ui.tableWidgetByUser->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tableWidgetByUser->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableWidgetByUser->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableWidgetByUser->setSelectionMode(QAbstractItemView::SingleSelection);
	//ui.tableWidgetByUser->setFont(font);

	//ui.tableWidgetByUser->verticalHeader()->setFont(font);
	ui.tableWidgetByUser->verticalHeader()->setMinimumWidth(200);

	//////////////////////////////////////////////////////////////////////////

	QStringList headerLabels2;
	headerLabels2.append("CASH");
	headerLabels2.append("CREDIT");
	headerLabels2.append("CHEQUE");
	headerLabels2.append("CARD");
	headerLabels2.append("LOYALTY");

	ui.tableWidgetTotal->setHorizontalHeaderLabels(headerLabels2);
	ui.tableWidgetTotal->horizontalHeader()->setStretchLastSection(true);
	ui.tableWidgetTotal->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tableWidgetTotal->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableWidgetTotal->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableWidgetTotal->setSelectionMode(QAbstractItemView::SingleSelection);
	//ui.tableWidgetTotal->setFont(font);

	//ui.tableWidgetTotal->verticalHeader()->setFont(font);
	ui.tableWidgetTotal->verticalHeader()->setMinimumWidth(200);

	QObject::connect(ui.generateButton, SIGNAL(clicked()), this, SLOT(slotGenerate()));

	//ui.datelbl->setText(QDate::currentDate().toString("yyyy-MM-dd"));

	while (ui.tableWidgetTotal->rowCount() > 0)
	{
		ui.tableWidgetTotal->removeRow(0);
	}

	int row = ui.tableWidgetTotal->rowCount();
	ui.tableWidgetTotal->insertRow(row);
	ui.tableWidgetTotal->setVerticalHeaderItem(row, new QTableWidgetItem("Total"));
	double cashSales = 0, creditSales = 0, chequeSales = 0, cardSales = 0;
	QSqlQuery totalBillQry("SELECT* FROM bill WHERE deleted = 0 AND status = 1 AND  DATE(date) = CURDATE()");
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

	while (ui.tableWidgetByUser->rowCount() > 0)
	{
		ui.tableWidgetByUser->removeRow(0);
	}
	//QSqlQuery userQry("SELECT * FROM user JOIN usertype ON user.usertype_id = usertype.usertype_id WHERE usertype.usertype_name <> 'DEV'  AND user.active = 1");

	QSqlQuery userQry("SELECT * FROM user JOIN usertype ON user.usertype_id = usertype.usertype_id WHERE user.active = 1");
	while (userQry.next())
	{
		QString uId = userQry.value("user_id").toString();
		QString uName = userQry.value("display_name").toString();
		QSqlQuery queryUserType("SELECT * FROM user JOIN usertype ON user.usertype_id = usertype.usertype_id WHERE user.user_id = " + uId + " AND usertype.usertype_name <> 'DEV'");
		if (queryUserType.next())
		{
			cashSales = 0, creditSales = 0, chequeSales = 0, cardSales = 0;

			row = ui.tableWidgetByUser->rowCount();
			ui.tableWidgetByUser->insertRow(row);

			QTableWidgetItem* nameItem = new QTableWidgetItem(uName);
			ui.tableWidgetByUser->setVerticalHeaderItem(row, nameItem);

			//QSqlQuery userSalesQry("SELECT SUM(amount) as total, payment_type FROM payment JOIN bill ON payment.bill_id = bill.bill_id WHERE bill.deleted = 0 AND bill.status = 1 AND DATE(bill.date) = CURDATE() AND bill.user_id = "+uId+"  Group By payment.payment_type");
			QSqlQuery userBillQry("SELECT * FROM bill WHERE status = 1 AND DATE(bill.date) = CURDATE() AND bill.user_id = " + uId);
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
			cashSalesWidget = new QTableWidgetItem(QString::number(cashSales, 'f', 2));
			cashSalesWidget->setTextAlignment(Qt::AlignRight);
			ui.tableWidgetByUser->setItem(row, 0, cashSalesWidget);

			creditSalesWidget = new QTableWidgetItem(QString::number(creditSales, 'f', 2));
			creditSalesWidget->setTextAlignment(Qt::AlignRight);
			ui.tableWidgetByUser->setItem(row, 1, creditSalesWidget);

			chequeSalesWidget = new QTableWidgetItem(QString::number(chequeSales, 'f', 2));
			chequeSalesWidget->setTextAlignment(Qt::AlignRight);
			ui.tableWidgetByUser->setItem(row, 2, chequeSalesWidget);

			cardSalesWidget = new QTableWidgetItem(QString::number(cardSales, 'f', 2));
			cardSalesWidget->setTextAlignment(Qt::AlignRight);
			ui.tableWidgetByUser->setItem(row, 3, cardSalesWidget);
		}
	}
}

void ESSalesSummary::slotPrint(QPrinter* printer)
{
	report.print(printer);
	this->close();
}

void ESSalesSummary::slotGenerate()
{
	bool print = ui.checkBox->isChecked();
	if (print)
	{
		//KDReports::Report report;

		KDReports::TextElement titleElement("SALES SUMMARY");
		titleElement.setPointSize(13);
		titleElement.setBold(true);
		report.addElement(titleElement, Qt::AlignHCenter);


		QString dateStr = "Date : ";
		dateStr.append(QDateTime::currentDateTime().toString("yyyy-MM-dd"));


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
		//tableElement.setHeaderRowCount(5);
		tableElement.setHeaderColumnCount(2);
		tableElement.setBorder(1);
		tableElement.setWidth(100, KDReports::Percent);

		KDReports::Cell& cellPType = tableElement.cell(0, 0);
		KDReports::TextElement tEPType("Payment Type");
		tEPType.setPointSize(11);
		tEPType.setBold(true);
		cellPType.addElement(tEPType, Qt::AlignCenter);

		KDReports::Cell& cTotal = tableElement.cell(0, 1);
		KDReports::TextElement tETotal("Total");
		tETotal.setBold(true);
		tETotal.setPointSize(11);
		cTotal.addElement(tETotal, Qt::AlignCenter);

		int row = 1;
		double cashSales = 0, creditSales = 0, chequeSales = 0, cardSales = 0;
		QSqlQuery totalBillQry("SELECT* FROM bill WHERE deleted = 0 AND status = 1 AND  DATE(date) = CURDATE()");
		while (totalBillQry.next())
		{
			QSqlQuery queryUserType("SELECT * FROM user JOIN usertype ON user.usertype_id = usertype.usertype_id WHERE user.user_id = " + totalBillQry.value("user_id").toString() + " AND usertype.usertype_name <> 'DEV'");
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
		printRow(tableElement, row, 0, "CASH");
		printRow(tableElement, row++, 1, QString::number(cashSales, 'f', 2));
		printRow(tableElement, row, 0, "CREDIT");
		printRow(tableElement, row++, 1, QString::number(creditSales, 'f', 2));
		printRow(tableElement, row, 0, "CHEQUE");
		printRow(tableElement, row++, 1, QString::number(chequeSales, 'f', 2));
		printRow(tableElement, row, 0, "CARD");
		printRow(tableElement, row++, 1, QString::number(cardSales, 'f', 2));

		report.addElement(tableElement);

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

ESSalesSummary::~ESSalesSummary()
{

}

void ESSalesSummary::printRow(KDReports::TableElement& tableElement, int row, int col, QString elementStr, Qt::AlignmentFlag alignment /*= Qt::AlignLeft*/)
{
	KDReports::Cell& cell = tableElement.cell(row, col);
	KDReports::TextElement te(elementStr);
	te.setPointSize(ES::Session::getInstance()->getBillItemFontSize());
	cell.addElement(te, alignment);
}
