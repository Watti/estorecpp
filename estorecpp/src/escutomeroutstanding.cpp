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
	m_paymentButtonMapper = new QSignalMapper(this);

	QStringList headerLabels;
	headerLabels.append("CustomerID");
	headerLabels.append("Name");
	headerLabels.append("Phone");
	headerLabels.append("Address");
	headerLabels.append("Comments");
	headerLabels.append("Current Outstanding");
	headerLabels.append("Actions");

	ui.customers->setHorizontalHeaderLabels(headerLabels);
	ui.customers->horizontalHeader()->setStretchLastSection(true);
	ui.customers->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.customers->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.customers->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.customers->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.customers->verticalHeader()->hide();
	ui.customers->hideColumn(0);

	QObject::connect(ui.searchText, SIGNAL(textChanged(QString)), this, SLOT(slotSearchCustomers()));
	QObject::connect(m_paymentButtonMapper, SIGNAL(mapped(QString)), this, SLOT(slotPay(QString)));

	slotSearchCustomers();

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

		QString customerId = q.value("customer_id").toString();
		double outstandingAmount = getTotalOutstanding(customerId);
		QColor rowColor;
		if (outstandingAmount > 0)
		{
			rowColor.setRgb(245, 169, 169);
		}
		else
		{
			rowColor.setRgb(169, 245, 208);
		}

		QTableWidgetItem* item = NULL;
		item = new QTableWidgetItem(customerId);
		item->setBackgroundColor(rowColor);
		ui.customers->setItem(row, 0, item);

		item = new QTableWidgetItem(q.value("name").toString());
		item->setBackgroundColor(rowColor);
		ui.customers->setItem(row, 1, item);

		item = new QTableWidgetItem(q.value("phone").toString());
		item->setBackgroundColor(rowColor);
		ui.customers->setItem(row, 2, item);

		item = new QTableWidgetItem(q.value("address").toString());
		item->setBackgroundColor(rowColor);
		ui.customers->setItem(row, 3, item);

		item = new QTableWidgetItem(q.value("comments").toString());
		item->setBackgroundColor(rowColor);
		ui.customers->setItem(row, 4, item);
				
		QTableWidgetItem* outstandingItem = new QTableWidgetItem(QString::number(outstandingAmount, 'f', 2));
		outstandingItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
		outstandingItem->setBackgroundColor(rowColor);
		ui.customers->setItem(row, 5, outstandingItem);

		//ui.customers->setItem(row, 6, new QTableWidgetItem("-"));
		QWidget* base = new QWidget(ui.customers);
		QPushButton* paymentBtn = new QPushButton("Settle", base);

		m_paymentButtonMapper->setMapping(paymentBtn, customerId);
		QObject::connect(paymentBtn, SIGNAL(clicked()), m_paymentButtonMapper, SLOT(map()));

		QHBoxLayout *layout = new QHBoxLayout;
		layout->setContentsMargins(0, 0, 0, 0);
		layout->addWidget(paymentBtn);
		layout->insertStretch(2);
		base->setLayout(layout);
		ui.customers->setCellWidget(row, 6, base);
		base->show();
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

void ESCustomerOutstanding::slotPay(QString customerId)
{
	QSqlQuery q("SELECT * FROM customer_outstanding WHERE custome_id = " + customerId);
	if (q.next())
	{

	}
}

