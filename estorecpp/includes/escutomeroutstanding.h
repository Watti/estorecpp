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
	void slotPay(QString customerId);
	void slotPrev();
	void slotNext();

private:
	float getTotalOutstanding(QString customerId);

	Ui::CustomerOutstandingWidget ui;
	QSignalMapper* m_paymentButtonMapper;
	int m_pageOffset;
	int m_startingLimit;
	int m_totalRecords;
	int m_nextCounter;
	int m_maxNextCount;

};

#endif