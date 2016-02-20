#include "escurrentbills.h"
#include "utility/esmainwindowholder.h"
#include "utility/esdbconnection.h"
#include <QMessageBox>

ESCurrentBills::ESCurrentBills(QWidget *parent)
: QWidget(parent)
{
	ui.setupUi(this);

	QStringList headerLabels;
	headerLabels.append("Bill ID");
	headerLabels.append("Date");
	headerLabels.append("Payment Method");
	headerLabels.append("Amount");
	headerLabels.append("User");
	headerLabels.append("Status");
	headerLabels.append("Actions");

	ui.tableWidget->setHorizontalHeaderLabels(headerLabels);
	ui.tableWidget->horizontalHeader()->setStretchLastSection(true);
	ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);

	if (!ES::DbConnection::instance()->open())
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Critical);
		mbox.setText(QString("Cannot connect to the database : ESManageStockItems::displayStockItems "));
		mbox.exec();
	}
	else
	{
		QSqlQuery queryPM("SELECT * FROM payment");
		while (queryPM.next())
		{
			ui.pmComboBox->addItem("COMMITTED", 1);
			ui.pmComboBox->addItem("PENDING", 2);
			ui.pmComboBox->addItem("CANCELED", 3);
		}
	}
}

ESCurrentBills::~ESCurrentBills()
{

}
