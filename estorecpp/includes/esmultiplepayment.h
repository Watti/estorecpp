#ifndef esmultiplepayment_h__
#define esmultiplepayment_h__

#include "ui_multiplepayment.h"
#include <QtGui>

class ESMultiplePayment : public QWidget
{
	Q_OBJECT

public:
	ESMultiplePayment(QWidget *parent = 0);

	~ESMultiplePayment();

	Ui::MultiplePaymentWidget& getUI() { return ui; };
	void setCustomerId(QString customerId);

	public slots:
	void slotSearch();
	void slotPaymentMethodSelected();
	void slotCalculateBalance();
	void slotAdd();

private:
	bool validate();
	Ui::MultiplePaymentWidget ui;
	QString m_customerId;
	QString m_paymentType;
	QSignalMapper* m_removeButtonSignalMapper;

};

#endif // esmultiplepayment_h__
