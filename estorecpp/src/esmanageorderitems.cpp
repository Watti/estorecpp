#include "esmanageorderitems.h"
#include "utility/esmainwindowholder.h"
#include "utility/esdbconnection.h"
#include <QMessageBox>
#include <QPushButton>
#include "utility/utility.h"
#include "esaddorderitem.h"
#include "esordercheckin.h"

ESManageOrderItems::ESManageOrderItems(QWidget *parent/* = 0*/)
{
	ui.setupUi(this);

	m_updateButtonSignalMapper = new QSignalMapper(this);
	m_removeButtonSignalMapper = new QSignalMapper(this);

	QObject::connect(m_updateButtonSignalMapper, SIGNAL(mapped(QString)), this, SLOT(slotUpdate(QString)));
	QObject::connect(m_removeButtonSignalMapper, SIGNAL(mapped(QString)), this, SLOT(slotRemove(QString)));
	QObject::connect(ui.userSearch, SIGNAL(textChanged(QString)), this, SLOT(slotSearch()));
	QObject::connect(ui.supplierSearch, SIGNAL(textChanged(QString)), this, SLOT(slotSearch()));
	QObject::connect(ui.addOrderItemBtn, SIGNAL(clicked()), this, SLOT(slotAddNewOrderItem()));
	QObject::connect(ui.showAllOrders, SIGNAL(stateChanged(int)), this, SLOT(slotSearch()));

	QStringList headerLabels;
	headerLabels.append("Order ID");	
	headerLabels.append("Supplier ID");
	headerLabels.append("Supplier Name");
	headerLabels.append("Placed Date");
	headerLabels.append("Comments");
	headerLabels.append("User");
	headerLabels.append("Actions");

	ui.tableWidget->setHorizontalHeaderLabels(headerLabels);
	ui.tableWidget->horizontalHeader()->setStretchLastSection(true);
	ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.prevBtn->setDisabled(true);
	ui.nextBtn->setDisabled(true);

	if (!ES::DbConnection::instance()->open())
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Critical);
		mbox.setText(QString("Cannot connect to the database : ESManageOrderItems::ESManageOrderItems "));
		mbox.exec();
	}
	else
	{
		slotSearch();
	}
}

ESManageOrderItems::~ESManageOrderItems()
{

}

void ESManageOrderItems::slotAddNewOrderItem()
{
	AddOrderItem* addOrderItem = new AddOrderItem(this);
	ES::MainWindowHolder::instance()->getMainWindow()->setCentralWidget(addOrderItem);
	addOrderItem->show();
}

void ESManageOrderItems::slotSearch()
{

	QString searchUser = ui.userSearch->text();
	QString searchSupplier = ui.supplierSearch->text();

	while (ui.tableWidget->rowCount() > 0)
	{
		ui.tableWidget->removeRow(0);
	}

	QString uId = nullptr;
	bool userGiven = false, supplierGiven = false;
	if (searchUser != nullptr && !searchSupplier.isEmpty())
	{
		userGiven = true;
	}
	if (searchSupplier != nullptr && !searchSupplier.isEmpty())
	{
		supplierGiven = true;
	}

	QString searchQuery = "SELECT * FROM purchase_order WHERE deleted = 0 ";
	if (!ui.showAllOrders->isChecked())
	{
		searchQuery.append("AND checked_in = 0");
	}

	if (!supplierGiven && userGiven)
	{
		searchQuery = "SELECT * FROM purchase_order so, user u WHERE so.user_id = u.user_id AND u.username LIKE '%" + searchUser + "%' AND so.deleted = 0 ";
		if (!ui.showAllOrders->isChecked())
		{
			searchQuery.append("AND checked_in = 0");
		}
	}
	else if (supplierGiven && !userGiven)
	{
		searchQuery = "SELECT * FROM purchase_order so, item i WHERE so.item_id = i.item_id AND i.item_code LIKE '%" + searchSupplier + "%' AND so.deleted = 0 ";
		if (!ui.showAllOrders->isChecked())
		{
			searchQuery.append("AND checked_in = 0");
		}
	}
	else if (supplierGiven && userGiven)
	{
		searchQuery = "SELECT * FROM purchase_order so, user u , item i WHERE (so.user_id = u.user_id AND so.item_id = i.item_id) AND u.username LIKE '%" + searchUser + "%' AND i.item_code LIKE '%" + searchSupplier + "%' AND so.deleted = 0 ";
		if (!ui.showAllOrders->isChecked())
		{
			searchQuery.append("AND checked_in = 0");
		}
	}
	ui.tableWidget->setSortingEnabled(false);
	QSqlQuery queryItems(searchQuery);
	displayItems(queryItems);
	ui.tableWidget->setSortingEnabled(true);
}

