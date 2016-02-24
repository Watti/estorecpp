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

	void reloadMenus();

public slots:
	void slotManageStockItems();
	void slotManageItems();
	void slotManageItemCategories();
	void slotManageItemPrices();
	void slotManageOrderItems();
	void slotAddBill();
	void slotCurrentBills();
	void slotConfigure();
	void slotShowStatus();
	void slotLogin();
	void slotProfile();
	void slotLogout();
	void slotManageUsers();

protected:
	void closeEvent(QCloseEvent *event);

private:
	Ui::ESMainWindowClass ui;
	QAction* m_actionLogin;
	QAction* m_actionLogout;
	QAction* m_actionProfile;
	QAction* m_actionManageUsers;
};

#endif // ESMAINWINDOW_H
