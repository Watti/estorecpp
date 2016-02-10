#ifndef ADDBILLITEM_H
#define ADDBILLITEM_H
#include "ui_addbillitem.h"
#include <QKeyEvent>
#include <QtGui>
#include "esaddbill.h"
#include "QSignalMapper"

class ESAddBillItem : public QWidget
{
	Q_OBJECT

public:
	ESAddBillItem(ESAddBill* cart, QWidget *parent = 0);
	~ESAddBillItem();

	Ui::AddBillItemWidget& getUI() { return ui; };

	public slots:
	void slotSearch();
	void slotRemove(QString);

protected:
	void keyPressEvent(QKeyEvent * event);

private:
	void addToBill(QString itemCode);

	Ui::AddBillItemWidget ui;
	ESAddBill* m_cart;
	QSignalMapper* m_removeButtonSignalMapper;

};

#endif
