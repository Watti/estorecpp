#include "esmanageitems.h"
#include "esadditem.h"
#include "utility/esmainwindowholder.h"
#include "utility/esdbconnection.h"
#include <QMessageBox>
#include <QPushButton>
#include "utility/utility.h"


ESManageItems::ESManageItems(QWidget *parent /*= 0*/)
: QWidget(parent)
{
	ui.setupUi(this);
	m_updateButtonSignalMapper = new QSignalMapper(this);
	m_removeButtonSignalMapper = new QSignalMapper(this);

	QObject::connect(m_updateButtonSignalMapper, SIGNAL(mapped(QString)), this, SLOT(slotUpdate(QString)));
	QObject::connect(m_removeButtonSignalMapper, SIGNAL(mapped(QString)), this, SLOT(slotRemove(QString)));
	QObject::connect(ui.searchTextBox, SIGNAL(textChanged(QString)), this, SLOT(slotSearch()));
	QObject::connect(ui.categoryComboBox, SIGNAL(activated(QString)), this, SLOT(slotSearch()));

	QStringList headerLabels;
	headerLabels.append("Item Code");
	headerLabels.append("Item Name");
	headerLabels.append("Category");
	headerLabels.append("Qty");
	headerLabels.append("Min. Qty");
	headerLabels.append("Unit");
	headerLabels.append("Unit Price");
	headerLabels.append("Description");
	headerLabels.append("Actions");

	ui.tableWidget->setHorizontalHeaderLabels(headerLabels);
	ui.tableWidget->horizontalHeader()->setStretchLastSection(true);
	ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.prevBtn->setDisabled(true);
	ui.nextBtn->setDisabled(true);

	QObject::connect(ui.addNewItemButton, SIGNAL(clicked()), this, SLOT(slotAddNewItem()));
	if (!ES::DbConnection::instance()->open())
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Critical);
		mbox.setText(QString("Cannot connect to the database : ESManageStockItems::displayStockItems "));
		mbox.exec();
	}
	else
	{
		QSqlQuery queryCategory("SELECT * FROM item_category");
		QString catCode = "select";
		int catId = -1;

		ui.categoryComboBox->addItem(catCode, catId);

		while (queryCategory.next())
		{
			catId = queryCategory.value(0).toInt();
			ui.categoryComboBox->addItem(queryCategory.value(1).toString(), catId);
		}

		slotSearch();
	}

}

ESManageItems::~ESManageItems()
{

}

void ESManageItems::slotAddNewItem()
{
	AddItem* addItem = new AddItem(this);
	addItem->getUI().groupBox->setTitle("Add Item");
	ES::MainWindowHolder::instance()->getMainWindow()->setCentralWidget(addItem);
	addItem->show();
}

void ESManageItems::slotUpdate(QString itemId)
{
	AddItem* addItem = new AddItem(this);
	addItem->getUI().groupBox->setTitle("Update Item");
	addItem->getUI().addButton->setText("Update");
	QSqlQuery queryItem("SELECT * FROM item WHERE item_id = " + itemId);
	if (queryItem.next())
	{
		int categoryId = queryItem.value("itemcategory_id").toInt();
		int index = addItem->getUI().itemCategoryComboBox->findData(categoryId);
		addItem->getUI().itemCategoryComboBox->setCurrentIndex(index);
		addItem->getUI().itemCode->setText(queryItem.value("item_code").toString());
		addItem->getUI().barCode->setText(queryItem.value("bar_code").toString());
		addItem->getUI().unitText->setText(queryItem.value("unit").toString());
		addItem->getUI().descriptionText->setText(queryItem.value("description").toString());
		addItem->getUI().itemName->setText(queryItem.value("item_name").toString());
	}
	addItem->setUpdate(true);
	addItem->setItemId(itemId);
	ES::MainWindowHolder::instance()->getMainWindow()->setCentralWidget(addItem);
	addItem->show();
}

void ESManageItems::slotRemove(QString itemId)
{
	if (ES::Utility::verifyUsingMessageBox(this, "EStore", "Do you really want to remove this?"))
	{
		QString str("UPDATE item SET deleted = 1 WHERE item_id = " + itemId);
		QSqlQuery q;
		if (q.exec(str))
		{
			while (ui.tableWidget->rowCount() > 0)
			{
				ui.tableWidget->removeRow(0);
			}
			QSqlQuery displayQuery("SELECT * from Item WHERE deleted = 0");
			displayItems(displayQuery);
		}
	}
}

