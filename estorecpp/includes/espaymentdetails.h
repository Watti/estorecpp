#ifndef espaymentdetails_h__
#define espaymentdetails_h__

#include "ui_paymentdetails.h"
#include <QtGui>

class ESPaymentDetails : public QDialog
{
	Q_OBJECT

public:
	ESPaymentDetails();
	~ESPaymentDetails();

	Ui::PaymentDetailsDialog& getUi() { return ui; }

public slots:
	void slotSearch();

private:
	Ui::PaymentDetailsDialog ui;

};

#endif // espaymentdetails_h__
