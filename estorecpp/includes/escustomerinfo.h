#ifndef escustomerinfo_h__
#define escustomerinfo_h__

#include "ui_customerinfo.h"
#include <QtGui>
#include "QSignalMapper"

class ESCustomerInfo : public QWidget
{
	Q_OBJECT

public:
	ESCustomerInfo(QWidget *parent = 0);
	~ESCustomerInfo();

	Ui::CustomerInfomationWidget& getUI() { return ui; }

	public slots:
	void slotSearch();
	void slotCustomerSelected(int row, int col);
	void slotPopulateCustomerHistory();
	void slotShowPaymentDetails(QString billId);
	void slotDeleteCustomer();
	void slotEditCustomer();

private:
	Ui::CustomerInfomationWidget ui;
	QString m_selectedCustomerId;
	QSignalMapper* m_paymentDetailsMapper;

};

#endif // escustomerinfo_h__
