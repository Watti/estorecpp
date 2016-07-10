#include "esmainwindow.h"
#include "esloginwidget.h"
#include "esmanagestockitems.h"
#include "esitemcategories.h"
#include "esmanageitems.h"
#include "esitemprices.h"
#include "esmanageorderitems.h"
#include "esmanagesuppliers.h"
#include "esaddbill.h"
#include "escurrentbills.h"
#include "escashbalanceconfigure.h"
#include "escashbalancestatus.h"
#include "esmanageusers.h"
#include "esgeneratestatisticswidget.h"
#include "esgenericsalesstatistics.h"
#include "essystemsettings.h"
#include "escalculator.h"
#include "esstockreport.h"
#include "esbackuprestore.h"
#include "utility/esmenumanager.h"
#include "utility/session.h"
#include "utility/utility.h"
#include <QMessageBox>
#include <QShortcut>
#include "esreturnitems.h"
#include "esreminders.h"
#include "esaddcustomer.h"
#include "escustomerinfo.h"
#include "utility/esmainwindowholder.h"
#include "essecondarydisplay.h"
#include "essalesummaryreport.h"
#include "escutomeroutstanding.h"
#include "espettycashsummary.h"
#include "essalesreportcontainer.h"

ESMainWindow::ESMainWindow(QWidget *parent)
: QMainWindow(parent)
{
	ui.setupUi(this);

	calc = new ESCalculator(this);
	calc->setWindowModality(Qt::ApplicationModal);

	QMenuBar* rightMenubar = new QMenuBar(ui.menuBar);
	QMenu* menuLogin = new QMenu("User", rightMenubar);

	m_actionLogin = new QAction("Login", menuLogin);
	menuLogin->addAction(m_actionLogin);
	m_actionProfile = new QAction("Profile", menuLogin);
	menuLogin->addAction(m_actionProfile);
	m_actionLogout = new QAction("Logout", menuLogin);
	menuLogin->addAction(m_actionLogout);
	m_actionManageUsers = new QAction("Manage Users", menuLogin);
	menuLogin->addAction(m_actionManageUsers);

	rightMenubar->addMenu(menuLogin);
	ui.menuBar->setCornerWidget(rightMenubar);

	// connections
	QObject::connect(ui.actionManageStockItems, SIGNAL(triggered()), this, SLOT(slotManageStockItems()));
	QObject::connect(ui.actionManageItems, SIGNAL(triggered()), this, SLOT(slotManageItems()));
	QObject::connect(ui.actionManageItemCategories, SIGNAL(triggered()), this, SLOT(slotManageItemCategories()));
	QObject::connect(ui.actionManageItemPrices, SIGNAL(triggered()), this, SLOT(slotManageItemPrices()));
	QObject::connect(ui.actionReturnItems, SIGNAL(triggered()), this, SLOT(slotReturnItems()));
	QObject::connect(ui.actionManageOrderItems, SIGNAL(triggered()), this, SLOT(slotManageOrderItems()));
	QObject::connect(ui.actionManageSuppliers, SIGNAL(triggered()), this, SLOT(slotManageSuppliers()));
	QObject::connect(ui.actionAddBill, SIGNAL(triggered()), this, SLOT(slotAddBill()));
	QObject::connect(ui.actionCurrentBills, SIGNAL(triggered()), this, SLOT(slotCurrentBills()));
	QObject::connect(ui.actionConfigure, SIGNAL(triggered()), this, SLOT(slotConfigure()));
	QObject::connect(ui.actionShowStatus, SIGNAL(triggered()), this, SLOT(slotShowStatus()));
	QObject::connect(ui.actionGeneral, SIGNAL(triggered()), this, SLOT(slotGeneralReports()));
	QObject::connect(ui.actionSales, SIGNAL(triggered()), this, SLOT(slotSalesReports()));
	QObject::connect(ui.actionStocks, SIGNAL(triggered()), this, SLOT(slotStocksReports()));
	QObject::connect(ui.actionSettings, SIGNAL(triggered()), this, SLOT(slotSettings()));
	QObject::connect(ui.actionBackupRestore, SIGNAL(triggered()), this, SLOT(slotBackupRestore()));
	QObject::connect(ui.actionOffers, SIGNAL(triggered()), this, SLOT(slotOffers()));
	QObject::connect(ui.actionGiftVouchers, SIGNAL(triggered()), this, SLOT(slotGiftVouchers()));
	QObject::connect(m_actionLogin, SIGNAL(triggered()), this, SLOT(slotLogin()));
	QObject::connect(m_actionProfile, SIGNAL(triggered()), this, SLOT(slotProfile()));
	QObject::connect(m_actionLogout, SIGNAL(triggered()), this, SLOT(slotLogout())); 
	QObject::connect(m_actionManageUsers, SIGNAL(triggered()), this, SLOT(slotManageUsers()));
	QObject::connect(ui.actionManageCustomers, SIGNAL(triggered()), this, SLOT(slotAddCustomer()));
	QObject::connect(ui.actionViewCustomerHistory, SIGNAL(triggered()), this, SLOT(slotViewCustomerHistory()));
	QObject::connect(ui.actionCustomerOutstanding, SIGNAL(triggered()), this, SLOT(slotViewCustomerOutstanding()));
	QObject::connect(ui.actionPettyCashSummary, SIGNAL(triggered()), this, SLOT(slotPettyCashSummary()));

	new QShortcut(QKeySequence(Qt::Key_F9), this, SLOT(slotShowCalculator()));

	// initialize menu manager
	ES::MenuManager* mmgr = ES::MenuManager::instance();
	mmgr->addMenu("User", menuLogin);
	mmgr->addMenu("Stock", ui.menuStock);
	mmgr->addMenu("Items", ui.menuItems);
	mmgr->addMenu("Orders", ui.menuOrders);
	mmgr->addMenu("Customers", ui.menuCustomers);
	mmgr->addMenu("Billing", ui.menuBilling);
	mmgr->addMenu("Reports", ui.menuReports);
	mmgr->addMenu("Cash Balance", ui.menuCashBalance);
	mmgr->addMenu("System", ui.menuSystem);
	mmgr->addMenu("Business", ui.menuBussiness);
	mmgr->addMenu("Help", ui.menuHelp);

	mmgr->addAction("Login", m_actionLogin);
	mmgr->addAction("Profile", m_actionProfile);
	mmgr->addAction("Logout", m_actionLogout);
	mmgr->addAction("Manage Users", m_actionManageUsers);

	mmgr->addAction("Manage Stock Items", ui.actionManageStockItems);
	ui.actionManageStockItems->setIcon(QIcon("icons/manage_store.png"));

	ui.mainToolBar->setIconSize(QSize(36, 36));
	//ui.mainToolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

	QLabel* logoLabel = new QLabel(ui.mainToolBar);
	logoLabel->setMinimumWidth(90);
	logoLabel->setPixmap(QPixmap("icons/progex_s.png"));

	ui.mainToolBar->addWidget(logoLabel);
	ui.mainToolBar->addAction(ui.actionManageStockItems);
	mmgr->addSeparator(ui.mainToolBar->addSeparator());

	ui.mainToolBar->setMinimumSize(48, 48);

	mmgr->addAction("Manage Items", ui.actionManageItems);
	ui.actionManageItems->setIcon(QIcon("icons/manage_items.png"));
	ui.mainToolBar->addAction(ui.actionManageItems);
	mmgr->addAction("Manage Item Categories", ui.actionManageItemCategories);
	ui.actionManageItemCategories->setIcon(QIcon("icons/manage_item_categories.png"));
	ui.mainToolBar->addAction(ui.actionManageItemCategories);
	mmgr->addAction("Manage Item Prices", ui.actionManageItemPrices);
	ui.actionManageItemPrices->setIcon(QIcon("icons/manage_item_prices.png"));
	ui.mainToolBar->addAction(ui.actionManageItemPrices);
	mmgr->addAction("Return Items", ui.actionReturnItems);
	ui.actionReturnItems->setIcon(QIcon("icons/return_item.png"));
	ui.mainToolBar->addAction(ui.actionReturnItems);
	mmgr->addSeparator(ui.mainToolBar->addSeparator());

	mmgr->addAction("Manage Order Items", ui.actionManageOrderItems);
	ui.actionManageOrderItems->setIcon(QIcon("icons/truck.png"));
	ui.mainToolBar->addAction(ui.actionManageOrderItems);
	mmgr->addAction("Manage Suppliers", ui.actionManageSuppliers);
	ui.actionManageSuppliers->setIcon(QIcon("icons/supplier.png"));
	ui.mainToolBar->addAction(ui.actionManageSuppliers);
	mmgr->addSeparator(ui.mainToolBar->addSeparator());

	mmgr->addAction("Manage Customers", ui.actionManageCustomers);
	ui.actionManageCustomers->setIcon(QIcon("icons/customer.png"));
	ui.mainToolBar->addAction(ui.actionManageCustomers);
	mmgr->addAction("View History", ui.actionViewCustomerHistory);
	ui.actionViewCustomerHistory->setIcon(QIcon("icons/customer_history.png"));
	ui.mainToolBar->addAction(ui.actionViewCustomerHistory);
	mmgr->addAction("Customer Outstanding", ui.actionCustomerOutstanding);
	ui.actionCustomerOutstanding->setIcon(QIcon("icons/customer_outstanding.png"));
	ui.mainToolBar->addAction(ui.actionCustomerOutstanding);
	mmgr->addSeparator(ui.mainToolBar->addSeparator());

	mmgr->addAction("Add Bill", ui.actionAddBill);
	ui.actionAddBill->setIcon(QIcon("icons/add_bill.png"));
	ui.mainToolBar->addAction(ui.actionAddBill);
	mmgr->addAction("Current Bills", ui.actionCurrentBills);
	ui.actionCurrentBills->setIcon(QIcon("icons/all_bills.png"));
	ui.mainToolBar->addAction(ui.actionCurrentBills);
	mmgr->addSeparator(ui.mainToolBar->addSeparator());

	mmgr->addAction("Configure", ui.actionConfigure);
	ui.actionConfigure->setIcon(QIcon("icons/money.png"));
	ui.mainToolBar->addAction(ui.actionConfigure);
	mmgr->addAction("Show Status", ui.actionShowStatus);
	ui.actionShowStatus->setIcon(QIcon("icons/money_status.png"));
	ui.mainToolBar->addAction(ui.actionShowStatus);
	mmgr->addSeparator(ui.mainToolBar->addSeparator());
	
	mmgr->addAction("General", ui.actionGeneral);
	ui.actionGeneral->setIcon(QIcon("icons/reports.png"));
	ui.mainToolBar->addAction(ui.actionGeneral);
	mmgr->addAction("Sales", ui.actionSales);
	ui.actionSales->setIcon(QIcon("icons/sales_report.png"));
	ui.mainToolBar->addAction(ui.actionSales);
	mmgr->addAction("Stocks", ui.actionStocks);
	ui.actionStocks->setIcon(QIcon("icons/stock_report.png"));
	ui.mainToolBar->addAction(ui.actionStocks);
	mmgr->addSeparator(ui.mainToolBar->addSeparator());
	
	mmgr->addAction("Settings", ui.actionSettings);
	ui.actionSettings->setIcon(QIcon("icons/system.png"));
	ui.mainToolBar->addAction(ui.actionSettings);	
	mmgr->addAction("Backup & Restore", ui.actionBackupRestore);
	ui.actionBackupRestore->setIcon(QIcon("icons/hdd.png"));
	ui.mainToolBar->addAction(ui.actionBackupRestore);
	mmgr->addSeparator(ui.mainToolBar->addSeparator());

	mmgr->addAction("Offers", ui.actionOffers);
	ui.actionOffers->setIcon(QIcon("icons/offers.png"));
	ui.mainToolBar->addAction(ui.actionOffers);
	mmgr->addAction("Gift Vouchers", ui.actionGiftVouchers);
	ui.actionGiftVouchers->setIcon(QIcon("icons/gift_voucher.png"));
	ui.mainToolBar->addAction(ui.actionGiftVouchers);

	mmgr->addMenuActionMapping("Stock", "Manage Stock Items");
	mmgr->addMenuActionMapping("Items", "Manage Items");
	mmgr->addMenuActionMapping("Items", "Manage Item Categories");
	//mmgr->addMenuActionMapping("Items", "Manage Item Prices");
	mmgr->addMenuActionMapping("Items", "Return Items");
	mmgr->addMenuActionMapping("Orders", "Manage Order Items");
	mmgr->addMenuActionMapping("Orders", "Manage Suppliers");
	mmgr->addMenuActionMapping("Customers", "Manage Customers");
	mmgr->addMenuActionMapping("Customers", "View History");
	mmgr->addMenuActionMapping("Customers", "Customer Outstanding");
	mmgr->addMenuActionMapping("Billing", "Add Bill");
	mmgr->addMenuActionMapping("Billing", "Current Bills");
	mmgr->addMenuActionMapping("Cash Balance", "Configure");
	mmgr->addMenuActionMapping("Cash Balance", "Show Status");
	mmgr->addMenuActionMapping("System", "Settings");
	mmgr->addMenuActionMapping("System", "Backup & Restore");
	mmgr->addMenuActionMapping("Business", "Offers");
	mmgr->addMenuActionMapping("Business", "Gift Vouchers");
	mmgr->addMenuActionMapping("Reports", "General");
	mmgr->addMenuActionMapping("Reports", "Sales");
	mmgr->addMenuActionMapping("Reports", "Stocks");

	//mmgr->disableAll();
	QString perms = "";
	ES::MenuManager::instance()->reload(perms);

	slotLogin();

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

void ESMainWindow::slotReturnItems()
{
	ESReturnItems* retrunItems = new ESReturnItems(this);
	this->setCentralWidget(retrunItems);
	retrunItems->show();
}

void ESMainWindow::slotManageOrderItems()
{
	ESManageOrderItems* orderItems = new ESManageOrderItems(this);
	this->setCentralWidget(orderItems);
	orderItems->show();
}

void ESMainWindow::slotManageSuppliers()
{
	ESManageSuppliers* manageSuppliers = new ESManageSuppliers(this);
	this->setCentralWidget(manageSuppliers);
	manageSuppliers->show();
}

void ESMainWindow::slotAddBill()
{
	ESAddBill* addBill = new ESAddBill(this);
	this->setCentralWidget(addBill);
	addBill->show();
}

void ESMainWindow::slotCurrentBills()
{
	ESCurrentBills* currentBills = new ESCurrentBills(this);
	this->setCentralWidget(currentBills);
	currentBills->show();
}

void ESMainWindow::slotConfigure()
{
	ESCashBalanceConfigure* cashConfigure = new ESCashBalanceConfigure(this);
	this->setCentralWidget(cashConfigure);
	cashConfigure->show();
}

void ESMainWindow::slotShowStatus()
{
	ESCashBalanceStatus* cashStatus = new ESCashBalanceStatus(this);
	this->setCentralWidget(cashStatus);
	cashStatus->show();
}

void ESMainWindow::slotGeneralReports()
{
	ESGenerateStatistics* stats = new ESGenerateStatistics(this);
	this->setCentralWidget(stats);
	stats->show();
}

void ESMainWindow::slotSalesReports()
{
// 	ESGenericSalesStatistics* salesReport = new ESGenericSalesStatistics(this);
// 	this->setCentralWidget(salesReport);
// 	salesReport->show();

// 	ESSalesSummary* salesReport = new ESSalesSummary(this);
//  	this->setCentralWidget(salesReport);
//  	salesReport->show();

	ESSalesReportContainer* reports = new ESSalesReportContainer(this);
	this->setCentralWidget(reports);
	reports->show();
}

void ESMainWindow::slotStocksReports()
{
	ESStockReport* stockReport = new ESStockReport(this);
	this->setCentralWidget(stockReport);
	stockReport->show();
}

void ESMainWindow::slotSettings()
{
	ESSystemSettings* settings = new ESSystemSettings(this);
	this->setCentralWidget(settings);
	settings->show();
}

void ESMainWindow::slotBackupRestore()
{
	ESBackupRestore* backupRestore = new ESBackupRestore(this);
	this->setCentralWidget(backupRestore);
	backupRestore->show();
}

void ESMainWindow::slotOffers()
{
	ESReminders* reminders = new ESReminders(this);
	this->setCentralWidget(reminders);
	reminders->show();
}

void ESMainWindow::slotGiftVouchers()
{

}

void ESMainWindow::slotLogin()
{
	ESLoginWidget* loginWidget = new ESLoginWidget(this);
	this->setCentralWidget(loginWidget);
	loginWidget->show();
}

void ESMainWindow::slotProfile()
{

}

void ESMainWindow::slotLogout()
{
	ES::Session::getInstance()->invalidate();

	ESLoginWidget* loginWidget = new ESLoginWidget(this);
	this->setCentralWidget(loginWidget);
	loginWidget->show();
}

void ESMainWindow::slotManageUsers()
{
	ESManageUsers* manageUsers = new ESManageUsers(this);
	this->setCentralWidget(manageUsers);
	manageUsers->show();
}

void ESMainWindow::reloadMenus()
{

}

void ESMainWindow::closeEvent(QCloseEvent *event)
{
	//check for pending bills
	ES::Session::getInstance()->invalidate();
	ES::MainWindowHolder::instance()->getSecondaryDisplay()->close();

	event->accept();
}


void ESMainWindow::slotShowCalculator()
{	
	calc->show();
}

void ESMainWindow::slotAddCustomer()
{
	ESAddCustomer* addCustomer = new ESAddCustomer(this);
	this->setCentralWidget(addCustomer);
	addCustomer->show();
}

void ESMainWindow::slotViewCustomerHistory()
{
	ESCustomerInfo* customerInfo = new ESCustomerInfo(this);
	this->setCentralWidget(customerInfo);
	customerInfo->show();
}

void ESMainWindow::slotViewCustomerOutstanding()
{
	ESCustomerOutstanding* customerOutstanding = new ESCustomerOutstanding(this);
	this->setCentralWidget(customerOutstanding);
	customerOutstanding->show();
}

void ESMainWindow::slotPettyCashSummary()
{
// 	PettyCashSummary* pettyCashSummary = new PettyCashSummary(this);
// 	this->setCentralWidget(pettyCashSummary);
// 	pettyCashSummary->show();
}



