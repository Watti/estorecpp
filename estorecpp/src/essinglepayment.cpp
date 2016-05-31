#include "essinglepayment.h"
#include <QMessageBox>
#include "utility\session.h"
#include "QSqlQuery"


ESSinglePayment::ESSinglePayment(QWidget *parent /*= 0*/) : QWidget(parent)
{
	ui.setupUi(this);

	slotPaymentMethodSelected(ui.paymentMethodCombo->currentText());

	QObject::connect(ui.paymentMethodCombo, SIGNAL(activated(QString)), this, SLOT(slotPaymentMethodSelected(QString)));
	QObject::connect(ui.cashText, SIGNAL(textChanged(QString)), this, SLOT(slotCalculateBalance()));
	QObject::connect(ui.okBtn, SIGNAL(clicked()), this, SLOT(slotFinalizeBill()));
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
	if (!validate())
		return;

	QString billIdStr = ES::Session::getInstance()->getBillId();
	QString netAmountStr = ui.totalBillLbl->text();
	QString paymentType = ui.paymentMethodCombo->currentText();
	bool isValid = false;

	double netAmount = netAmountStr.toDouble(&isValid);
	if (!isValid || netAmount < 0)
	{
		return;
	}		
	int billId = billIdStr.toInt(&isValid);
	if (!isValid)
	{
		return;
	}

	if (paymentType == "CASH")
	{
		QSqlQuery query;
		query.prepare("INSERT INTO payment (bill_id, total_amount, cash) VALUES (?, ?, 1)");
		query.addBindValue(billId);
		query.addBindValue(netAmount);
		if (query.exec())
		{
			int lastInsertedId = query.lastInsertId().toInt();
			QSqlQuery q;
			q.prepare("INSERT INTO cash (payment_id, amount) VALUES (?, ?)");
			q.addBindValue(lastInsertedId);
			q.addBindValue(netAmount);
			if (!q.exec())
			{
				QMessageBox mbox;
				mbox.setIcon(QMessageBox::Critical);
				mbox.setText(QString("Failed"));
				mbox.exec();	
			}
		}
		else
		{
			QMessageBox mbox;
			mbox.setIcon(QMessageBox::Critical);
			mbox.setText(QString("Failed"));
			mbox.exec();
		}
	}

	this->close();
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

		ui.paymentType->setText("Amount :  ");
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

		ui.paymentType->setText("Amount :  ");
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
		ui.paymentType->setText("Amount :  ");
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
		ui.paymentType->setText("Amount :  ");
	}
	else if (pmMethod == "CASH")
	{
		ui.lbl1->hide();
		ui.lbl2->hide();
		ui.dateLbl->hide();

		ui.txt1->hide();
		ui.txt2->hide();
		ui.dateEdit->hide();

		ui.paymentType->setText("Cash :  ");
	}

	m_selectedPM = pmMethod;
	resize(500, 1);
	adjustSize();
}

bool ESSinglePayment::validate()
{
	return true;
}
