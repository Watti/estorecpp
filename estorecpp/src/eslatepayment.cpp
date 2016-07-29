#include "eslatepayment.h"
#include "QSqlQuery"
#include "entities\tabletextwidget.h"

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
	
	if (ui.doPrintCB->isChecked())
	{
		//print the bill
	}
}