void ESManageOrderItems::displayItems(QSqlQuery& queryOrder)
{
	int row = 0;
	while (queryOrder.next())
	{
		row = ui.tableWidget->rowCount();
		ui.tableWidget->insertRow(row);
		QString poId = queryOrder.value("purchaseorder_id").toString();
		bool checkedIn = (1 == queryOrder.value("checked_in").toInt());
		QColor red(245, 169, 169);
		QColor green(169, 245, 208);
		QTableWidgetItem* tbItem = NULL;
		tbItem = new QTableWidgetItem(poId);
		tbItem->setBackgroundColor(checkedIn ? red : green);
		
		ui.tableWidget->setItem(row, 0, tbItem);

		QSqlQuery q("SELECT * FROM supplier WHERE deleted = 0 AND supplier_id = " + queryOrder.value("supplier_id").toString());
		if (q.next())
		{
			tbItem = new QTableWidgetItem(q.value("supplier_code").toString());
			tbItem->setBackgroundColor(checkedIn ? red : green);
			ui.tableWidget->setItem(row, 1, tbItem);
			
			tbItem = new QTableWidgetItem(q.value("supplier_name").toString());
			tbItem->setBackgroundColor(checkedIn ? red : green);
			ui.tableWidget->setItem(row, 2, tbItem);
		}

		QDate orderDate = QDate::fromString(queryOrder.value("order_date").toString(), "yyyy-MM-dd");
		tbItem = new QTableWidgetItem(queryOrder.value("order_date").toString().append(" (").append(orderDate.toString("MMMM dd")).append(")"));
		tbItem->setBackgroundColor(checkedIn ? red : green);
		ui.tableWidget->setItem(row, 3, tbItem);
		
		tbItem = new QTableWidgetItem(queryOrder.value("comments").toString());
		tbItem->setBackgroundColor(checkedIn ? red : green);
		ui.tableWidget->setItem(row, 4, tbItem);
		QSqlQuery queryUser("SELECT * FROM user WHERE user_id = " + queryOrder.value("user_id").toString());
		if (queryUser.next())
		{
			tbItem = new QTableWidgetItem(queryUser.value("display_name").toString());
			tbItem->setBackgroundColor(checkedIn ? red : green);
			ui.tableWidget->setItem(row, 5, tbItem);
		}

		QWidget* base = new QWidget(ui.tableWidget);
		QPalette p(base->palette());
		p.setColor(QPalette::Background, checkedIn ? red : green);
		base->setAutoFillBackground(true);
		base->setPalette(p);
		QPushButton* checkInBtn = NULL;

		if (!checkedIn)
		{
			checkInBtn = new QPushButton("Check In", base);
			checkInBtn->setMaximumWidth(100);
		}
		QPushButton* removeBtn = new QPushButton("Remove", base);
		removeBtn->setMaximumWidth(100);

		if (!checkedIn)
		{
			m_updateButtonSignalMapper->setMapping(checkInBtn, poId);
			QObject::connect(checkInBtn, SIGNAL(clicked()), m_updateButtonSignalMapper, SLOT(map()));
		}

		m_removeButtonSignalMapper->setMapping(removeBtn, poId);
		QObject::connect(removeBtn, SIGNAL(clicked()), m_removeButtonSignalMapper, SLOT(map()));

		QHBoxLayout *layout = new QHBoxLayout;
		layout->setContentsMargins(0, 0, 0, 0);
		if (!checkedIn)
		{
			layout->addWidget(checkInBtn);
		}
		layout->addWidget(removeBtn);
		layout->insertStretch(2);
		base->setLayout(layout);
		ui.tableWidget->setCellWidget(row, 6, base);
		base->show();
	}
}

void ESManageOrderItems::slotUpdate(QString orderId)
{
	ESOrderCheckIn* orderCheckin = new ESOrderCheckIn(orderId, this);
	ES::MainWindowHolder::instance()->getMainWindow()->setCentralWidget(orderCheckin);
	orderCheckin->show();
}

void ESManageOrderItems::slotRemove(QString order_id)
{
	if (ES::Utility::verifyUsingMessageBox(this, "EStore", "Do you really want to remove this?"))
	{
		QString str("UPDATE stock_order SET deleted = 1 WHERE order_id = " + order_id);
		QSqlQuery q;
		if (q.exec(str))
		{
			while (ui.tableWidget->rowCount() > 0)
			{
				ui.tableWidget->removeRow(0);
			}
			QSqlQuery displayQuery("SELECT * from stock_order WHERE deleted = 0");
			displayItems(displayQuery);
		}
	}
}