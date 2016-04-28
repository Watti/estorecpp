#include "espaymentwidget.h"

ESPayment::ESPayment(QWidget *parent /*= 0*/) :
QWidget(parent)
{
	ui.setupUi(this);

	ui.cardNoLbl->hide();
	ui.cardNoText->hide();
}

ESPayment::~ESPayment()
{

}

void ESPayment::slotSearch()
{

}
