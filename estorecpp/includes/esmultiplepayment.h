#ifndef esmultiplepayment_h__
#define esmultiplepayment_h__

#include "ui_multiplepayment.h"
#include <QtGui>
#include "esaddbill.h"

class ESMultiplePayment : public QWidget
{
	Q_OBJECT

public:
	ESMultiplePayment(ESAddBill* addBill, QWidget *parent = 0);

	~ESMultiplePayment();

	Ui::MultiplePaymentWidget& getUI() { return ui; };
	void setCustomerId(QString customerId);

	public slots:
	void slotSearch();
	void slotPaymentMethodSelected();
	void slotCalculateBalance();
	void slotAdd();
	void slotRemove(int row);
	void slotFinalizeBill();

private:
	bool validate();
	void finishBill(double netAmount, int billId);
	void printBill(int billId, float total);

	Ui::MultiplePaymentWidget ui;
	QString m_customerId;
	QString m_paymentType;
	QSignalMapper* m_removeButtonSignalMapper;
	ESAddBill* m_addBill;

};

#endif // esmultiplepayment_h__
