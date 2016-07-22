#include "escustomerinfo.h"
#include "utility/esdbconnection.h"
#include <QMessageBox>
#include <QPushButton>
#include "espaymentdetails.h"

ESCustomerInfo::ESCustomerInfo(QWidget *parent /*= 0*/) : QWidget(parent)
{
	ui.setupUi(this);

	m_paymentDetailsMapper = new QSignalMapper(this);

	QStringList headerLabels;
	headerLabels.append("CustomerID");
	headerLabels.append("Name");
	headerLabels.append("Phone");
	headerLabels.append("Address");
	headerLabels.append("Comments");
	
	ui.customers->setHorizontalHeaderLabels(headerLabels);
	ui.customers->horizontalHeader()->setStretchLastSection(true);
	ui.customers->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.customers->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.customers->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.customers->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.customers->hideColumn(0);

	QStringList headerLabels2;
	headerLabels2.append("Bill ID");
	headerLabels2.append("Date");
	headerLabels2.append("Amount");
	headerLabels2.append("Actions");

	ui.customerHistory->setHorizontalHeaderLabels(headerLabels2);
	ui.customerHistory->horizontalHeader()->setStretchLastSection(true);
	ui.customerHistory->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	ui.customerHistory->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.customerHistory->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.customerHistory->setSelectionMode(QAbstractItemView::NoSelection);

	slotSearch();

	QObject::connect(ui.customers, SIGNAL(cellPressed(int, int)), this, SLOT(slotCustomerSelected(int, int)));
	QObject::connect(ui.showFullHistory, SIGNAL(stateChanged(int)), this, SLOT(slotPopulateCustomerHistory()));
	QObject::connect(ui.searchText, SIGNAL(textChanged(QString)), this, SLOT(slotSearch()));
	QObject::connect(m_paymentDetailsMapper, SIGNAL(mapped(QString)), this, SLOT(slotShowPaymentDetails(QString)));
	QObject::connect(ui.deleteBtn, SIGNAL(clicked()), this, SLOT(slotDeleteCustomer()));

	ui.commentsLbl->setWordWrap(true);
	m_selectedCustomerId = "-1";
	ui.searchText->setFocus();
}

ESCustomerInfo::~ESCustomerInfo()
{

}

void ESCustomerInfo::slotSearch()
{
	while (ui.customers->rowCount() > 0)
	{
		ui.customers->removeRow(0);
	}
	QString searchText = ui.searchText->text();
	QString queryStr = "SELECT * FROM customer WHERE deleted = 0";

	if (!searchText.isEmpty())
	{
		queryStr += " AND (name LIKE '%" + searchText + "%' OR address LIKE '%" + searchText + "%' OR comments LIKE '%" + searchText + "%')";
	}

	QSqlQuery q(queryStr);

	int row = 0;
	while (q.next())
	{
		row = ui.customers->rowCount();
		ui.customers->insertRow(row);

		ui.customers->setItem(row, 0, new QTableWidgetItem(q.value("customer_id").toString()));
		ui.customers->setItem(row, 1, new QTableWidgetItem(q.value("name").toString()));
		ui.customers->setItem(row, 2, new QTableWidgetItem(q.value("phone").toString()));
		ui.customers->setItem(row, 3, new QTableWidgetItem(q.value("address").toString()));
		ui.customers->setItem(row, 4, new QTableWidgetItem(q.value("comments").toString()));

		row++;
	}

}

void ESCustomerInfo::slotCustomerSelected(int row, int col)
{
	if (row > -1)
	{
		QString customerId = ui.customers->item(row, 0)->text();
		m_selectedCustomerId = customerId;

		QSqlQuery q("SELECT * FROM customer WHERE customer_id = ?");
		q.addBindValue(customerId);

		if (q.exec())
		{
			if (q.next())
			{
				ui.nameLbl->setText(q.value("name").toString());
				ui.phoneLbl->setText(q.value("phone").toString());
				ui.addressLbl->setText(q.value("address").toString());
				ui.commentsLbl->setText(q.value("comments").toString());
			}
		}
		else
		{
			QMessageBox mbox;
			mbox.setIcon(QMessageBox::Critical);
			mbox.setText(QString("Cannot find the customer"));
			mbox.exec();
		}

		// Populate customer history
		slotPopulateCustomerHistory();
	}
}

void ESCustomerInfo::slotPopulateCustomerHistory()
{
	if (m_selectedCustomerId.isEmpty())
	{
		return;
	}

	while (ui.customerHistory->rowCount() > 0)
	{
		ui.customerHistory->removeRow(0);
	}

	QString q("SELECT * FROM bill WHERE customer_id = " + m_selectedCustomerId + " ORDER BY date DESC ");
	if (!ui.showFullHistory->isChecked())
	{
		q.append("LIMIT 15");

		ui.prevBtn->setEnabled(false);
		ui.nextBtn->setEnabled(false);
	}
	else
	{
		ui.prevBtn->setEnabled(true);
		ui.nextBtn->setEnabled(true);
	}

	QSqlQuery billQuery(q);
	while (billQuery.next())
	{
		int row = ui.customerHistory->rowCount();
		ui.customerHistory->insertRow(row);

		QString billId = billQuery.value("bill_id").toString();

		QTableWidgetItem* billItem = new QTableWidgetItem(billId);
		billItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
		ui.customerHistory->setItem(row, 0, billItem);

		QDateTime dt = billQuery.value("date").toDateTime();
		ui.customerHistory->setItem(row, 1, new QTableWidgetItem(dt.toString("yyyy-MM-dd (hh:mm)")));

		QTableWidgetItem* item = new QTableWidgetItem(QString::number(billQuery.value("amount").toDouble(), 'f', 2));
		item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
		ui.customerHistory->setItem(row, 2, item);

		QPushButton* btn = new QPushButton("Payment Details", ui.customerHistory);
		btn->setMaximumWidth(200);
		ui.customerHistory->setCellWidget(row, 3, btn);

		m_paymentDetailsMapper->setMapping(btn, billId);
		QObject::connect(btn, SIGNAL(clicked()), m_paymentDetailsMapper, SLOT(map()));
	}
}

