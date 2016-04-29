#ifndef espaymentwidget_h__
#define espaymentwidget_h__

#include "ui_paymentui.h"
#include <QtGui>
class ESAddBill;
class ESPayment : public QWidget
{
	Q_OBJECT

public:
	ESPayment(ESAddBill* addBill , QWidget *parent = 0);
	~ESPayment();

	Ui::PaymentWidget& getUI() { return ui; };

	public slots:
	void slotSearch();
	void slotCalculateBalance();
	void slotEnableCardInfo();
	void finalizeBill();
	
private:

	Ui::PaymentWidget ui;
	ESAddBill* m_addBill;

};

#endif // espaymentwidget_h__
