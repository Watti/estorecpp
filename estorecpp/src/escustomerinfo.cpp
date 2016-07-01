#include "escustomerinfo.h"
#include "utility/esdbconnection.h"
#include <QMessageBox>
#include <QPushButton>

ESCustomerInfo::ESCustomerInfo(QWidget *parent /*= 0*/) : QWidget(parent)
{
	ui.setupUi(this);

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

	ui.commentsLbl->setWordWrap(true);
	m_selectedCustomerId = "";
	
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

	QSqlQuery q("SELECT * FROM customer WHERE deleted = 0");

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

	QString q("SELECT * FROM bill WHERE user_id = " + m_selectedCustomerId + " ORDER BY date DESC ");
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

		QTableWidgetItem* billItem = new QTableWidgetItem(billQuery.value("bill_id").toString());
		billItem->setTextAlignment(Qt::AlignRight);
		ui.customerHistory->setItem(row, 0, billItem);

		QDateTime dt = billQuery.value("date").toDateTime();
		ui.customerHistory->setItem(row, 1, new QTableWidgetItem(dt.toString("yyyy-MM-dd (hh:mm)")));

		QTableWidgetItem* item = new QTableWidgetItem(QString::number(billQuery.value("amount").toDouble(), 'f', 2));
		item->setTextAlignment(Qt::AlignRight);
		ui.customerHistory->setItem(row, 2, item);

		QPushButton* btn = new QPushButton("Payment Details", ui.customerHistory);
		btn->setMaximumWidth(200);
		ui.customerHistory->setCellWidget(row, 3, btn);
	}
}

