#include "eslatepayment.h"


ESLatePayment::ESLatePayment(QString billId, QWidget *parent /*= 0*/) : QWidget(parent)
{
	ui.setupUi(this);
	m_billId = billId;

	QStringList headerLabels1;
	headerLabels1.append("Bill ID");
	headerLabels1.append("Date");
	headerLabels1.append("Amount");
	headerLabels1.append("Actions");

	ui.bills->setHorizontalHeaderLabels(headerLabels1);
	ui.bills->horizontalHeader()->setStretchLastSection(true);
	ui.bills->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.bills->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.bills->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.bills->setSelectionMode(QAbstractItemView::SingleSelection);

	QStringList headerLabels;
	headerLabels.append("Payment Type");
	headerLabels.append("Amount");
	headerLabels.append("Interest");
	headerLabels.append("Payment");
	headerLabels.append("Due Date");
	headerLabels.append("Number");
	headerLabels.append("Bank");
	headerLabels.append("Actions");
	headerLabels.append("Id");

	ui.tableWidget->setHorizontalHeaderLabels(headerLabels);
	ui.tableWidget->horizontalHeader()->setStretchLastSection(true);
	ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	ui.tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.tableWidget->hideColumn(8);

	if (!billId.isEmpty())
	{
		
	}
}

ESLatePayment::~ESLatePayment()
{

}

void ESLatePayment::slotSearch()
{

}
