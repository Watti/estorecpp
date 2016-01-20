#include "esmainwindow.h"
#include "esloginwidget.h"
#include "esmanagestockitems.h"
#include "esitemcategories.h"
#include "esmanageitems.h"
#include "esitemprices.h"
#include "esmanageorderitems.h"
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
	QObject::connect(ui.actionManageOrderItems, SIGNAL(triggered()), this, SLOT(slotManageOrderItems()));
	QObject::connect(ui.actionAddBill, SIGNAL(triggered()), this, SLOT(slotAddBill()));
	QObject::connect(ui.actionCurrentBills, SIGNAL(triggered()), this, SLOT(slotCurrentBills()));
	QObject::connect(m_actionLogin, SIGNAL(triggered()), this, SLOT(slotLogin()));

	// initialize menu manager
	ES::MenuManager* mmgr = ES::MenuManager::instance();
	mmgr->addMenu("Stock", ui.menuStock);
	mmgr->addMenu("Items", ui.menuItems);
	mmgr->addMenu("Orders", ui.menuOrders);
	mmgr->addMenu("Billing", ui.menuBilling);
	mmgr->addMenu("Reports", ui.menuReports);
	mmgr->addMenu("Help", ui.menuHelp);

	mmgr->addAction("Manage Stock Items", ui.actionManageStockItems);
	ui.actionManageStockItems->setIcon(QIcon("icons/manage_store.png"));
	ui.mainToolBar->addAction(ui.actionManageStockItems);
	ui.mainToolBar->addSeparator();

	ui.mainToolBar->setMinimumSize(40, 40);

	mmgr->addAction("Manage Items", ui.actionManageItems);
	ui.actionManageItems->setIcon(QIcon("icons/manage_items.png"));
	ui.mainToolBar->addAction(ui.actionManageItems);
	mmgr->addAction("Manage Item Categories", ui.actionManageItemCategories);
	ui.actionManageItemCategories->setIcon(QIcon("icons/manage_item_categories.png"));
	ui.mainToolBar->addAction(ui.actionManageItemCategories);
	mmgr->addAction("Manage Item Prices", ui.actionManageItemPrices);
	ui.actionManageItemPrices->setIcon(QIcon("icons/manage_item_prices.png"));
	ui.mainToolBar->addAction(ui.actionManageItemPrices);
	ui.mainToolBar->addSeparator();

	mmgr->addAction("Manage Order Items", ui.actionManageOrderItems);
	ui.actionManageOrderItems->setIcon(QIcon("icons/order_item.png"));
	ui.mainToolBar->addAction(ui.actionManageOrderItems);
	ui.mainToolBar->addSeparator();

	mmgr->addAction("Add Bill", ui.actionAddBill);
	ui.actionAddBill->setIcon(QIcon("icons/add_bill.png"));
	ui.mainToolBar->addAction(ui.actionAddBill);
	mmgr->addAction("Current Bills", ui.actionCurrentBills);
	ui.actionCurrentBills->setIcon(QIcon("icons/all_bills.png"));
	ui.mainToolBar->addAction(ui.actionCurrentBills);
	ui.mainToolBar->addSeparator();

	mmgr->addMenuActionMapping("Stock", "Manage Stock Items");
	mmgr->addMenuActionMapping("Items", "Manage Items");
	mmgr->addMenuActionMapping("Items", "Manage Item Categories");
	mmgr->addMenuActionMapping("Items", "Manage Item Prices");
	mmgr->addMenuActionMapping("Orders", "Manage Order Items");
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
	ESManageItems* manageItems = new ESManageItems(this);
	this->setCentralWidget(manageItems);
	manageItems->show();
}

void ESMainWindow::slotManageItemCategories()
{
	ESItemCategories* itemCategories = new ESItemCategories(this);
	this->setCentralWidget(itemCategories);
	itemCategories->show();
}

void ESMainWindow::slotManageItemPrices()
{
	ESItemPrices* itemPrices = new ESItemPrices(this);
	this->setCentralWidget(itemPrices);
	itemPrices->show();
}

void ESMainWindow::slotManageOrderItems()
{
	ESManageOrderItems* orderItems = new ESManageOrderItems(this);
	this->setCentralWidget(orderItems);
	orderItems->show();
}

void ESMainWindow::slotAddBill()
{
	
}

void ESMainWindow::slotCurrentBills()
{
	
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

