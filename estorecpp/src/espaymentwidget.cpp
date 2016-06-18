#include "espaymentwidget.h"
#include "utility/esdbconnection.h"
#include <QShortcut>
#include "utility/session.h"
#include "esaddbill.h"
#include "easylogging++.h"
#include <QMessageBox>
#include <QDesktopWidget>
#include "essinglepayment.h"
#include "esmultiplepayment.h"

ESPayment::ESPayment(ESAddBill* addBill, QWidget *parent /*= 0*/) :
QWidget(parent), m_addBill(addBill)
{
	m_customerId = "-1";
	ui.setupUi(this);
	ui.addressText->setWordWrap(true);
	ui.commentsText->setWordWrap(true);

	QStringList headerLabels;
	headerLabels.append("CustomerID");
	headerLabels.append("Name");
	headerLabels.append("Address");
	headerLabels.append("Comments");
	headerLabels.append("Phone");

	ui.customers->setHorizontalHeaderLabels(headerLabels);
	ui.customers->horizontalHeader()->setStretchLastSection(true);
	ui.customers->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.customers->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.customers->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.customers->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.customers->hideColumn(0);
		
	new QShortcut(QKeySequence(Qt::Key_Escape), this, SLOT(close()));
	QObject::connect(ui.searchText, SIGNAL(textChanged(QString)), this, SLOT(slotSearch()));
	QObject::connect(ui.customers, SIGNAL(cellPressed(int, int)), this, SLOT(slotCustomerSeleced(int, int)));
	QObject::connect(ui.singlePaymentButton, SIGNAL(clicked()), this, SLOT(slotSinglePayment()));
	QObject::connect(ui.multiplePaymentButton, SIGNAL(clicked()), this, SLOT(slotMultiplePayment()));

	slotSearch();
}

ESPayment::~ESPayment()
{

}

void ESPayment::slotSearch()
{
	while (ui.customers->rowCount() > 0)
	{
		ui.customers->removeRow(0);
	}

	QString q = "SELECT * FROM customer WHERE deleted = 0";
	QString searchText = ui.searchText->text();
	if (!searchText.isEmpty())
	{
		q.append(" AND (name LIKE '%" + searchText + "%' OR address LIKE '%" + searchText + "%')");
	}

	QSqlQuery queryCustomers(q);

	int row = 0;
	while (queryCustomers.next())
	{
		row = ui.customers->rowCount();
		ui.customers->insertRow(row);

		ui.customers->setItem(row, 0, new QTableWidgetItem(queryCustomers.value("customer_id").toString()));
		ui.customers->setItem(row, 1, new QTableWidgetItem(queryCustomers.value("name").toString()));
		ui.customers->setItem(row, 2, new QTableWidgetItem(queryCustomers.value("address").toString()));
		ui.customers->setItem(row, 3, new QTableWidgetItem(queryCustomers.value("comments").toString()));
		ui.customers->setItem(row, 4, new QTableWidgetItem(queryCustomers.value("phone").toString()));

		row++;
	}
	
}

void ESPayment::slotCustomerSeleced(int row, int col)
{
	m_customerId = ui.customers->item(row, 0)->text();

	QSqlQuery query;
	query.prepare("SELECT * FROM customer WHERE customer_id = ?");
	query.addBindValue(m_customerId);

	if (query.exec())
	{
		if (query.next())
		{
			ui.nameText->setText(query.value("name").toString());
			ui.phoneText->setText(query.value("phone").toString());
			ui.addressText->setText(query.value("address").toString());
			ui.commentsText->setText(query.value("comments").toString());

			m_name = ui.nameText->text();
			m_phone = ui.phoneText->text();
			m_address = ui.addressText->text();
			m_comments = ui.commentsText->text();
		}
	}
	else
	{
		m_customerId = "-1";
		m_name = "";
		m_phone = "";
		m_address = "";
		m_comments = "";
	}

}

void ESPayment::slotSinglePayment()
{
	ESSinglePayment* singlePayment = new ESSinglePayment(m_addBill, 0);
	singlePayment->setWindowState(Qt::WindowActive);
	singlePayment->setWindowModality(Qt::ApplicationModal);
	singlePayment->setAttribute(Qt::WA_DeleteOnClose);
	singlePayment->setCustomerId(m_customerId);

	singlePayment->getUI().nameText->setText(m_name);
	singlePayment->getUI().phoneText->setText(m_phone);
	singlePayment->getUI().addressText->setText(m_address);
	singlePayment->getUI().commentsText->setText(m_comments);

	singlePayment->getUI().netAmountLbl->setText(m_netAmount);
	singlePayment->getUI().totalBillLbl->setText(m_netAmount);
	singlePayment->getUI().noOfItemsLbl->setText(m_noOfItems);
	singlePayment->getUI().balanceLbl->setText("0.00");

	singlePayment->show();
	close();
}

void ESPayment::slotMultiplePayment()
{
	ESMultiplePayment* multiplePayment = new ESMultiplePayment(m_addBill, 0);
	multiplePayment->setWindowState(Qt::WindowActive);
	multiplePayment->setWindowModality(Qt::ApplicationModal);
	multiplePayment->setAttribute(Qt::WA_DeleteOnClose);
	multiplePayment->setCustomerId(m_customerId);

	multiplePayment->getUI().nameText->setText(m_name);
	multiplePayment->getUI().phoneText->setText(m_phone);
	multiplePayment->getUI().addressText->setText(m_address);
	multiplePayment->getUI().commentsText->setText(m_comments);

	multiplePayment->getUI().netAmountLbl->setText(m_netAmount);
	multiplePayment->getUI().totalBillLbl->setText(m_netAmount);
	multiplePayment->getUI().noOfItemsLbl->setText(m_noOfItems);
	multiplePayment->getUI().balanceLbl->setText("0.00");

	multiplePayment->show();
	close();
}

