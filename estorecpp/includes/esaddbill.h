#ifndef ADDBILL_H
#define ADDBILL_H
#include "ui_addbill.h"
#include "QSignalMapper"

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
	void slotRemoveItem(QString);

	void resetBill();

	QSignalMapper* getRemoveButtonSignalMapper() const { return m_removeButtonSignalMapper; }

private:
	Ui::AddBillWidget ui;
	void calculateAndDisplayTotal();
	QSignalMapper* m_removeButtonSignalMapper;
};

#endif
