#include "espaymentwidget.h"
#include "utility/esdbconnection.h"
#include <QShortcut>
#include "utility/session.h"
#include "esaddbill.h"
#include "easylogging++.h"
#include "QMessageBox"

ESPayment::ESPayment(ESAddBill* addBill, QWidget *parent /*= 0*/) :
QWidget(parent), m_addBill(addBill)
{
	ui.setupUi(this);

	ui.cardNoLbl->hide();
	ui.cardNoText->hide();
	ui.cardAmountLbl->hide();
	ui.cardAmountText->hide();

	new QShortcut(QKeySequence(Qt::Key_Escape), this, SLOT(close()));
	QObject::connect(ui.cashText, SIGNAL(textChanged(QString)), this, SLOT(slotCalculateBalance()));
	QObject::connect(ui.cardAmountText, SIGNAL(textChanged(QString)), this, SLOT(slotCalculateBalance()));
	QObject::connect(ui.paymentMethodCombo, SIGNAL(activated(QString)), this, SLOT(slotEnableCardInfo()));
	QObject::connect(ui.okBtn, SIGNAL(clicked()), this, SLOT(finalizeBill()));

	QSqlQuery queryPayment("SELECT * FROM payment_method");
	QStringList catogory;
	while (queryPayment.next())
	{
		ui.paymentMethodCombo->addItem(queryPayment.value("type").toString(), queryPayment.value("type_id").toInt());
	}
}

ESPayment::~ESPayment()
{

}

void ESPayment::slotSearch()
{

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
		if (!ui.cardAmountText->text().isEmpty())
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
		}
	}
	double amount = ui.totalBillLbl->text().toDouble();

	ui.balanceLbl->setText(QString::number(cash - (amount - cardAmount), 'f', 2));
}

void ESPayment::slotEnableCardInfo()
{
	if (ui.paymentMethodCombo->currentText() == "LOYALTY CARD" || ui.paymentMethodCombo->currentText() == "CREDIT CARD")
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
	}
}

void ESPayment::finalizeBill()
{
	QString billId = ES::Session::getInstance()->getBillId();
	QString netAmount = ui.totalBillLbl->text();
	QString paymentType = ui.paymentMethodCombo->currentData().toString();

	if (netAmount > 0)
	{
		QString cardNo = ui.cardNoText->text();
		QString queryUpdateStr("UPDATE bill set amount = " + netAmount + ", payment_method = " + paymentType + ", card_no = "+ cardNo + " , status = 1 WHERE bill_id = " + billId );
		QSqlQuery query;;
		if (query.exec(queryUpdateStr))
		{
			this->close();
			m_addBill->resetBill();
		}
		else
		{
			LOG(ERROR) << queryUpdateStr.toLatin1().data();
		}
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
