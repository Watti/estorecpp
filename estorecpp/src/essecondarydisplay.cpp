#include "essecondarydisplay.h"

ESSecondaryDisplay::ESSecondaryDisplay(QWidget *parent /*= 0*/) : QWidget(parent)
{
	ui.setupUi(this);

	ui.baseWidget->hide();
	ui.thankYouLabel->show();
}

ESSecondaryDisplay::~ESSecondaryDisplay()
{

}

void ESSecondaryDisplay::slotSearch()
{

}

void ESSecondaryDisplay::startBill()
{
	ui.baseWidget->show();
	ui.thankYouLabel->hide();
}

void ESSecondaryDisplay::endBill()
{
	ui.baseWidget->hide();
	ui.thankYouLabel->show();
}

