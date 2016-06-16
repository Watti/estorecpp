#ifndef CASHBALANCECONFIGURE_H
#define CASHBALANCECONFIGURE_H

#include "ui_cashbalanceconfigure.h"

class ESCashBalanceConfigure : public QWidget
{
	Q_OBJECT

public:
	enum CashType
	{
		EXPENSE = 0,
		INCOME = 1
	};
	ESCashBalanceConfigure(QWidget *parent = 0);
	~ESCashBalanceConfigure();

	Ui::CashBalanceConfigure& getUI();
public slots:
//void startDay();
//void exit();
void slotPettyCash();

private:
	Ui::CashBalanceConfigure ui;
};

#endif