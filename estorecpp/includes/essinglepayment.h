#ifndef essinglepayment_h__
#define essinglepayment_h__

#include "ui_singlepayment.h"
#include <QtGui>
#include "esaddbill.h"

class ESSinglePayment : public QWidget
{
	Q_OBJECT

public:
	ESSinglePayment(ESAddBill* addBill, QWidget *parent = 0);

	~ESSinglePayment();

	Ui::SinglePaymentWidget& getUI() { return ui; };
	void setCustomerId(QString customerId);

	public slots:
	void slotSearch();
	void slotPaymentMethodSelected(QString);
	void slotCalculateBalance();
	void slotFinalizeBill();
	void slotInterestChanged();

private:
	void handleChequePayment(int billId, double netAmount);
	void handleCreditPayment(int billId, double netAmount);
	void handleCashPayment(int billId, double netAmount);
	void finishBill(double netAmount, int billId);
	bool validate();

	Ui::SinglePaymentWidget ui;
	QString m_customerId;
	QString m_selectedPM;
	ESAddBill* m_addBill;

};


#endif // essinglepayment_h__
