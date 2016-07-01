#ifndef ESMAINWINDOW_H
#define ESMAINWINDOW_H

#include <QtWidgets/QMainWindow>
#include "ui_esmainwindow.h"
#include "escalculator.h"

class ESMainWindow : public QMainWindow
{
	Q_OBJECT

public:
	ESMainWindow(QWidget *parent = 0);
	~ESMainWindow();

	void reloadMenus();

public slots:
	void slotManageStockItems();
	void slotManageItems();
	void slotManageItemCategories();
	void slotManageItemPrices();
	void slotReturnItems();
	void slotManageOrderItems();
	void slotManageSuppliers();
	void slotAddBill();
	void slotCurrentBills();
	void slotConfigure();
	void slotShowStatus();
	void slotGeneralReports();
	void slotSalesReports();
	void slotStocksReports();
	void slotSettings();
	void slotBackupRestore();
	void slotOffers();
	void slotGiftVouchers();
	void slotLogin();
	void slotProfile();
	void slotLogout();
	void slotShowCalculator();
	void slotAddCustomer();
	void slotViewCustomerHistory();
	void slotViewCustomerOutstanding();
	void slotManageUsers();

protected:
	void closeEvent(QCloseEvent *event);

private:
	Ui::ESMainWindowClass ui;
	QAction* m_actionLogin;
	QAction* m_actionLogout;
	QAction* m_actionProfile;
	QAction* m_actionManageUsers;
	ESCalculator* calc;
};

#endif // ESMAINWINDOW_H
