#ifndef ADDBILLITEM_H
#define ADDBILLITEM_H
#include "ui_addbillitem.h"
#include <QKeyEvent>
#include <QtGui>
#include "esaddbill.h"

class ESAddBillItem : public QWidget
{
	Q_OBJECT

public:
	ESAddBillItem(ESAddBill* cart, QWidget *parent = 0);
	~ESAddBillItem();

	Ui::AddBillItemWidget& getUI() { return ui; };

	public slots:
	void slotSearch();
	void slotHideImages();
	void slotItemDoubleClicked(int, int);
	void focus();

protected:
	void keyPressEvent(QKeyEvent * event);

private:
	void addToBill(QString stockId);

	Ui::AddBillItemWidget ui;
	ESAddBill* m_cart;

};

#endif
