#include "salesummaryreport.h"
#include "QSqlQuery"

ESSalesSummary::ESSalesSummary(QWidget *parent /*= 0*/) : QWidget(parent)
{
	ui.setupUi(this);

	QStringList headerLabels;
	headerLabels.append("Payment Mode");
	headerLabels.append("Sub Total");

	ui.tableWidget->setHorizontalHeaderLabels(headerLabels);
	ui.tableWidget->horizontalHeader()->setStretchLastSection(true);
	ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);

	QObject::connect(ui.generateButton, SIGNAL(clicked()), this, SLOT(slotGenerate()));

	while (ui.tableWidget->rowCount() > 0)
	{
		ui.tableWidget->removeRow(0);
	}
	ui.datelbl->setText(QDate::currentDate().toString("yyyy-MM-dd"));
	QSqlQuery qurySales("SELECT SUM(amount) as total, payment_type FROM payment JOIN bill ON payment.bill_id = bill.bill_id WHERE bill.deleted = 0 AND bill.status = 1 AND bill.date >= CURDATE() - INTERVAL 1 DAY Group By payment.payment_type");
	while (qurySales.next())
	{
		int row = ui.tableWidget->rowCount();
		ui.tableWidget->insertRow(row);
		QTableWidgetItem* itemMode = new QTableWidgetItem(qurySales.value("payment_type").toString().toUpper());
		ui.tableWidget->setItem(row, 0, itemMode);

		QTableWidgetItem* itemSum = new QTableWidgetItem(qurySales.value("total").toString());
		ui.tableWidget->setItem(row, 1 , itemSum);
	}
}

void ESSalesSummary::slotPrint(QPrinter* printer) 
{
	report.print(printer);
}

void ESSalesSummary::slotGenerate()
{

}

ESSalesSummary::~ESSalesSummary()
{

}
