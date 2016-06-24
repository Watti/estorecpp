#ifndef ADDBILL_H
#define ADDBILL_H
#include "ui_addbill.h"
#include "QSignalMapper"
#include <QSqlQuery>

class ESAddBill : public QWidget
{
	Q_OBJECT

public:
	ESAddBill(QWidget *parent = 0);
	~ESAddBill();

	Ui::AddBillWidget& getUI();
	QSignalMapper* getRemoveButtonSignalMapper() const { return m_removeButtonSignalMapper; }

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
	void slotOpenDisplay();

	void slotCellDoubleClicked(int row, int col);

	void slotQuantityCellUpdated(QString txt, int row, int col);
	void slotSellingPriceUpdated(QString txt, int row, int col);

	void resetBill();
	void proceedPendingBill(QString billId);

protected:
	void keyPressEvent(QKeyEvent* evt);

private:
	void checkAndContinuePendingBill();
	void calculateAndDisplayTotal();
	void populateTable(QSqlQuery &queryBillTable);

	Ui::AddBillWidget ui;
	QSignalMapper* m_removeButtonSignalMapper;
};

#endif
