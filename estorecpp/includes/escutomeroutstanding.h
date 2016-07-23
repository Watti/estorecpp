#ifndef ES_CUSTOMER_OUTSTANDING_H
#define ES_CUSTOMER_OUTSTANDING_H
#include "ui_customeroutstanding.h"
#include <QStandardItemModel>
#include <QtGui>

class ESCustomerOutstanding : public QWidget
{
	Q_OBJECT

public:
	ESCustomerOutstanding(QWidget *parent = 0);
	~ESCustomerOutstanding();

	Ui::CustomerOutstandingWidget& getUI() { return ui; }

	public slots:
	void slotSearchCustomers();
	void slotCustomerSelected(int row, int col);
	void populateCustomerOutstanding();
	void slotPay(QString billId);
	void slotMultiPay();

private:
	float getTotalOutstanding(QString customerId);

	Ui::CustomerOutstandingWidget ui;
	QString m_selectedCustomerId;
	QSignalMapper* m_paymentDetailsMapper;

};

#endif