#include "essinglepayment.h"
#include <QMessageBox>
#include "utility\session.h"


ESSinglePayment::ESSinglePayment(QWidget *parent /*= 0*/) : QWidget(parent)
{
	ui.setupUi(this);

	slotPaymentMethodSelected(ui.paymentMethodCombo->currentText());

	QObject::connect(ui.paymentMethodCombo, SIGNAL(activated(QString)), this, SLOT(slotPaymentMethodSelected(QString)));
}

ESSinglePayment::~ESSinglePayment()
{

}

void ESSinglePayment::slotSearch()
{

}


void ESSinglePayment::slotCalculateBalance()
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


void ESSinglePayment::slotFinalizeBill()
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

void ESSinglePayment::slotPaymentMethodSelected(QString pmMethod)
{
	if (pmMethod == "CREDIT")
	{
		ui.lbl1->hide();
		ui.lbl2->hide();
		ui.dateLbl->show(); // due date

		ui.txt1->hide();
		ui.txt2->hide();
		ui.dateEdit->show();
	}
	else if (pmMethod == "CHEQUE")
	{
		ui.lbl1->show(); // cheque no
		ui.lbl2->show(); // bank
		ui.dateLbl->show(); // due date

		ui.txt1->show();
		ui.txt2->show();
		ui.dateEdit->show();

		ui.lbl1->setText("Cheque No. :  ");
		ui.lbl2->setText("Bank :  ");
	}
	else if (pmMethod == "CREDIT CARD")
	{
		ui.lbl1->show(); // card no
		ui.lbl2->hide();
		ui.dateLbl->hide();

		ui.txt1->show();
		ui.txt2->hide();
		ui.dateEdit->hide();

		ui.lbl1->setText("Card No. :  ");
	}
	else if (pmMethod == "LOYALITY CARD")
	{
		ui.lbl1->show(); // card no
		ui.lbl2->hide();
		ui.dateLbl->hide();

		ui.txt1->show();
		ui.txt2->hide();
		ui.dateEdit->hide();

		ui.lbl1->setText("Card No. :  ");
	}
	else
	{
		ui.lbl1->hide();
		ui.lbl2->hide();
		ui.dateLbl->hide();

		ui.txt1->hide();
		ui.txt2->hide();
		ui.dateEdit->hide();
	}

	m_selectedPM = pmMethod;
	resize(500, 1);
	adjustSize();
}
