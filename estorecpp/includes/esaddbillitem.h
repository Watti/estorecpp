#ifndef ADDBILLITEM_H
#define ADDBILLITEM_H
#include "ui_addbillitem.h"
#include <QKeyEvent>
#include <QtGui>

class ESAddBillItem : public QWidget
{
	Q_OBJECT

public:
	ESAddBillItem(QTableWidget* cart, QWidget *parent = 0);
	~ESAddBillItem();

	Ui::AddBillItemWidget& getUI() { return ui; };

	public slots:
	void slotSearch();

protected:
	void keyPressEvent(QKeyEvent * event);

private:
	void addToBill(QString itemCode);

	Ui::AddBillItemWidget ui;
	QTableWidget* m_cart;

};

#endif
