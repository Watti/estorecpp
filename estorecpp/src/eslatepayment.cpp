#include "eslatepayment.h"
#include "QSqlQuery"
#include "entities\tabletextwidget.h"

namespace
{
	QString convertToPriceFormat(QString text, int row, int col, QTableWidget* table)
	{
		double val = text.toDouble();
		return QString::number(val, 'f', 2);
	}
}

ESLatePayment::ESLatePayment(QWidget *parent /*= 0*/) : QWidget(parent)
{
	ui.setupUi(this);
	m_removeBillButtonMapper = new QSignalMapper(this);
	m_removePaymentButtonMapper = new QSignalMapper(this);
	
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

	QObject::connect(m_removeBillButtonMapper, SIGNAL(mapped(QString)), this, SLOT(slotRemoveBill(QString)));
	QObject::connect(m_removePaymentButtonMapper, SIGNAL(mapped(QString)), this, SLOT(slotRemovePayment(QString)));
	QObject::connect(ui.tableWidget, SIGNAL(cellDoubleClicked(int, int)), this, SLOT(slotItemDoubleClicked(int, int)));
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
		m_removeBillButtonMapper->setMapping(removeBtn, billId);
		QObject::connect(removeBtn, SIGNAL(clicked()), m_removeBillButtonMapper, SLOT(map()));
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

void ESLatePayment::slotRemovePayment(QString id)
{
	for (int r = 0; r < ui.tableWidget->rowCount(); ++r)
	{
		QTableWidgetItem* item = ui.tableWidget->item(r, 8);
		if (item && item->text() == id)
		{
			ui.tableWidget->removeRow(r);
			return;
		}
	}
}

void ESLatePayment::slotOk()
{
	for (int r = 0; r < ui.tableWidget->rowCount(); ++r)
	{
		// add row to payment table, with `payment_order` = 2.
		// update `cash` table
	}
}

void ESLatePayment::slotAddCash()
{
	int row = ui.tableWidget->rowCount();
	ui.tableWidget->insertRow(row);

	ui.tableWidget->setItem(row, 0, new QTableWidgetItem("CASH"));
	ui.tableWidget->setItem(row, 1, new QTableWidgetItem("0.00"));
	ui.tableWidget->setItem(row, 2, new QTableWidgetItem("-/-"));
	ui.tableWidget->setItem(row, 3, new QTableWidgetItem("0.00"));
	ui.tableWidget->setItem(row, 4, new QTableWidgetItem("-/-"));
	ui.tableWidget->setItem(row, 5, new QTableWidgetItem("-/-"));
	ui.tableWidget->setItem(row, 6, new QTableWidgetItem("-/-"));
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

void ESLatePayment::slotItemDoubleClicked(int row, int col)
{
	QTableWidgetItem* paymentItem = ui.tableWidget->item(row, 0);
	if (paymentItem)
	{
		QString paymentType = paymentItem->text();
		if (paymentType == "CASH")
		{
			if (col == 1) // amount
			{
				
			}
			else if (col == 3) // payment
			{
				QTableWidgetItem* item = ui.tableWidget->item(row, 1);
				QString amountStr = item->text();

				TableTextWidget* textWidget = new TableTextWidget(ui.tableWidget, row, col, ui.tableWidget);
				QObject::connect(textWidget, SIGNAL(notifyEnterPressed(QString, int, int)), this, SLOT(slotCashPaymentUpdated(QString, int, int)));
				textWidget->setTextFormatterFunc(convertToPriceFormat);
				textWidget->setText(amountStr);
				textWidget->selectAll();
				ui.tableWidget->setCellWidget(row, col, textWidget);
				textWidget->setFocus();
			}
		}
		else if (paymentType == "CREDIT")
		{

		}
		else if (paymentType == "CHEQUE")
		{

		}
		else if (paymentType == "CARD")
		{

		}
		else if (paymentType == "LOYALTY")
		{

		}
	}
	
}

void ESLatePayment::slotCashPaymentUpdated(QString txt, int row, int col)
{
	QString price = QString::number(txt.toDouble(), 'f', 2);
	ui.tableWidget->setItem(row, 1, new QTableWidgetItem(price));
}
