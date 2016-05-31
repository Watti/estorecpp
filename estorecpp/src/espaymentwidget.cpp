#include "espaymentwidget.h"
#include "utility/esdbconnection.h"
#include <QShortcut>
#include "utility/session.h"
#include "esaddbill.h"
#include "easylogging++.h"
#include <QMessageBox>
#include <QDesktopWidget>

ESPayment::ESPayment(ESAddBill* addBill, QWidget *parent /*= 0*/) :
QWidget(parent), m_addBill(addBill)
{
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

	ui.singlePaymentWidget->hide();
	this->resize(600, 1);
	
	new QShortcut(QKeySequence(Qt::Key_Escape), this, SLOT(close()));
	QObject::connect(ui.searchText, SIGNAL(textChanged(QString)), this, SLOT(slotSearch()));
	QObject::connect(ui.cashText, SIGNAL(textChanged(QString)), this, SLOT(slotCalculateBalance()));
	QObject::connect(ui.paymentMethodCombo, SIGNAL(activated(QString)), this, SLOT(slotEnableCardInfo()));
	QObject::connect(ui.okBtn, SIGNAL(clicked()), this, SLOT(finalizeBill()));
	QObject::connect(ui.customers, SIGNAL(cellClicked(int, int)), this, SLOT(slotCustomerSeleced(int, int)));
	QObject::connect(ui.singlePaymentButton, SIGNAL(clicked()), this, SLOT(slotSinglePayment()));
	QObject::connect(ui.multiplePaymentButton, SIGNAL(clicked()), this, SLOT(slotMultiplePayment()));

	QSqlQuery queryPayment("SELECT * FROM payment_method");
	while (queryPayment.next())
	{
		ui.paymentMethodCombo->addItem(queryPayment.value("type").toString(), queryPayment.value("type_id").toInt());
	}

	slotSearch();

	ui.cashText->setFocus();
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

void ESPayment::slotCalculateBalance()
{
	bool isValid = false;
	double cash = 0, cardAmount = 0;
	if (!ui.cashText->text().isEmpty())
	{
		cash = ui.cashText->text().toDouble(&isValid);
		if (!isValid)
		{
			QMessageBox mbox;
			mbox.setIcon(QMessageBox::Warning);
			mbox.setText(QString("Invalid input - Cash"));
			mbox.exec();
			ui.cashText->clear();
			return;
		}
	}
	
	if (ui.paymentMethodCombo->currentText() == "LOYALTY CARD" || ui.paymentMethodCombo->currentText() == "CREDIT CARD")
	{
		/*if (!ui.cardAmountText->text().isEmpty())
		{
			cardAmount = ui.cardAmountText->text().toDouble(&isValid);

			if (!isValid)
			{
				QMessageBox mbox;
				mbox.setIcon(QMessageBox::Warning);
				mbox.setText(QString("Invalid input - Card Amount"));
				mbox.exec();
				ui.cardAmountText->clear();
				return;
			}
		}*/
	}
	double amount = ui.totalBillLbl->text().toDouble();

	ui.balanceLbl->setText(QString::number(cash - (amount - cardAmount), 'f', 2));

	if (ui.cashText->text().isEmpty())
	{
		ui.balanceLbl->clear();
	}
}

void ESPayment::slotEnableCardInfo()
{
	/*if (ui.paymentMethodCombo->currentText() == "LOYALTY CARD" || ui.paymentMethodCombo->currentText() == "CREDIT CARD")
	{
		ui.cardNoText->show();
		ui.cardNoLbl->show();
		ui.cardAmountLbl->show();
		ui.cardAmountText->show();
	}
	else
	{
		ui.cardNoText->hide();
		ui.cardNoLbl->hide();
		ui.cardAmountLbl->hide();
		ui.cardAmountText->hide();
	}*/
}

void ESPayment::finalizeBill()
{
	QString billId = ES::Session::getInstance()->getBillId();
	QString netAmount = ui.totalBillLbl->text();
	QString paymentType = ui.paymentMethodCombo->currentData().toString();

	if (netAmount > 0)
	{
		/*QString queryUpdateStr("UPDATE bill set amount = " + netAmount + ", payment_method = " + paymentType + " , status = 1 WHERE bill_id = " + billId );
		QSqlQuery query;
		if (ui.paymentMethodCombo->currentText() == "LOYALTY CARD" || ui.paymentMethodCombo->currentText() == "CREDIT CARD")
		{
			QString cardNo = ui.cardNoText->text();
			QString cardAmount = ui.cardAmountText->text();
			QString cardPaymentQryStr("INSERT INTO card_payment (bill_id , amount, card_no, payment_type_id ) VALUES ("+billId+ ", "+cardAmount+","+cardNo+", "+paymentType+")");
			QSqlQuery queryCardPayment;
			if (!queryCardPayment.exec(cardPaymentQryStr))
			{
				LOG(ERROR) << cardPaymentQryStr.toLatin1().data();
			}
		}
		if (query.exec(queryUpdateStr))
		{
			this->close();
			m_addBill->resetBill();
		}
		else
		{
			LOG(ERROR) << queryUpdateStr.toLatin1().data();
		}*/
	}
	else
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Warning);
		mbox.setText(QString("Empty Bill ! ! !"));
		mbox.exec();
		this->close();
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
		}
	}
	else
	{
		m_customerId = "";
	}

}

void ESPayment::slotSinglePayment()
{
	ui.customerSearchWidget->hide();
	ui.paymentTypes->hide();
	resize(500, 1);
	ui.singlePaymentWidget->show();	
	update();
	resize(500, 1);
}

void ESPayment::slotMultiplePayment()
{

}

