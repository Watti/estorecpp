#include "esmainwindow.h"
#include "esloginwidget.h"
#include "esmanagestockitems.h"

ESMainWindow::ESMainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	QMenuBar* rightMenubar = new QMenuBar(ui.menuBar);
	QMenu* menuLogin = new QMenu("User", rightMenubar);

	QAction* actionLogin = new QAction("Login", menuLogin);
	menuLogin->addAction(actionLogin);

	rightMenubar->addMenu(menuLogin);
	ui.menuBar->setCornerWidget(rightMenubar);

	// connections
	QObject::connect(ui.actionManageStockItems, SIGNAL(triggered()), this, SLOT(slotManageStockItems()));
	QObject::connect(ui.actionManageItems, SIGNAL(triggered()), this, SLOT(slotManageItems()));
	QObject::connect(ui.actionManageItemCategories, SIGNAL(triggered()), this, SLOT(slotManageItemCategories()));
	QObject::connect(ui.actionManageItemPrices, SIGNAL(triggered()), this, SLOT(slotManageItemPrices()));
	QObject::connect(ui.actionAddBill, SIGNAL(triggered()), this, SLOT(slotAddBill()));
	QObject::connect(ui.actionCurrentBills, SIGNAL(triggered()), this, SLOT(slotCurrentBills()));
	QObject::connect(actionLogin, SIGNAL(triggered()), this, SLOT(slotLogin()));
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

