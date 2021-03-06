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
	ui.pdf->hide();
	m_generateUserReportMapper = new QSignalMapper(this);


	ui.fromDate->setDate(QDate::currentDate());
	ui.toDate->setDate(QDate::currentDate().addDays(1));

	QStringList headerLabels;
	headerLabels.append("CASH");
	headerLabels.append("CREDIT");
	headerLabels.append("CHEQUE");
	headerLabels.append("CARD");
	headerLabels.append("LOYALTY");
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
	QObject::connect(ui.fromDate, SIGNAL(dateChanged(const QDate &)), this, SLOT(slotDateChanged()));
	QObject::connect(ui.toDate, SIGNAL(dateChanged(const QDate &)), this, SLOT(slotDateChanged()));
	QObject::connect(m_generateUserReportMapper, SIGNAL(mapped(QString)), this, SLOT(slotGenerateReportForGivenUser(QString)));

	//displayResults();

}

void ESSalesSummary::slotPrint(QPrinter* printer)
{
	report.print(printer);
	this->close();
}

void ESSalesSummary::slotGenerate()
{
	//KDReports::Report report;

	KDReports::TextElement titleElement("SALES SUMMARY");
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
	QString qStr;
	if (ES::Session::getInstance()->getUser()->getType() == ES::User::SENIOR_MANAGER ||
		ES::Session::getInstance()->getUser()->getType() == ES::User::DEV)
	{
		qStr = "SELECT* FROM bill WHERE deleted = 0 AND status = 1 AND  DATE(date) BETWEEN '" + fromDateStr + "' AND '" + endDateStr + "'";
	}
	else
	{
		qStr = "SELECT* FROM bill WHERE deleted = 0 AND status = 1 AND visible = 1 AND  DATE(date) BETWEEN '" + fromDateStr + "' AND '" + endDateStr + "'";
	}

	QSqlQuery totalBillQry(qStr);
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
	printRow(tableElement, row, 0, "CASH");
	printRow(tableElement, row++, 1, QString::number(cashSales, 'f', 2));
	printRow(tableElement, row, 0, "CREDIT");
	printRow(tableElement, row++, 1, QString::number(creditSales, 'f', 2));
	printRow(tableElement, row, 0, "CHEQUE");
	printRow(tableElement, row++, 1, QString::number(chequeSales, 'f', 2));
	printRow(tableElement, row, 0, "CARD");
	printRow(tableElement, row++, 1, QString::number(cardSales, 'f', 2));
	printRow(tableElement, row, 0, "TOTAL");
	double grandTotal = cardSales + chequeSales + creditSales + cashSales;
	printRow(tableElement, row++, 1, QString::number(grandTotal, 'f', 2));
	report.addElement(tableElement);

	QPrinter printer;
	printer.setPaperSize(QPrinter::A4);

	printer.setFullPage(false);
	printer.setOrientation(QPrinter::Portrait);

	if (ui.pdf->isChecked())
	{
		report.exportToFile("./reports/sales_summary_report.pdf");
	}

	QPrintPreviewDialog *dialog = new QPrintPreviewDialog(&printer, this);
	QObject::connect(dialog, SIGNAL(paintRequested(QPrinter*)), this, SLOT(slotPrint(QPrinter*)));
	dialog->setWindowTitle(tr("Print Document"));
	ES::MainWindowHolder::instance()->getMainWindow()->setCentralWidget(dialog);
	dialog->exec();

	//report.print(&printer);
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

void ESSalesSummary::slotDateChanged()
{
	displayResults();
}

void ESSalesSummary::displayResults()
{
	while (ui.tableWidgetTotal->rowCount() > 0)
	{
		ui.tableWidgetTotal->removeRow(0);
	}

	int row = ui.tableWidgetTotal->rowCount();
	ui.tableWidgetTotal->insertRow(row);
	ui.tableWidgetTotal->setVerticalHeaderItem(row, new QTableWidgetItem("Total"));
	double cashSales = 0, creditSales = 0, chequeSales = 0, cardSales = 0;

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
	QString qStr;
	if (ES::Session::getInstance()->getUser()->getType() == ES::User::SENIOR_MANAGER ||
		ES::Session::getInstance()->getUser()->getType() == ES::User::DEV)
	{
		qStr = "SELECT* FROM bill WHERE deleted = 0 AND status = 1 AND  DATE(date) BETWEEN '" + fromDateStr + "' AND '" + endDateStr + "'";
	}
	else
	{
		qStr = "SELECT* FROM bill WHERE deleted = 0 AND status = 1 AND visible = 1 AND  DATE(date) BETWEEN '" + fromDateStr + "' AND '" + endDateStr + "'";
	}

	QSqlQuery totalBillQry(qStr);
	while (totalBillQry.next())
	{
		QSqlQuery queryUserType("SELECT * FROM user JOIN usertype ON user.usertype_id = usertype.usertype_id WHERE user.active = 1 AND user.user_id = " + totalBillQry.value("user_id").toString() + " AND usertype.usertype_name <> 'DEV' AND usertype.usertype_name <> 'SENIOR MANAGER'");
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
		QSqlQuery queryUserType("SELECT * FROM user JOIN usertype ON user.usertype_id = usertype.usertype_id WHERE user.user_id = " + uId + " AND usertype.usertype_name <> 'DEV' AND usertype.usertype_name <> 'SENIOR MANAGER'");
		if (queryUserType.next())
		{
			cashSales = 0, creditSales = 0, chequeSales = 0, cardSales = 0;

			row = ui.tableWidgetByUser->rowCount();
			ui.tableWidgetByUser->insertRow(row);

			QTableWidgetItem* nameItem = new QTableWidgetItem(uName);
			ui.tableWidgetByUser->setVerticalHeaderItem(row, nameItem);
			QString qUserStr;
			if (ES::Session::getInstance()->getUser()->getType() == ES::User::SENIOR_MANAGER ||
				ES::Session::getInstance()->getUser()->getType() == ES::User::DEV)
			{
				qUserStr = "SELECT * FROM bill WHERE status = 1 AND deleted = 0 AND user_id = " + uId + " AND  DATE(date) BETWEEN '" + fromDateStr + "' AND '" + endDateStr + "'";
			}
			else
			{
				qUserStr = "SELECT * FROM bill WHERE status = 1 AND deleted = 0 AND visible = 1 AND user_id = " + uId + " AND  DATE(date) BETWEEN '" + fromDateStr + "' AND '" + endDateStr + "'";
			}

			QSqlQuery userBillQry(qUserStr);
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

			// todo :loyalty

			QWidget* base = new QWidget(ui.tableWidgetByUser);

			QPushButton* detailBtn = new QPushButton(base);
			detailBtn->setIcon(QIcon("icons/pdf.png"));
			detailBtn->setIconSize(QSize(24, 24));
			detailBtn->setMaximumWidth(100);

			m_generateUserReportMapper->setMapping(detailBtn, uId);
			QObject::connect(detailBtn, SIGNAL(clicked()), m_generateUserReportMapper, SLOT(map()));

			QHBoxLayout *layout = new QHBoxLayout;
			layout->setContentsMargins(0, 0, 0, 0);
			layout->addWidget(detailBtn);
			layout->insertStretch(2);
			base->setLayout(layout);
			ui.tableWidgetByUser->setCellWidget(row, 5, base);
			base->show();

		}
	}
}

void ESSalesSummary::slotGenerateReportForGivenUser(QString userId)
{
	KDReports::TextElement titleElement("Sales Summary Report");
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
	tableElement.setHeaderColumnCount(4);
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

	int row = 1;
	double cashSales = 0, creditSales = 0, chequeSales = 0, cardSales = 0;
	QString qStr;
	if (ES::Session::getInstance()->getUser()->getType() == ES::User::SENIOR_MANAGER ||
		ES::Session::getInstance()->getUser()->getType() == ES::User::DEV)
	{
		qStr = "SELECT * FROM bill WHERE status = 1 AND deleted = 0 AND bill.user_id = " + userId + " AND  DATE(date) BETWEEN '" + fromDateStr + "' AND '" + endDateStr + "'";
	}
	else
	{
		qStr = "SELECT * FROM bill WHERE status = 1 AND deleted = 0 AND visible = 1 AND bill.user_id = " + userId + " AND  DATE(date) BETWEEN '" + fromDateStr + "' AND '" + endDateStr + "'";
	}

	QSqlQuery userBillQry(qStr);
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

	printRow(tableElement, row, 0, QString::number(cashSales, 'f', 2));
	printRow(tableElement, row, 1, QString::number(creditSales, 'f', 2));
	printRow(tableElement, row, 2, QString::number(chequeSales, 'f', 2));
	printRow(tableElement, row, 3, QString::number(cardSales, 'f', 2));

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
}
