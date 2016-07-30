#include "eslatepayment.h"
#include "QSqlQuery"
#include "entities\tabletextwidget.h"
#include "QMessageBox"
#include "escutomeroutstanding.h"

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
	QObject::connect(ui.okBtn, SIGNAL(clicked()), this, SLOT(slotOk()));
	QObject::connect(ui.payingAmountCash, SIGNAL(textChanged(QString)), this, SLOT(slotCalculateCashBalance()));
	QObject::connect(ui.payingAmountCheque, SIGNAL(textChanged(QString)), this, SLOT(slotCalculateChequeBalance()));
}

ESLatePayment::~ESLatePayment()
{

}

void ESLatePayment::slotOk()
{
	bool valid = false;
	bool isCheque = false;
	QString chNo = "", dueDate = "", bank = "", remarks = "";
	float payingAmount = 0;
	int index = ui.tabWidget->currentIndex();
	if (index == 0)
	{
		//cash
		payingAmount = ui.payingAmountCash->text().toFloat(&valid);
		if (!valid)
		{
			QMessageBox mbox;
			mbox.setIcon(QMessageBox::Critical);
			mbox.setText(QString("Please enter a number"));
			mbox.exec();
			return;
		}

		remarks = ui.remarksCash->toPlainText();
	}
	else if (index == 1)
	{
		//cheque
		valid = false;
		payingAmount = ui.payingAmountCheque->text().toFloat(&valid);
		chNo = ui.chequeNo->text();
		dueDate = ui.dueDate->date().toString("yyyy-MM-dd");
		bank = ui.bank->text();
		remarks = ui.remarksCheque->toPlainText();
		if (!valid)
		{
			QMessageBox mbox;
			mbox.setIcon(QMessageBox::Critical);
			mbox.setText(QString("Please enter a number"));
			mbox.exec();
			return;
		}
		isCheque = true;
	}
	QString qryStr("SELECT * FROM customer_outstanding WHERE customer_id = " + m_customerId);
	QSqlQuery queryOutstanding(qryStr);
	if (queryOutstanding.next())
	{
		float currentOutstanding = queryOutstanding.value("current_outstanding").toFloat();
		if (payingAmount > currentOutstanding)
		{
			payingAmount = currentOutstanding;
		}
		float newOutstanding = currentOutstanding - payingAmount;
		QSqlQuery qOutstandingUpdate;
		QString updateQryStr("UPDATE customer_outstanding SET current_outstanding = " + QString::number(newOutstanding)+" WHERE customer_id = "+m_customerId);
		if (qOutstandingUpdate.exec(updateQryStr))
		{
			if (isCheque)
			{
				QString qStr("INSERT INTO cheque_information (customer_id, cheque_number, bank, due_date, amount) VALUES (" +
					m_customerId + ",'" + chNo + "','" + bank + "','" + dueDate + "', '" + QString::number(payingAmount) + "')");
				QSqlQuery qryCheckInsert;
				if (!qryCheckInsert.exec(qStr))
				{
					QMessageBox mbox;
					mbox.setIcon(QMessageBox::Critical);
					mbox.setText(QString("Insertion to cheque_information has been failed ! ! !"));
					mbox.exec();
				}
			}
			if (ui.doPrintCB->isChecked())
			{
				//print the bill

			}
		}
		else
		{
			QMessageBox mbox;
			mbox.setIcon(QMessageBox::Critical);
			mbox.setText(QString("Error !!! Unable to update the outstanding"));
			mbox.exec();
			return;
		}
		ESCustomerOutstanding* parent = static_cast<ESCustomerOutstanding*>(parent);
		if (parent)
			parent->slotSearchCustomers();
		this->close();
	}
}

void ESLatePayment::setCustomerId(QString customerId)
{
	m_customerId = customerId;
}

void ESLatePayment::slotCalculateCashBalance()
{
	bool valid = false;
	if (ui.payingAmountCash->text().isEmpty())
	{
		ui.remainingAmountCash->setText(ui.currentOutstandingCash->text());
	}
	float payingAmount = ui.payingAmountCash->text().toFloat(&valid);
	if (valid)
	{
		float remainingAmount = ui.currentOutstandingCash->text().toFloat() - payingAmount;
		ui.remainingAmountCash->setText(QString::number(remainingAmount, 'f', 2));
	}
}

void ESLatePayment::slotCalculateChequeBalance()
{
	bool valid = false;
	if (ui.payingAmountCheque->text().isEmpty())
	{
		ui.remainingAmountCheque->setText(ui.currentOutstandingCheque->text());
	}
	float payingAmount = ui.payingAmountCheque->text().toFloat(&valid);
	if (valid)
	{
		float remainingAmount = ui.currentOutstandingCheque->text().toFloat() - payingAmount;
		ui.remainingAmountCheque->setText(QString::number(remainingAmount, 'f', 2));
	}
}
