#include "esmanageorderitems.h"
#include "utility/esmainwindowholder.h"
#include "utility/esdbconnection.h"
#include <QMessageBox>
#include <QPushButton>
#include "utility/utility.h"
#include "esaddorderitem.h"

ESManageOrderItems::ESManageOrderItems(QWidget *parent/* = 0*/)
{
	ui.setupUi(this);

	m_updateButtonSignalMapper = new QSignalMapper(this);
	m_removeButtonSignalMapper = new QSignalMapper(this);

	QObject::connect(m_updateButtonSignalMapper, SIGNAL(mapped(QString)), this, SLOT(slotUpdate(QString)));
	QObject::connect(m_removeButtonSignalMapper, SIGNAL(mapped(QString)), this, SLOT(slotRemove(QString)));
	QObject::connect(ui.usernameSearch, SIGNAL(textChanged(QString)), this, SLOT(slotSearch()));
	QObject::connect(ui.itemCodeSearch, SIGNAL(textChanged(QString)), this, SLOT(slotSearch()));
	QObject::connect(ui.addOrderItemBtn, SIGNAL(clicked()), this, SLOT(slotAddNewOrderItem()));

	QStringList headerLabels;
	headerLabels.append("Item Code");
	headerLabels.append("User");
	headerLabels.append("Date");
	headerLabels.append("Price");
	headerLabels.append("Qty");
	headerLabels.append("Comment");
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

	QString searchName = ui.usernameSearch->text();
	QString searchItemCode = ui.itemCodeSearch->text();

	while (ui.tableWidget->rowCount() > 0)
	{
		ui.tableWidget->removeRow(0);
	}

	QString searchQuery = "SELECT * FROM stock_order WHERE deleted = 0";
	QString uId = nullptr;
	bool uNameGiven = false, itemCodeGiven = false;
	if (searchName != nullptr && !searchName.isEmpty())
	{
		uNameGiven = true;
	}
	if (searchItemCode != nullptr && !searchItemCode.isEmpty())
	{
		itemCodeGiven = true;
	}

	if (!itemCodeGiven && uNameGiven)
	{
		searchQuery = "SELECT * from stock_order so, user u WHERE so.user_id = u.user_id AND u.username LIKE '%" + searchName + "%' AND so.deleted = 0";
	}
	else if (itemCodeGiven && !uNameGiven)
	{
		searchQuery = "SELECT * from stock_order so, item i WHERE so.item_id = i.item_id AND i.item_code LIKE '%" + searchItemCode + "%' AND so.deleted = 0";
	}
	else if (itemCodeGiven && uNameGiven)
	{
		searchQuery = "SELECT * from stock_order so, user u , item i WHERE (so.user_id = u.user_id AND so.item_id = i.item_id) AND u.username LIKE '%" + searchName + "%' AND i.item_code LIKE '%" + searchItemCode + "%' AND so.deleted = 0";
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
		QString itemId = queryOrder.value("item_id").toString();
		QString orderId = queryOrder.value("order_id").toString();

		ui.tableWidget->setItem(row, 2, new QTableWidgetItem(queryOrder.value("date_added").toString()));
		ui.tableWidget->setItem(row, 4, new QTableWidgetItem(queryOrder.value("quantity").toString()));
		ui.tableWidget->setItem(row, 3, new QTableWidgetItem(queryOrder.value("unit_price").toString()));
		ui.tableWidget->setItem(row, 5, new QTableWidgetItem(queryOrder.value("description").toString()));

		QSqlQuery queryItems("SELECT * FROM item WHERE item_id = " + itemId);
		if (queryItems.next())
		{
			ui.tableWidget->setItem(row, 0, new QTableWidgetItem(queryItems.value("item_code").toString()));
		}

		QSqlQuery queryUser("SELECT * FROM user WHERE user_id = " + queryOrder.value("user_id").toString());
		if (queryUser.next())
		{
			ui.tableWidget->setItem(row, 1, new QTableWidgetItem(queryUser.value("display_name").toString()));
		}

		QWidget* base = new QWidget(ui.tableWidget);
		QPushButton* updateBtn = new QPushButton("Process", base);
		updateBtn->setMaximumWidth(100);
		QPushButton* removeBtn = new QPushButton("Remove", base);
		removeBtn->setMaximumWidth(100);

		m_updateButtonSignalMapper->setMapping(updateBtn, orderId);
		QObject::connect(updateBtn, SIGNAL(clicked()), m_updateButtonSignalMapper, SLOT(map()));

		QObject::connect(removeBtn, SIGNAL(clicked()), m_removeButtonSignalMapper, SLOT(map()));
		m_removeButtonSignalMapper->setMapping(removeBtn, orderId);

		QHBoxLayout *layout = new QHBoxLayout;
		layout->setContentsMargins(0, 0, 0, 0);
		layout->addWidget(updateBtn);
		layout->addWidget(removeBtn);
		layout->insertStretch(2);
		base->setLayout(layout);
		ui.tableWidget->setCellWidget(row, 6, base);
		base->show();
	}
}

void ESManageOrderItems::slotUpdate(QString orderId)
{

	/*AddOrderItem* addOrder = new AddOrderItem(this);
	addOrder->getUI().groupBox->setTitle("Update Order Item");
	addOrder->getUI().addOrderItemButton->setText("Update");
	QSqlQuery queryOrder("SELECT * FROM stock_order WHERE order_id = " + orderId);
	if (queryOrder.next())
	{
		QString price = queryOrder.value("unit_price").toString();
		QString itemId = queryOrder.value("item_id").toString();
		addOrder->getUI().unitPrice->setText(price);
		addOrder->getUI().itemDescription->setText(queryOrder.value("description").toString());
		addOrder->getUI().qty->setText(queryOrder.value("quantity").toString());

		QSqlQuery queryItem("SELECT * FROM item WHERE item_id = " + itemId);
		if (queryItem.next())
		{
			addOrder->getUI().itemCode->setText(queryItem.value("item_code").toString());
		}
	}
	addOrder->setUpdate(true);
	addOrder->setOrderId(orderId);
	ES::MainWindowHolder::instance()->getMainWindow()->setCentralWidget(addOrder);
	addOrder->show();*/
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