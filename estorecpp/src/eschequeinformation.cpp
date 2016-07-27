#include "eschequeinformation.h"

ESChequeInformation::ESChequeInformation(QWidget *parent /*= 0*/) : QWidget(parent)
{
	ui.setupUi(this);

	QStringList headerLabels;
	headerLabels.append("Customer");
	headerLabels.append("Cheque No.");
	headerLabels.append("Bank");
	headerLabels.append("Due Date");
	headerLabels.append("Discount");
	headerLabels.append("Actions");

	ui.tableWidget->setHorizontalHeaderLabels(headerLabels);
	ui.tableWidget->horizontalHeader()->setStretchLastSection(true);
	ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);

	slotSearch();
}

ESChequeInformation::~ESChequeInformation()
{

}

void ESChequeInformation::slotSearch()
{

}
