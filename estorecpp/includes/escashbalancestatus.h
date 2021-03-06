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
	void displayStatus();

public slots:
void dayOff();
void slotCalculateTotal();

private:
	Ui::CashBalanceStatus ui;

};

#endif