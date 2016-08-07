#ifndef ESRETURNITEMWIDGET_H
#define ESRETURNITEMWIDGET_H
#include "ui_ReturnItems.h"
#include "QPrinter"
#include "KDReportsReport.h"
#include "QSignalMapper"
#include "entities\returnbill.h"

class ESReturnItems : public QWidget
{
	Q_OBJECT

public:
	ESReturnItems(QWidget *parent = 0);
	~ESReturnItems();
	Ui::ReturnItems& getUI();
	void updateNewItemTable();
	long getBillId() { return m_bill.getBillId(); }
	long getOldBillId() { return m_bill.getOldBillId(); }
	void updateDatabase();

	public slots:
	void slotSelect();
	void slotItemDoubleClicked(int row, int col);
	void slotRemove(QString itemCode);
	void slotRemoveNewItem(int rowId);
	void slotPrintReturnBill();
	void slotQuantityCellUpdated(QString, int, int);
	void slotNewItemQuantityCellUpdated(QString, int, int);
	void slotInterestChanged();
	void slotNewInterestChanged();
	void slotShowAddItem();
	void slotStartBill();
	void slotCommit();

	void slotPrint(QPrinter* printer);

protected:
	void keyPressEvent(QKeyEvent* evt);

private:
	void showTotal();
	void updateReturnItemTable();
	void setEnabled(bool enable);

	Ui::ReturnItems ui;
	QSignalMapper* m_removeButtonSignalMapper;
	QSignalMapper* m_removeNewItemButtonSignalMapper;
	int m_idGenerator;
	KDReports::Report report;
	int m_oldBillId;
	bool m_hasInterest;
	QString m_customerId;
	ES::ReturnBill m_bill;
	float m_billOutstanding;
};

#endif