#ifndef ADDBILL_H
#define ADDBILL_H
#include "ui_addbill.h"

class ESAddBill : public QWidget
{
	Q_OBJECT

public:
	ESAddBill(QWidget *parent = 0);
	~ESAddBill();

	Ui::AddBillWidget& getUI() { return ui; };

	public slots:
	void slotShowAddItem();
	void slotStartNewBill();

private:
	Ui::AddBillWidget ui;

};

#endif
