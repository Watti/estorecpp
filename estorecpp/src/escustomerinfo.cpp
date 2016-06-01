#include "escustomerinfo.h"
#include "utility/esdbconnection.h"
#include <QMessageBox>

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

	slotSearch();

	QObject::connect(ui.customers, SIGNAL(cellPressed(int, int)), this, SLOT(slotCustomerSelected(int, int)));
	ui.commentsLbl->setWordWrap(true);
	
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
	}
}

