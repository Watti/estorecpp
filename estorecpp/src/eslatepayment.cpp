#include "eslatepayment.h"
#include "QSqlQuery"


ESLatePayment::ESLatePayment(QWidget *parent /*= 0*/) : QWidget(parent)
{
	ui.setupUi(this);
	m_removeButtonMapper = new QSignalMapper(this);
	
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

	QObject::connect(ui.addBillBtn, SIGNAL(clicked()), this, SLOT(slotAddBill()));
	QObject::connect(ui.okBtn, SIGNAL(clicked()), this, SLOT(slotOk()));
	
	QObject::connect(ui.cashBtn, SIGNAL(clicked()), this, SLOT(slotAddCash()));
	QObject::connect(ui.creditBtn, SIGNAL(clicked()), this, SLOT(slotAddCredit()));
	QObject::connect(ui.chequeBtn, SIGNAL(clicked()), this, SLOT(slotAddCheque()));
	QObject::connect(ui.cardBtn, SIGNAL(clicked()), this, SLOT(slotAddCard()));
	QObject::connect(ui.loyaltyBtn, SIGNAL(clicked()), this, SLOT(slotAddLoyalty()));

	QObject::connect(m_removeButtonMapper, SIGNAL(mapped(QString)), this, SLOT(slotRemoveBill(QString)));
}

ESLatePayment::~ESLatePayment()
{

}

void ESLatePayment::slotAddBill()
{
	QString billId = ui.billIdText->text();
	addBill(billId);
	ui.billIdText->setText("");
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

	for (int r = 0; r < ui.bills->rowCount(); ++r)
	{
		QTableWidgetItem* item = ui.bills->item(r, 0);
		if (item && item->text() == billId)
		{
			return;
		}
	}

	QSqlQuery q("SELECT * FROM bill WHERE bill_id = " + billId);
	if (q.next())
	{
		int row = ui.bills->rowCount();
		ui.bills->insertRow(row);

		ui.bills->setItem(row, 0, new QTableWidgetItem(billId));
		ui.bills->setItem(row, 1, new QTableWidgetItem(q.value("date").toString()));
		ui.bills->setItem(row, 2, new QTableWidgetItem(QString::number(q.value("amount").toDouble(), 'f', 2)));

		QWidget* base = new QWidget(ui.bills);
		QHBoxLayout *layout = new QHBoxLayout;
		layout->setContentsMargins(0, 0, 0, 0);

		QPushButton* removeBtn = new QPushButton("Remove", base);
		removeBtn->setMaximumWidth(100);
		m_removeButtonMapper->setMapping(removeBtn, billId);
		QObject::connect(removeBtn, SIGNAL(clicked()), m_removeButtonMapper, SLOT(map()));
		layout->addWidget(removeBtn);

		layout->insertStretch(2);
		base->setLayout(layout);
		ui.bills->setCellWidget(row, 3, base);
		base->show();
	}
}

void ESLatePayment::setCustomerId(QString customerId)
{
	m_selectedCustomerId = customerId;
}

void ESLatePayment::slotRemoveBill(QString billId)
{
	for (int r = 0; r < ui.bills->rowCount(); ++r)
	{
		QTableWidgetItem* item = ui.bills->item(r, 0);
		if (item && item->text() == billId)
		{
			ui.bills->removeRow(r);
			return;
		}
	}
}

void ESLatePayment::slotOk()
{

}

void ESLatePayment::slotAddCash()
{

}

void ESLatePayment::slotAddCredit()
{

}

void ESLatePayment::slotAddCheque()
{

}

void ESLatePayment::slotAddCard()
{

}

void ESLatePayment::slotAddLoyalty()
{

}
