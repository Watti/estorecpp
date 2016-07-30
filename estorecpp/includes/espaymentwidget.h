#ifndef espaymentwidget_h__
#define espaymentwidget_h__

#include "ui_paymentui.h"
#include <QtGui>
class ESAddBill;
class ESPayment : public QWidget
{
	Q_OBJECT

public:
	ESPayment(ESAddBill* addBill , QWidget *parent = 0, bool isReturnBill = false);

	~ESPayment();

	Ui::PaymentWidget& getUI() { return ui; };

	void setNetAmount(QString netAmount) { m_netAmount = netAmount; }
	void setNoOfItems(QString noOfItems) { m_noOfItems = noOfItems; }

	public slots:
	void slotSearch();
	void slotCustomerSeleced(int,int);
	void slotSinglePayment();
	void slotMultiplePayment();

	QString getTotalAmount() const;
	void setTotalAmount(QString val);
	
private:

	Ui::PaymentWidget ui;
	ESAddBill* m_addBill;
	QString m_customerId;
	QString m_name, 
		m_phone, 
		m_address, 
		m_comments;

	QString m_netAmount;
	QString m_totalAmount;
	QString m_noOfItems;
	bool m_isReturnBill;

};

#endif // espaymentwidget_h__
