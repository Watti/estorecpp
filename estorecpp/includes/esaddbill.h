#ifndef ADDBILL_H
#define ADDBILL_H
#include "ui_addbill.h"

class ESAddBill : public QWidget
{
	Q_OBJECT

public:
	ESAddBill(QWidget *parent = 0);
	~ESAddBill();

	Ui::AddBillWidget& getUI();;

	public slots:
	void slotShowAddItem();
	void slotStartNewBill();
	void showTime();
	void slotSearch();
	void slotReturnPressed(QString saleId, int row);
	void slotCommit();
	void slotSuspend();
	void slotCancel();

	void resetBill();


private:
	Ui::AddBillWidget ui;
	void calculateAndDisplayTotal();
};

#endif
