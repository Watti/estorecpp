#include "essalesummaryreport.h"
#include "QSqlQuery"

ESSalesSummary::ESSalesSummary(QWidget *parent /*= 0*/) : QWidget(parent)
{
	ui.setupUi(this);

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
	ui.tableWidgetByUser->setFont(font);

	ui.tableWidgetByUser->verticalHeader()->setFont(font);
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
	ui.tableWidgetTotal->setFont(font);

	ui.tableWidgetTotal->verticalHeader()->setFont(font);
	ui.tableWidgetTotal->verticalHeader()->setMinimumWidth(200);

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
