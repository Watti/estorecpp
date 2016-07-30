#ifndef esaddcustomer_h__
#define esaddcustomer_h__

#include "ui_addcustomer.h"
#include <QtGui>

class ESAddCustomer : public QWidget
{
	Q_OBJECT

public:
	ESAddCustomer(QWidget *parent = 0);
	~ESAddCustomer();

	Ui::AddCustomerWidget& getUI() { return ui; }
	void setUpdate(bool update);
	void setCustomerId(QString val);
	void setOutstandingId(QString val);

	public slots:
	void slotProcess();

private:
	Ui::AddCustomerWidget ui;
	bool m_update;
	QString m_id;
	QString m_outstandingId;
};


#endif // esaddcustomer_h__
