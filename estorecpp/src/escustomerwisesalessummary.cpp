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
#include "utility\utility.h"
#include "escustomerwisesalessummary .h"

ESCustomerWiseSalesSummary::ESCustomerWiseSalesSummary(QWidget *parent /*= 0*/) : QWidget(parent), report(NULL)
{
	ui.setupUi(this);
	m_generateReportSignalMapper = new QSignalMapper(this);

	QStringList headerLabels;
	headerLabels.append("Name");
	headerLabels.append("Phone");
	headerLabels.append("Address");
	headerLabels.append("Total Amount");

	QFont font = this->font();
	font.setBold(true);
	font.setPointSize(12);

	ui.tableWidget->setHorizontalHeaderLabels(headerLabels);
	ui.tableWidget->horizontalHeader()->setStretchLastSection(true);
	ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);

	ui.fromDate->setDate(QDate::currentDate());
	ui.toDate->setDate(QDate::currentDate().addDays(1));

	QObject::connect(ui.fromDate, SIGNAL(dateChanged(const QDate &)), this, SLOT(slotSearch()));
	QObject::connect(ui.toDate, SIGNAL(dateChanged(const QDate &)), this, SLOT(slotSearch()));
	QObject::connect(ui.generateButton, SIGNAL(clicked()), this, SLOT(slotGenerateReport()));
}

ESCustomerWiseSalesSummary::~ESCustomerWiseSalesSummary()
{
	delete report;
}

void ESCustomerWiseSalesSummary::slotPrint(QPrinter* printer)
{
	report->print(printer);
	this->close();
}

void ESCustomerWiseSalesSummary::slotSearch()
{
	QString stardDateStr = ui.fromDate->date().toString("yyyy-MM-dd");
	QString endDateStr = ui.toDate->date().toString("yyyy-MM-dd");

	while (ui.tableWidget->rowCount() > 0)
	{
		ui.tableWidget->removeRow(0);
	}
	double grandTotal = 0;
	QSqlQuery customerQry("SELECT * FROM customer WHERE deleted = 0 ORDER BY name");
	while (customerQry.next())
	{
		double cashSales = 0, creditSales = 0, chequeSales = 0, cardSales = 0, customerTotalAmount = 0;
		QString customerId = customerQry.value("customer_id").toString();
		QString cName = customerQry.value("name").toString();
		QString phone = customerQry.value("phone").toString();
		QString address = customerQry.value("address").toString();
		QString comments = customerQry.value("comments").toString();

		QSqlQuery totalBillQry("SELECT* FROM bill WHERE deleted = 0 AND status = 1 AND  DATE(date) BETWEEN '" + stardDateStr + "' AND '" + endDateStr + "' AND customer_id = " + customerId);
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
		customerTotalAmount += cashSales + cardSales + chequeSales + creditSales;
		grandTotal += customerTotalAmount;

		int row = ui.tableWidget->rowCount();
		ui.tableWidget->insertRow(row);


		QTableWidgetItem *nameWidget = new QTableWidgetItem(cName);
		nameWidget->setTextAlignment(Qt::AlignLeft);
		ui.tableWidget->setItem(row, 0, nameWidget);

		QTableWidgetItem *phoneWidget = new QTableWidgetItem(phone);
		phoneWidget->setTextAlignment(Qt::AlignLeft);
		ui.tableWidget->setItem(row, 1, phoneWidget);

		QTableWidgetItem *addressWidget = new QTableWidgetItem(address);
		addressWidget->setTextAlignment(Qt::AlignLeft);
		ui.tableWidget->setItem(row, 2, addressWidget);

		QTableWidgetItem *totalSalesWidget = new QTableWidgetItem(QString::number(customerTotalAmount, 'f', 2));
		totalSalesWidget->setTextAlignment(Qt::AlignRight);
		ui.tableWidget->setItem(row, 3, totalSalesWidget);
	}
	ui.totalLbl->setText(QString::number(grandTotal, 'f', 2));
}

