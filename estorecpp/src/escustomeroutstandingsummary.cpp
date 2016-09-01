#include "escustomeroutstandingsummary.h"
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

ESCustomerOutstandingSummary::ESCustomerOutstandingSummary(QWidget *parent /*= 0*/) : QWidget(parent), report(NULL)
{
	ui.setupUi(this);
	m_generateReportSignalMapper = new QSignalMapper(this);

	QStringList headerLabels;
	headerLabels.append("Name");
	headerLabels.append("Phone");
	headerLabels.append("Address");
	headerLabels.append("Outstanding Amount");

	QFont font = this->font();
	font.setBold(true);
	font.setPointSize(12);

	ui.tableWidget->setHorizontalHeaderLabels(headerLabels);
	ui.tableWidget->horizontalHeader()->setStretchLastSection(true);
	ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);

	QObject::connect(ui.generateButton, SIGNAL(clicked()), this, SLOT(slotGenerateReport()));
}

ESCustomerOutstandingSummary::~ESCustomerOutstandingSummary()
{
	delete report;
}

void ESCustomerOutstandingSummary::slotPrint(QPrinter* printer)
{
	report->print(printer);
	this->close();
}

void ESCustomerOutstandingSummary::slotSearch()
{
	while (ui.tableWidget->rowCount() > 0)
	{
		ui.tableWidget->removeRow(0);
	}
	double totalOutstanding = 0;
	QSqlQuery customerQry("SELECT * FROM customer WHERE deleted = 0  ORDER BY name");
	while (customerQry.next())
	{
		QString customerId = customerQry.value("customer_id").toString();
		double outstandingAmount = ES::Utility::getTotalCreditOutstanding(customerId);
		totalOutstanding += outstandingAmount;
		QString cName = customerQry.value("name").toString();
		QString phone = customerQry.value("phone").toString();
		QString address = customerQry.value("address").toString();
		QString comments = customerQry.value("comments").toString();

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

		QTableWidgetItem *outstandingWidget = new QTableWidgetItem(QString::number(outstandingAmount, 'f', 2));
		outstandingWidget->setTextAlignment(Qt::AlignRight);
		ui.tableWidget->setItem(row, 3, outstandingWidget);
	}
	ui.totalLbl->setText(QString::number(totalOutstanding, 'f', 2));
}

void ESCustomerOutstandingSummary::printRow(KDReports::TableElement& tableElement, int row, int col, QString elementStr, Qt::AlignmentFlag alignment /*= Qt::AlignLeft*/)
{
	KDReports::Cell& cell = tableElement.cell(row, col);
	KDReports::TextElement te(elementStr);
	te.setPointSize(ES::Session::getInstance()->getBillItemFontSize());
	cell.addElement(te, alignment);
}

Ui::CustomerOutstandingSummary& ESCustomerOutstandingSummary::getUI()
{
	return ui;
}


void ESCustomerOutstandingSummary::slotGenerateReport()
{
	report = new KDReports::Report;

	KDReports::TextElement titleElement("Customer Outstanding Report");
	titleElement.setPointSize(13);
	titleElement.setBold(true);
	report->addElement(titleElement, Qt::AlignHCenter);

	QString currDateStr = QDateTime::currentDateTime().toString("yyyy-MM-dd");
	QString dateStr = "Date : ";
	dateStr.append(currDateStr);


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
	tableElement.setHeaderColumnCount(4);
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
	double totalOutstanding = 0;
	while (customerQry.next())
	{
		QString customerId = customerQry.value("customer_id").toString();
		double outstandingAmount = ES::Utility::getTotalCreditOutstanding(customerId);
		totalOutstanding += outstandingAmount;
		QString cName = customerQry.value("name").toString();
		QString phone = customerQry.value("phone").toString();
		QString address = customerQry.value("address").toString();
		QString comments = customerQry.value("comments").toString();

		printRow(tableElement, row, 0, cName);
		printRow(tableElement, row, 1, phone);
		printRow(tableElement, row, 2, address);
		printRow(tableElement, row, 3, QString::number(outstandingAmount, 'f', 2));
		row++;
	}

	printRow(tableElement, row, 2, "Total ");
	printRow(tableElement, row, 3, QString::number(totalOutstanding, 'f', 2));

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
