#include "escashbalancestatus.h"

ESCashBalanceStatus::ESCashBalanceStatus(QWidget *parent) : QWidget(parent)
{
	ui.setupUi(this); 
}

ESCashBalanceStatus::~ESCashBalanceStatus()
{}

Ui::CashBalanceStatus& ESCashBalanceStatus::getUI()
{
	return ui;
}