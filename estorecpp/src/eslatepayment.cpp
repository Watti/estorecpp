#include "eslatepayment.h"
#include "QSqlQuery"
#include "entities\tabletextwidget.h"
#include "QMessageBox"

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
}

ESLatePayment::~ESLatePayment()
{

}

void ESLatePayment::slotOk()
{
	bool valid = false;
	int index = ui.tabWidget->currentIndex();
	if (index == 0)
	{
		//cash
		float payingAmount = ui.payingAmountCash->text().toFloat(&valid);
		if (!valid)
		{
			QMessageBox mbox;
			mbox.setIcon(QMessageBox::Critical);
			mbox.setText(QString("Please enter a number"));
			mbox.exec();
			return;
		}

		QString remarks = ui.remarksCash->toPlainText();

		QString qryStr("");
	}
	else if (index == 1)
	{
		//cheque
		valid = false;
		float payingAmount = ui.payingAmountCheque->text().toFloat(&valid);
		QString chNo = ui.chequeNo->text();
		QString dueDate = ui.dueDate->date().toString("yyyy-MM-dd");
		QString bank = ui.bank->text();
		QString remarks = ui.remarksCheque->toPlainText();
		if (!valid)
		{
			QMessageBox mbox;
			mbox.setIcon(QMessageBox::Critical);
			mbox.setText(QString("Please enter a number"));
			mbox.exec();
			return;
		}

	}
	if (ui.doPrintCB->isChecked())
	{
		//print the bill
	}
}

void ESLatePayment::setCustomerId(QString customerId)
{
	m_customerId = customerId;
}
