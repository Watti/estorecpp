#ifndef ESMAINWINDOW_H
#define ESMAINWINDOW_H

#include <QtWidgets/QMainWindow>
#include "ui_esmainwindow.h"

class ESMainWindow : public QMainWindow
{
	Q_OBJECT

public:
	ESMainWindow(QWidget *parent = 0);
	~ESMainWindow();

public slots:
	void slotManageStockItems();
	void slotManageItems();
	void slotManageItemCategories();
	void slotManageItemPrices();
	void slotAddBill();
	void slotCurrentBills();
	void slotLogin();

private:
	Ui::ESMainWindowClass ui;
};

#endif // ESMAINWINDOW_H
