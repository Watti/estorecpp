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

	QString searchQuery = "SELECT * FROM stock_order";
	//bool categorySelected = false;
	if (searchName != nullptr)
	{
		// 		searchQuery.append(" WHERE deleted =0 AND itemcategory_id = ");
		// 		QString catId;
		// 		catId.setNum(categoryId);
		// 		searchQuery.append(catId);
		//		categorySelected = true;
	}

	if (!searchName.isEmpty())
	{
		// 		if (categorySelected)
		// 		{
		// 			searchQuery.append(" AND ");
		// 		}
		// 		else
		// 		{
		// 			searchQuery.append(" WHERE deleted = 0 AND ");
		// 		}
		// 		searchQuery.append(" (item_code LIKE '%" + searchText + "%' OR item_name LIKE '%" + searchText + "%')");
	}
	else
	{
		//if (!categorySelected)
		{
			searchQuery.append(" WHERE deleted=0");
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
		QString itemId = queryOrder.value("item_id").toString();
		QString orderId = queryOrder.value("order_id").toString();

		ui.tableWidget->setItem(row, 2, new QTableWidgetItem(queryOrder.value("date_added").toString()));
		ui.tableWidget->setItem(row, 4, new QTableWidgetItem(queryOrder.value("quantity").toString()));
		ui.tableWidget->setItem(row, 3, new QTableWidgetItem(queryOrder.value("price").toString()));
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
		QPushButton* updateBtn = new QPushButton("Update", base);
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

void ESManageOrderItems::slotUpdate(QString itemId)
{

}

void ESManageOrderItems::slotRemove(QString itemId)
{

}