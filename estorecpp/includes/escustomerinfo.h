#ifndef escustomerinfo_h__
#define escustomerinfo_h__

#include "ui_customerinfo.h"
#include <QtGui>

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

private:
	Ui::CustomerInfomationWidget ui;

};

#endif // escustomerinfo_h__
