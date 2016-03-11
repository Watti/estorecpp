#include "escashbalanceconfigure.h"

ESCashBalanceConfigure::ESCashBalanceConfigure(QWidget *parent) : QWidget(parent)
{
	ui.setupUi(this);
}

ESCashBalanceConfigure::~ESCashBalanceConfigure()
{}

Ui::CashBalanceConfigure& ESCashBalanceConfigure::getUI()
{
	return ui;
}