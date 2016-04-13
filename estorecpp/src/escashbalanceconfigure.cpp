#include "escashbalanceconfigure.h"

ESCashBalanceConfigure::ESCashBalanceConfigure(QWidget *parent) : QWidget(parent)
{
	ui.setupUi(this);
	ui.userLabel->setText("Hello");
}

ESCashBalanceConfigure::~ESCashBalanceConfigure()
{}

Ui::CashBalanceConfigure& ESCashBalanceConfigure::getUI()
{
	return ui;
}