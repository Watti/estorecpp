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

// 	while (ui.tableWidgetByUser->rowCount() > 0)
// 	{
// 		ui.tableWidgetByUser->removeRow(0);
// 	}
	ui.datelbl->setText(QDate::currentDate().toString("yyyy-MM-dd"));
//bill.date >= CURDATE() - INTERVAL 1 DAY 
	QSqlQuery totalSalesQry("SELECT SUM(amount) as total, payment_type FROM payment JOIN bill ON payment.bill_id = bill.bill_id WHERE bill.deleted = 0 AND bill.status = 1 Group By payment.payment_type");
	int row = ui.tableWidgetTotal->rowCount();
	int col = 0;
	while (totalSalesQry.next())
	{
		QString paymentType = totalSalesQry.value("payment_type").toString();
		double tot = totalSalesQry.value("total").toDouble();
		QTableWidgetItem* itemSum = new QTableWidgetItem(QString::number(tot,'f', 2));
		if (paymentType == "CASH")
		{
			ui.tableWidgetTotal->setItem(0, 0, itemSum);
		}
		if (paymentType == "CREDIT")
		{
			ui.tableWidgetTotal->setItem(0, 1, itemSum);
		}
		if (paymentType == "CHEQUE")
		{
			ui.tableWidgetTotal->setItem(0, 2, itemSum);
		}
		if (paymentType == "CREDIT CARD")
		{
			ui.tableWidgetTotal->setItem(0, 3, itemSum);
		}
		if (paymentType == "LOYALTY")
		{
			ui.tableWidgetTotal->setItem(0, 4, itemSum);
		}
	}

	QSqlQuery userQry("SELECT * FROM user WHERE active = 1");
	while (userQry.next())
	{
		QString uId = userQry.value("user_id").toString();
		QString uName = userQry.value("display_name").toString();

		row = ui.tableWidgetByUser->rowCount();
		ui.tableWidgetByUser->insertRow(row);
		QTableWidgetItem* nameItem = new QTableWidgetItem(uName);
		ui.tableWidgetByUser->setItem(row, 0, nameItem);
		QSqlQuery userSalesQry("SELECT SUM(amount) as total, payment_type FROM payment JOIN bill ON payment.bill_id = bill.bill_id WHERE bill.deleted = 0 AND bill.status = 1 AND bill.user_id = "+uId+"  Group By payment.payment_type");
		while (userSalesQry.next())
		{
			QString paymentType = userSalesQry.value("payment_type").toString();
			double tot = totalSalesQry.value("total").toDouble();
			QTableWidgetItem* itemSum = new QTableWidgetItem(QString::number(tot, 'f', 2));
			if (paymentType == "CASH")
			{
				ui.tableWidgetByUser->setItem(row, 1, itemSum);
			}
			if (paymentType == "CREDIT")
			{
				ui.tableWidgetByUser->setItem(row, 2, itemSum);
			}
			if (paymentType == "CHEQUE")
			{
				ui.tableWidgetByUser->setItem(row, 3, itemSum);
			}
			if (paymentType == "CREDIT CARD")
			{
				ui.tableWidgetByUser->setItem(row, 4, itemSum);
			}
			if (paymentType == "LOYALTY")
			{
				ui.tableWidgetByUser->setItem(row, 5, itemSum);
			}
		}
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
