#include "esmainwindow.h"
#include "esloginwidget.h"
#include "esmanagestockitems.h"
#include "utility/esmenumanager.h"

ESMainWindow::ESMainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	QMenuBar* rightMenubar = new QMenuBar(ui.menuBar);
	QMenu* menuLogin = new QMenu("User", rightMenubar);

	m_actionLogin = new QAction("Login", menuLogin);
	menuLogin->addAction(m_actionLogin);
	m_actionProfile = new QAction("Profile", menuLogin);
	//menuLogin->addAction(m_actionProfile);
	m_actionLogout = new QAction("Logout", menuLogin);
	//menuLogin->addAction(m_actionLogout);

	rightMenubar->addMenu(menuLogin);
	ui.menuBar->setCornerWidget(rightMenubar);

	// connections
	QObject::connect(ui.actionManageStockItems, SIGNAL(triggered()), this, SLOT(slotManageStockItems()));
	QObject::connect(ui.actionManageItems, SIGNAL(triggered()), this, SLOT(slotManageItems()));
	QObject::connect(ui.actionManageItemCategories, SIGNAL(triggered()), this, SLOT(slotManageItemCategories()));
	QObject::connect(ui.actionManageItemPrices, SIGNAL(triggered()), this, SLOT(slotManageItemPrices()));
	QObject::connect(ui.actionAddBill, SIGNAL(triggered()), this, SLOT(slotAddBill()));
	QObject::connect(ui.actionCurrentBills, SIGNAL(triggered()), this, SLOT(slotCurrentBills()));
	QObject::connect(m_actionLogin, SIGNAL(triggered()), this, SLOT(slotLogin()));

	// initialize menu manager
	ES::MenuManager* mmgr = ES::MenuManager::instance();
	mmgr->addMenu("Stock", ui.menuStock);
	mmgr->addMenu("Items", ui.menuItems);
	mmgr->addMenu("Billing", ui.menuBilling);
	mmgr->addMenu("Orders", ui.menuOrders);
	mmgr->addMenu("Reports", ui.menuReports);
	mmgr->addMenu("Help", ui.menuHelp);

	mmgr->addAction("Manage Stock Items", ui.actionManageStockItems);
	mmgr->addAction("Manage Items", ui.actionManageItems);
	mmgr->addAction("Manage Item Categories", ui.actionManageItemCategories);
	mmgr->addAction("Manage Item Prices", ui.actionManageItemPrices);
	mmgr->addAction("Add Bill", ui.actionAddBill);
	mmgr->addAction("Current Bills", ui.actionCurrentBills);

	mmgr->addMenuActionMapping("Stock", "Manage Stock Items");
	mmgr->addMenuActionMapping("Items", "Manage Items");
	mmgr->addMenuActionMapping("Items", "Manage Item Categories");
	mmgr->addMenuActionMapping("Items", "Manage Item Prices");
	mmgr->addMenuActionMapping("Billing", "Add Bill");
	mmgr->addMenuActionMapping("Billing", "Current Bills");

	mmgr->disableAll();

}

ESMainWindow::~ESMainWindow()
{

}

void ESMainWindow::slotManageStockItems()
{
	ESManageStockItems* manageStockItems = new ESManageStockItems(this);
	this->setCentralWidget(manageStockItems);
	manageStockItems->show();
}

void ESMainWindow::slotManageItems()
{
	close();
}

void ESMainWindow::slotManageItemCategories()
{
	close();
}

void ESMainWindow::slotManageItemPrices()
{
	close();
}

void ESMainWindow::slotAddBill()
{
	close();
}

void ESMainWindow::slotCurrentBills()
{
	close();
}

void ESMainWindow::slotLogin()
{
	ESLoginWidget* loginWidget = new ESLoginWidget(this);
	this->setCentralWidget(loginWidget);
	loginWidget->show();
}

void ESMainWindow::reloadMenus()
{

}

