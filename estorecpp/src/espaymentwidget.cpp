#include "espaymentwidget.h"
#include "utility/esdbconnection.h"
#include <QShortcut>

ESPayment::ESPayment(QWidget *parent /*= 0*/) :
QWidget(parent)
{
	ui.setupUi(this);

	ui.cardNoLbl->hide();
	ui.cardNoText->hide();

	new QShortcut(QKeySequence(Qt::Key_Escape), this, SLOT(close()));
	QObject::connect(ui.cashText, SIGNAL(textChanged(QString)), this, SLOT(slotCalculateBalance()));
	QObject::connect(ui.paymentMethodCombo, SIGNAL(activated(QString)), this, SLOT(slotEnableCardInfo()));

	QSqlQuery queryPayment("SELECT * FROM payment");
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
	double cash = ui.cashText->text().toDouble();
	double amount = ui.totalBillLbl->text().toDouble();

	ui.balanceLbl->setText(QString::number(cash - amount, 'f', 2));
}

void ESPayment::slotEnableCardInfo()
{
	if (ui.paymentMethodCombo->currentData().toInt() == 4)
	{
		ui.cardNoText->show();
		ui.cardNoLbl->show();
	}
	else
	{
		ui.cardNoText->hide();
		ui.cardNoLbl->hide();
	}
}