void ESManageItems::displayItems(QSqlQuery& queryItems)
{
	int row = 0;
	while (queryItems.next())
	{
		row = ui.tableWidget->rowCount();
		ui.tableWidget->insertRow(row);
		QString itemId = queryItems.value(0).toString();

		ui.tableWidget->setItem(row, 0, new QTableWidgetItem(queryItems.value(2).toString()));
		ui.tableWidget->setItem(row, 1, new QTableWidgetItem(queryItems.value(3).toString()));
		ui.tableWidget->setItem(row, 5, new QTableWidgetItem(queryItems.value(5).toString()));
		ui.tableWidget->setItem(row, 7, new QTableWidgetItem(queryItems.value(6).toString()));
		
		QSqlQuery queryCategories("SELECT * FROM item_category WHERE itemcategory_id = " + queryItems.value(1).toString());
		if (queryCategories.next())
		{
			ui.tableWidget->setItem(row, 2, new QTableWidgetItem(queryCategories.value(2).toString()));
		}

		QSqlQuery queryStocks("SELECT * FROM stock WHERE item_id = " + itemId);
		if (queryStocks.next())
		{
			ui.tableWidget->setItem(row, 3, new QTableWidgetItem(queryStocks.value(3).toString()));
			ui.tableWidget->setItem(row, 4, new QTableWidgetItem(queryStocks.value(4).toString()));

			QSqlQuery queryPrices("SELECT * FROM item_price WHERE itemprice_id = " + queryStocks.value(2).toString());
			if (queryPrices.next())
			{
				ui.tableWidget->setItem(row, 6, new QTableWidgetItem(queryPrices.value(2).toString()));
			}
		}
		else
		{
			ui.tableWidget->setItem(row, 3, new QTableWidgetItem("Not in Stock"));
			ui.tableWidget->setItem(row, 4, new QTableWidgetItem("Not in Stock"));
			ui.tableWidget->setItem(row, 6, new QTableWidgetItem("Not in Stock"));
		}

		
		QWidget* base = new QWidget(ui.tableWidget);
		QPushButton* updateBtn = new QPushButton("Update", base);
		updateBtn->setMaximumWidth(100);
		QPushButton* removeBtn = new QPushButton("Remove", base);
		removeBtn->setMaximumWidth(100);

		m_updateButtonSignalMapper->setMapping(updateBtn, itemId);
		QObject::connect(updateBtn, SIGNAL(clicked()), m_updateButtonSignalMapper, SLOT(map()));

		QObject::connect(removeBtn, SIGNAL(clicked()), m_removeButtonSignalMapper, SLOT(map()));
		m_removeButtonSignalMapper->setMapping(removeBtn, itemId);

		QHBoxLayout *layout = new QHBoxLayout;
		layout->setContentsMargins(0, 0, 0, 0);
		layout->addWidget(updateBtn);
		layout->addWidget(removeBtn);
		layout->insertStretch(2);
		base->setLayout(layout);
		ui.tableWidget->setCellWidget(row, 8, base);
		base->show();
	}

}

void ESManageItems::slotSearch()
{
	QString searchText = ui.searchTextBox->text();
	int categoryId = ui.categoryComboBox->currentData().toInt();
	
	while (ui.tableWidget->rowCount() > 0)
	{
		ui.tableWidget->removeRow(0);
	}

	QString searchQuery = "SELECT * FROM item ";
	bool useAnd = false;
	if (categoryId != -1)
	{
		searchQuery.append(" WHERE itemcategory_id = ");
		QString catId;
		catId.setNum(categoryId);
		searchQuery.append(catId);
		useAnd = true;
	}

	if (!searchText.isEmpty())
	{
		if (useAnd)
		{
			searchQuery.append(" AND ");
		}
		else
		{
			searchQuery.append(" WHERE ");
		}
		searchQuery.append(" (item_code LIKE '%" + searchText + "%' OR item_name LIKE '%" + searchText + "%')");
	}

	ui.tableWidget->setSortingEnabled(false);
	QSqlQuery queryItems(searchQuery);
	displayItems(queryItems);
	ui.tableWidget->setSortingEnabled(true);
}
