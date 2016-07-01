#include "essalesummaryreport.h"
#include "QSqlQuery"

ESSalesSummary::ESSalesSummary(QWidget *parent /*= 0*/) : QWidget(parent)
{
	ui.setupUi(this);

	QStringList headerLabels;
	headerLabels.append("Payment Mode");
	headerLabels.append("Sub Total");

	ui.tableWidgetByUser->setHorizontalHeaderLabels(headerLabels);
	ui.tableWidgetByUser->horizontalHeader()->setStretchLastSection(true);
	ui.tableWidgetByUser->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tableWidgetByUser->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableWidgetByUser->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableWidgetByUser->setSelectionMode(QAbstractItemView::SingleSelection);

	QObject::connect(ui.generateButton, SIGNAL(clicked()), this, SLOT(slotGenerate()));

	while (ui.tableWidgetByUser->rowCount() > 0)
	{
		ui.tableWidgetByUser->removeRow(0);
	}
	ui.datelbl->setText(QDate::currentDate().toString("yyyy-MM-dd"));
	QSqlQuery qurySales("SELECT SUM(amount) as total, payment_type FROM payment JOIN bill ON payment.bill_id = bill.bill_id WHERE bill.deleted = 0 AND bill.status = 1 AND bill.date >= CURDATE() - INTERVAL 1 DAY Group By payment.payment_type");
	while (qurySales.next())
	{
		int row = ui.tableWidgetByUser->rowCount();
		ui.tableWidgetByUser->insertRow(row);
		QTableWidgetItem* itemMode = new QTableWidgetItem(qurySales.value("payment_type").toString().toUpper());
		ui.tableWidgetByUser->setItem(row, 0, itemMode);

		QTableWidgetItem* itemSum = new QTableWidgetItem(qurySales.value("total").toString());
		ui.tableWidgetByUser->setItem(row, 1, itemSum);
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
