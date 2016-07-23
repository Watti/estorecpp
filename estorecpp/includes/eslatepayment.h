#ifndef eslatepayment_h__
#define eslatepayment_h__

#include "ui_latepaymentwidget.h"
#include <QtGui>

class ESLatePayment : public QWidget
{
	Q_OBJECT
public:
	ESLatePayment(QString billId, QWidget *parent = 0);
	~ESLatePayment();

	Ui::LatePaymentWidget& getUI() { return ui; }

public slots:
	void slotSearch();

private:
	Ui::LatePaymentWidget ui;
	QString m_billId;

};

#endif // eslatepayment_h__
