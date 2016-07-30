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

	public slots:
	void slotSelect();
	void slotItemDoubleClicked(int row, int col);
	void slotRemove(QString itemCode);
	void slotPrintReturnBill();
	void slotQuantityCellUpdated(QString, int, int);
	void slotInterestChanged();
	void slotShowAddItem();
	void slotStartBill();

	void slotPrint(QPrinter* printer);

private:
	void showTotal();
	void updateReturnItemTable();
	void setEnabled(bool enable);

	Ui::ReturnItems ui;
	QSignalMapper* m_removeButtonSignalMapper;
	int m_idGenerator;
	KDReports::Report report;
	int m_oldBillId;
	double m_total;
	bool m_hasInterest;

	ES::ReturnBill m_bill;
};

#endif