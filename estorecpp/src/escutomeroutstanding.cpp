#include "escutomeroutstanding.h"
#include <QtGui>
#include "QSqlQuery"
#include "QPushButton"
#include "QMessageBox"
#include "eslatepayment.h"
#include "utility\session.h"

ESCustomerOutstanding::ESCustomerOutstanding(QWidget *parent /*= 0*/) : QWidget(parent)
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
	ui.customers->verticalHeader()->hide();
	ui.customers->hideColumn(0);

	QStringList headerLabels2;
	headerLabels2.append("Payment Type");
	headerLabels2.append("Bill ID");
	headerLabels2.append("Amount");
	headerLabels2.append("Due Date");
	headerLabels2.append("Actions");

	ui.outstandings->setHorizontalHeaderLabels(headerLabels2);
	ui.outstandings->horizontalHeader()->setStretchLastSection(true);
	ui.outstandings->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.outstandings->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.outstandings->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.outstandings->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.outstandings->verticalHeader()->hide();
	//ui.outstandings->hideColumn(0);

	ui.paymentType->addItem("ALL");
	ui.paymentType->addItem("CREDIT");
	ui.paymentType->addItem("CHEQUE");
	ui.paymentType->setCurrentIndex(0);

	QObject::connect(ui.customers, SIGNAL(cellPressed(int, int)), this, SLOT(slotCustomerSelected(int, int)));
	QObject::connect(ui.searchText, SIGNAL(textChanged(QString)), this, SLOT(slotSearchCustomers()));
	QObject::connect(ui.paymentType, SIGNAL(activated(int)), this, SLOT(populateCustomerOutstanding()));
	QObject::connect(ui.checkBox, SIGNAL(stateChanged(int)), this, SLOT(populateCustomerOutstanding()));
	QObject::connect(m_paymentDetailsMapper, SIGNAL(mapped(QString)), this, SLOT(slotPay(QString)));

	slotSearchCustomers();

	m_selectedCustomerId = "-1";
	ui.searchText->setFocus();
}

ESCustomerOutstanding::~ESCustomerOutstanding()
{

}

void ESCustomerOutstanding::slotSearchCustomers()
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
	}
}

void ESCustomerOutstanding::slotCustomerSelected(int row, int col)
{
	if (row > -1)
	{
		QString customerId = ui.customers->item(row, 0)->text();
		m_selectedCustomerId = customerId;

		// Populate customer history
		populateCustomerOutstanding();
	}
}

