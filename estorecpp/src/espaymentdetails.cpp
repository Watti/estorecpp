#include "espaymentdetails.h"

ESPaymentDetails::ESPaymentDetails() : QDialog(0)
{
	ui.setupUi(this);
	
	QStringList headerLabels;
	headerLabels.append("Payment Type");
	headerLabels.append("Amount");
	headerLabels.append("Interest");
	headerLabels.append("Due Date");
	headerLabels.append("Check");

	ui.tableWidget->setHorizontalHeaderLabels(headerLabels);
	ui.tableWidget->horizontalHeader()->setStretchLastSection(true);
	ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	ui.tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
}

ESPaymentDetails::~ESPaymentDetails()
{
	
}

void ESPaymentDetails::slotSearch()
{

}
