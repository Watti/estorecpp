#ifndef ESORDERCHECKIN_H
#define ESORDERCHECKIN_H

#include "ui_ordercheckinwidget.h"

class ESOrderCheckIn : public QWidget
{
	Q_OBJECT

public:
	ESOrderCheckIn(QString orderId, QWidget *parent = 0);
	
	~ESOrderCheckIn();
	
	public slots:
	void slotSearch();
	void slotItemSelected(int row, int col);
	void slotAddToStock();
	void slotRemoveFromStock();
	void slotHoldOrder();
	void slotFinalizeOrder();
	void slotGenerateGRN();
	void slotCellDoubleClicked(int, int);
private:
	Ui::OrderCheckInWidget ui;
	QString m_orderId;

};

#endif