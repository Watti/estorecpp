#ifndef esaddbillitem2_h__
#define esaddbillitem2_h__

#include "ui_addbillitem.h"
#include <QKeyEvent>
#include <QtGui>
#include "esaddbill.h"
#include "entities\returnbill.h"

class ESReturnItems;
class ESAddBillItem2 : public QWidget
{
	Q_OBJECT

public:
	ESAddBillItem2(ES::ReturnBill& cart, ESReturnItems* widget, QWidget *parent = 0);
	~ESAddBillItem2();

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
	ES::ReturnBill& m_cart;
	ESReturnItems* m_widget;

};

#endif // esaddbillitem2_h__
