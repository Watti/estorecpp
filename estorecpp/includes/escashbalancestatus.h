#ifndef CASHBALANCESTATUS_H
#define CASHBALANCESTATUS_H

#include "ui_cashbalancestatus.h"

class ESCashBalanceStatus : public QWidget
{
	Q_OBJECT

public:
	ESCashBalanceStatus(QWidget *parent = 0);
	~ESCashBalanceStatus();

	Ui::CashBalanceStatus& getUI();

private:
	Ui::CashBalanceStatus ui;

};

#endif