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

	public slots:
	void slotProcess();

private:
	Ui::AddCustomerWidget ui;
	bool m_update;
	QString m_id;

};


#endif // esaddcustomer_h__