void ESCustomerOutstanding::populateCustomerOutstanding()
{
	if (m_selectedCustomerId == "-1")
	{
		return;
	}

	while (ui.outstandings->rowCount() > 0)
	{
		ui.outstandings->removeRow(0);
	}

	QString query;
	query.append("SELECT * FROM customer_outstanding WHERE customer_id = ");
	query.append(m_selectedCustomerId);
	query.append(" AND settled = 0");

	QSqlQuery q(query);
	while (q.next())
	{
		int row = ui.outstandings->rowCount();
		ui.outstandings->insertRow(row);

		QString paymentId = q.value("payment_id").toString();
		QSqlQuery qry("SELECT * FROM payment WHERE payment_id = " + paymentId);
		QString pm = q.value("payment_method").toString();
		QString dueDate = "";
		float interest = 0;
		float totalAmount = 0;
		if (pm == "CREDIT")
		{
			QSqlQuery qq("SELECT * FROM credit WHERE credit_id = " + q.value("table_id").toString());
			if (qq.next())
			{
				dueDate = qq.value("due_date").toString();
				interest = qq.value("interest").toFloat();
				float amount = qq.value("amount").toFloat();
				totalAmount += (amount * (100 + interest) / 100);
			}
		}
		else if (pm == "CHEQUE")
		{
			QSqlQuery qq("SELECT * FROM cheque WHERE cheque_id = " + q.value("table_id").toString());
			if (qq.next())
			{
				dueDate = qq.value("due_date").toString();
				interest = qq.value("interest").toFloat();
				float amount = qq.value("amount").toFloat();
				totalAmount += (amount * (100 + interest) / 100);
			}
		}

		ui.outstandings->setItem(row, 0, new QTableWidgetItem(pm));
		QString billId = "-1";

		if (qry.next())
		{
			billId = qry.value("bill_id").toString();
			ui.outstandings->setItem(row, 1, new QTableWidgetItem(billId));
		}

		QTableWidgetItem* item = new QTableWidgetItem(QString::number(totalAmount, 'f', 2));
		item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
		ui.outstandings->setItem(row, 2, item);

		QTableWidgetItem* itemDate = new QTableWidgetItem(dueDate);
		itemDate->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
		ui.outstandings->setItem(row, 3, itemDate);
		//ui.outstandings->setItem(row, 4, new QTableWidgetItem("-"));

		QWidget* base = new QWidget(ui.outstandings);
		QHBoxLayout *layout = new QHBoxLayout;
		layout->setContentsMargins(0, 0, 0, 0);

		QPushButton* payBtn = new QPushButton("Pay", base);
		payBtn->setMaximumWidth(100);
		m_paymentDetailsMapper->setMapping(payBtn, billId);
		QObject::connect(payBtn, SIGNAL(clicked()), m_paymentDetailsMapper, SLOT(map()));
		layout->addWidget(payBtn);

		layout->insertStretch(2);
		base->setLayout(layout);
		ui.outstandings->setCellWidget(row, 4, base);
		base->show();

	}
}

void ESCustomerOutstanding::slotPay(QString billId)
{
	QSqlQuery qry("SELECT * FROM bill WHERE bill_id = " + billId);
	if (qry.next())
	{
		QString customerIdStr = qry.value("customer_id").toString();
		QSqlQuery q("SELECT * FROM customer WHERE customer_id = " + customerIdStr);
		if (q.next())
		{
			ESLatePayment* latePayment = new ESLatePayment(0);
			latePayment->setWindowState(Qt::WindowActive);
			latePayment->setWindowModality(Qt::ApplicationModal);
			latePayment->setAttribute(Qt::WA_DeleteOnClose);

			latePayment->getUI().customerName->setText(q.value("name").toString());
			latePayment->getUI().cashierName->setText(ES::Session::getInstance()->getUser()->getName());

			//outstanding start
			float totalAmount = 0;
			int customerId = customerIdStr.toInt();
			if (customerId > -1)
			{
				totalAmount = getTotalOutstanding(customerIdStr);
			}
			// outstanding end

			latePayment->show();
		}
	}	
}


float ESCustomerOutstanding::getTotalOutstanding(QString customerId)
{
	float totalAmount = 0.f;
	QString query;
	query.append("SELECT * FROM customer_outstanding WHERE customer_id = ");
	query.append(customerId);
	query.append(" AND settled = 0");

	QSqlQuery q(query);
	while (q.next())
	{
		QString paymentId = q.value("payment_id").toString();
		QSqlQuery qry("SELECT * FROM payment WHERE payment_id = " + paymentId);
		QString pm = q.value("payment_method").toString();
		float interest = 0;
		if (pm == "CREDIT")
		{
			QSqlQuery qq("SELECT * FROM credit WHERE credit_id = " + q.value("table_id").toString());
			if (qq.next())
			{
				interest = qq.value("interest").toFloat();
				float amount = qq.value("amount").toFloat();
				totalAmount += (amount * (100 + interest) / 100);
			}
		}
		else if (pm == "CHEQUE")
		{
			QSqlQuery qq("SELECT * FROM cheque WHERE cheque_id = " + q.value("table_id").toString());
			if (qq.next())
			{
				interest = qq.value("interest").toFloat();
				float amount = qq.value("amount").toFloat();
				totalAmount += (amount * (100 + interest) / 100);
			}
		}
	}
	return totalAmount;
}

