#ifndef essinglepayment_h__
#define essinglepayment_h__

#include "ui_singlepayment.h"
#include <QtGui>

class ESSinglePayment : public QWidget
{
	Q_OBJECT

public:
	ESSinglePayment(QWidget *parent = 0);

	~ESSinglePayment();

	Ui::SinglePaymentWidget& getUI() { return ui; };
	void setCustomerId(QString customerId);

	public slots:
	void slotSearch();
	void slotPaymentMethodSelected(QString);
	void slotCalculateBalance();
	void slotFinalizeBill();

private:
	void handleChequePayment(int billId, double netAmount);
	void handleCreditPayment(int billId, double netAmount);
	void handleCashPayment(int billId, double netAmount);
	void finishBill(double netAmount, int billId);
	bool validate();

	Ui::SinglePaymentWidget ui;
	QString m_customerId;
	QString m_selectedPM;

};


#endif // essinglepayment_h__