void ESCustomerInfo::slotShowPaymentDetails(QString billId)
{
	QSqlQuery q("SELECT * FROM payment WHERE bill_id = " + billId);

	ESPaymentDetails infoDialog;
	infoDialog.setWindowTitle("Bill ID : " + billId);
	QTableWidget* table = infoDialog.getUi().tableWidget;

	while (q.next())
	{
		int row = table->rowCount();
		table->insertRow(row);

		QString paymentId = q.value("payment_id").toString();
		QString paymentType = q.value("payment_type").toString();
		table->setItem(row, 0, new QTableWidgetItem(paymentType));

		if (paymentType == "CASH")
		{
			QSqlQuery cashQ("SELECT * FROM cash WHERE payment_id = " + paymentId);
			if (cashQ.next())
			{
				QTableWidgetItem* item = new QTableWidgetItem(QString::number(cashQ.value("amount").toDouble(), 'f', 2));
				item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
				table->setItem(row, 1, item);
				item = new QTableWidgetItem("0.00");
				item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
				table->setItem(row, 2, item);
				table->setItem(row, 3, new QTableWidgetItem("-"));
				table->setItem(row, 4, new QTableWidgetItem("-"));
			}			
		}
		else if (paymentType == "CREDIT")
		{
			QSqlQuery creditQ("SELECT * FROM credit WHERE payment_id = " + paymentId);
			if (creditQ.next())
			{
				QTableWidgetItem* item = new QTableWidgetItem(QString::number(creditQ.value("amount").toDouble(), 'f', 2));
				item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
				table->setItem(row, 1, item);
				item = new QTableWidgetItem(QString::number(creditQ.value("interest").toDouble(), 'f', 2));
				item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
				table->setItem(row, 2, item);
				table->setItem(row, 3, new QTableWidgetItem(creditQ.value("due_date").toString()));
				table->setItem(row, 4, new QTableWidgetItem("-"));
			}				
		}
		else if (paymentType == "CHEQUE")
		{
			QSqlQuery chequeQ("SELECT * FROM cheque WHERE payment_id = " + paymentId);
			if (chequeQ.next())
			{
				QTableWidgetItem* item = new QTableWidgetItem(QString::number(chequeQ.value("amount").toDouble(), 'f', 2));
				item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
				table->setItem(row, 1, item);
				item = new QTableWidgetItem(QString::number(chequeQ.value("interest").toDouble(), 'f', 2));
				item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
				table->setItem(row, 2, item);
				table->setItem(row, 3, new QTableWidgetItem(chequeQ.value("due_date").toString()));
				table->setItem(row, 4, new QTableWidgetItem("-"));
			}
		}
		else if (paymentType == "CARD")
		{
			QSqlQuery cardQ("SELECT * FROM card WHERE payment_id = " + paymentId);
			if (cardQ.next())
			{
				QTableWidgetItem* item = new QTableWidgetItem(QString::number(cardQ.value("amount").toDouble(), 'f', 2));
				item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
				table->setItem(row, 1, item);
				item = new QTableWidgetItem(QString::number(cardQ.value("interest").toDouble(), 'f', 2));
				item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
				table->setItem(row, 2, item);
				table->setItem(row, 3, new QTableWidgetItem(cardQ.value("due_date").toString()));
				table->setItem(row, 4, new QTableWidgetItem("-"));
			}
		}
		else if (paymentType == "LOYALTY")
		{
			QSqlQuery loyaltyQ("SELECT * FROM loyalty WHERE payment_id = " + paymentId);
			if (loyaltyQ.next())
			{
				QTableWidgetItem* item = new QTableWidgetItem(QString::number(loyaltyQ.value("amount").toDouble(), 'f', 2));
				item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
				table->setItem(row, 1, item);
				item = new QTableWidgetItem(QString::number(loyaltyQ.value("interest").toDouble(), 'f', 2));
				item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
				table->setItem(row, 2, item);
				table->setItem(row, 3, new QTableWidgetItem(loyaltyQ.value("due_date").toString()));
				table->setItem(row, 4, new QTableWidgetItem("-"));
			}
		}
	}

	infoDialog.exec();
}

void ESCustomerInfo::slotDeleteCustomer()
{
	if (m_selectedCustomerId.isEmpty() || m_selectedCustomerId == "-1")
	{
		return;
	}
	QSqlQuery q("UPDATE customer SET deleted = 1 WHERE customer_id = " + m_selectedCustomerId);

	m_selectedCustomerId = "-1";
	slotSearch();

	ui.nameLbl->setText("");
	ui.phoneLbl->setText("");
	ui.addressLbl->setText("");
	ui.commentsLbl->setText("");
}

