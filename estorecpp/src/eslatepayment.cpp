#include "eslatepayment.h"
#include "QSqlQuery"


ESLatePayment::ESLatePayment(QWidget *parent /*= 0*/) : QWidget(parent)
{
	ui.setupUi(this);
	
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

}

ESLatePayment::~ESLatePayment()
{

}

void ESLatePayment::slotAddBill()
{
	QString billId = ui.billIdText->text();
	addBill(billId);
}

void ESLatePayment::addBill(QString billId)
{
	if (billId.isEmpty())
	{
		return;
	}
	bool valid = false;
	(void)billId.toInt(&valid);
	if (!valid)
	{
		return;
	}

	QSqlQuery q("SELECT * FROM bill WHERE bill_id = " + billId);
	if (q.next())
	{
		int row = ui.bills->rowCount();
		ui.bills->insertRow(row);

		ui.bills->setItem(row, 0, new QTableWidgetItem(billId));
		ui.bills->setItem(row, 1, new QTableWidgetItem(q.value("date").toString()));
		ui.bills->setItem(row, 2, new QTableWidgetItem(QString::number(q.value("amount").toDouble(), 'f', 2)));
	}
}

void ESLatePayment::setCustomerId(QString customerId)
{
	m_selectedCustomerId = customerId;
}
