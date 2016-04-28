#ifndef espaymentwidget_h__
#define espaymentwidget_h__

#include "ui_paymentui.h"
#include <QtGui>

class ESPayment : public QWidget
{
	Q_OBJECT

public:
	ESPayment(QWidget *parent = 0);
	~ESPayment();

	Ui::PaymentWidget& getUI() { return ui; };

	public slots:
	void slotSearch();
	void slotCalculateBalance();
	
private:

	Ui::PaymentWidget ui;

};

#endif // espaymentwidget_h__
