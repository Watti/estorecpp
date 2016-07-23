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
	void addBill(QString billId);
	void setCustomerId(QString customerId);

public slots:
	void slotAddBill();
	void slotRemoveBill(QString billId);
	void slotOk();

	void slotAddCash();
	void slotAddCredit();
	void slotAddCheque();
	void slotAddCard();
	void slotAddLoyalty();

private:
	Ui::LatePaymentWidget ui;
	QString m_selectedCustomerId;
	QSignalMapper* m_removeButtonMapper;

};

#endif // eslatepayment_h__