void ESCustomerWiseSalesSummary::printRow(KDReports::TableElement& tableElement, int row, int col, QString elementStr, Qt::AlignmentFlag alignment /*= Qt::AlignLeft*/)
{
	KDReports::Cell& cell = tableElement.cell(row, col);
	KDReports::TextElement te(elementStr);
	te.setPointSize(ES::Session::getInstance()->getBillItemFontSize());
	cell.addElement(te, alignment);
}

Ui::CustomerWiseSalesSummary& ESCustomerWiseSalesSummary::getUI()
{
	return ui;
}


void ESCustomerWiseSalesSummary::slotGenerateReport()
{
	QString stardDateStr = ui.fromDate->date().toString("yyyy-MM-dd");
	QString endDateStr = ui.toDate->date().toString("yyyy-MM-dd");

	report = new KDReports::Report;

	KDReports::TextElement titleElement("Customer Wise Sales Summary Report");
	titleElement.setPointSize(13);
	titleElement.setBold(true);
	report->addElement(titleElement, Qt::AlignHCenter);

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
	tableElement.setHeaderColumnCount(1);
	tableElement.setBorder(1);
	tableElement.setWidth(100, KDReports::Percent);

	{
		KDReports::Cell& cell = tableElement.cell(0, 0);
		KDReports::TextElement cTextElement("Name");
		cTextElement.setPointSize(11);
		cTextElement.setBold(true);
		cell.addElement(cTextElement, Qt::AlignCenter);
	}
	{
		KDReports::Cell& cell = tableElement.cell(0, 1);
		KDReports::TextElement cTextElement("Phone");
		cTextElement.setPointSize(11);
		cTextElement.setBold(true);
		cell.addElement(cTextElement, Qt::AlignCenter);
	}
	{
		KDReports::Cell& cell = tableElement.cell(0, 2);
		KDReports::TextElement cTextElement("Address");
		cTextElement.setPointSize(11);
		cTextElement.setBold(true);
		cell.addElement(cTextElement, Qt::AlignCenter);
	}
	{
		KDReports::Cell& cell = tableElement.cell(0, 3);
		KDReports::TextElement cTextElement("Amount");
		cTextElement.setPointSize(11);
		cTextElement.setBold(true);
		cell.addElement(cTextElement, Qt::AlignCenter);
	}

	QSqlQuery customerQry("SELECT * FROM customer WHERE deleted = 0 ORDER BY name");
	int row = 1;
	double grandTotal = 0;
	while (customerQry.next())
	{
		double cashSales = 0, creditSales = 0, chequeSales = 0, cardSales = 0, customerTotalAmount = 0;
		QString customerId = customerQry.value("customer_id").toString();
		QString cName = customerQry.value("name").toString();
		QString phone = customerQry.value("phone").toString();
		QString address = customerQry.value("address").toString();
		QString comments = customerQry.value("comments").toString();

		QSqlQuery totalBillQry("SELECT* FROM bill WHERE deleted = 0 AND status = 1 AND  DATE(date) BETWEEN '" + stardDateStr + "' AND '" + endDateStr + "' AND customer_id = " + customerId);
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

		customerTotalAmount += cashSales + cardSales + chequeSales + creditSales;
		grandTotal += customerTotalAmount;

		printRow(tableElement, row, 0, cName);
		printRow(tableElement, row, 1, phone);
		printRow(tableElement, row, 2, address);
		printRow(tableElement, row, 3, QString::number(customerTotalAmount, 'f', 2));
		row++;
	}

	printRow(tableElement, row, 2, "Total ");
	printRow(tableElement, row, 3, QString::number(grandTotal, 'f', 2));

	report->addElement(tableElement);

	QPrinter printer;
	printer.setPaperSize(QPrinter::A4);

	printer.setFullPage(false);
	printer.setOrientation(QPrinter::Portrait);

	QPrintPreviewDialog *dialog = new QPrintPreviewDialog(&printer, this);
	QObject::connect(dialog, SIGNAL(paintRequested(QPrinter*)), this, SLOT(slotPrint(QPrinter*)));
	dialog->setWindowTitle(tr("Print Document"));
	ES::MainWindowHolder::instance()->getMainWindow()->setCentralWidget(dialog);
	dialog->exec();

}
