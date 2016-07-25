#ifndef eslatepayment_h__
#define eslatepayment_h__

#include "ui_latepaymentwidget.h"
#include <QtGui>

class ESLatePayment : public QWidget
{
	Q_OBJECT
public:
	ESLatePayment(QWidget *parent = 0);
	~ESLatePayment();

	Ui::LatePaymentWidget& getUI() { return ui; }
	void setCustomerId(QString customerId);

public slots:
	void slotOk();

private:
	Ui::LatePaymentWidget ui;

};

#endif // eslatepayment_h__
