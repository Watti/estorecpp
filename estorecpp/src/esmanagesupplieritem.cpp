#include "esmanagesupplieritem.h"
#include "utility/esmainwindowholder.h"
#include "utility/esdbconnection.h"
#include "utility/utility.h"
#include <QMessageBox>

ESManageSupplierItem::ESManageSupplierItem(QWidget *parent /*= 0*/) : QWidget(parent)
{
	ui.setupUi(this);

	QStringList headerLabels;
	headerLabels.append("Item ID");
	headerLabels.append("Item Code");
	headerLabels.append("Item Name");
	headerLabels.append("Category");
	headerLabels.append("Min. Qty");
	headerLabels.append("Unit");

	ui.itemTableWidget->setHorizontalHeaderLabels(headerLabels);
	ui.itemTableWidget->horizontalHeader()->setStretchLastSection(true);
	ui.itemTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.itemTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.itemTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.itemTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.itemTableWidget->hideColumn(0);

	QStringList headerLabels1;
	headerLabels1.append("Item ID");
	headerLabels1.append("Item Code");
	headerLabels1.append("Item Name");
	headerLabels1.append("Category");
	headerLabels1.append("Purchasing Price");
	headerLabels1.append("Qty");

	ui.selectedItemTableWidget->setHorizontalHeaderLabels(headerLabels1);
	ui.selectedItemTableWidget->horizontalHeader()->setStretchLastSection(true);
	ui.selectedItemTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.selectedItemTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.selectedItemTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.selectedItemTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.selectedItemTableWidget->hideColumn(0);

	QObject::connect(ui.searchTextBox, SIGNAL(textChanged(QString)), this, SLOT(slotSearch()));
	QObject::connect(ui.categoryComboBox, SIGNAL(activated(QString)), this, SLOT(slotSearch()));

	if (!ES::DbConnection::instance()->open())
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Critical);
		mbox.setText(QString("Cannot connect to the database : AddItem"));
		mbox.exec();
	}
	else
	{
		QSqlQuery queryCategory("SELECT * FROM item_category WHERE deleted = 0");
		QString catCode = DEFAULT_DB_COMBO_VALUE;
		int catId = -1;

		ui.categoryComboBox->addItem(catCode, catId);

		while (queryCategory.next())
		{
			catId = queryCategory.value(0).toInt();
			ui.categoryComboBox->addItem(queryCategory.value(1).toString() + " / " + queryCategory.value("itemcategory_name").toString(), catId);
		}

		slotSearch();
	}
}

ESManageSupplierItem::~ESManageSupplierItem()
{

}

void ESManageSupplierItem::slotSearch()
{
	QString searchText = ui.searchTextBox->text();
	int categoryId = ui.categoryComboBox->currentData().toInt();

	while (ui.itemTableWidget->rowCount() > 0)
	{
		ui.itemTableWidget->removeRow(0);
	}

	QString searchQuery = "SELECT * FROM item ";
	bool categorySelected = false;
	if (categoryId != -1)
	{
		searchQuery.append(" WHERE deleted =0 AND itemcategory_id = ");
		QString catId;
		catId.setNum(categoryId);
		searchQuery.append(catId);
		categorySelected = true;
	}

	if (!searchText.isEmpty())
	{
		if (categorySelected)
		{
			searchQuery.append(" AND ");
		}
		else
		{
			searchQuery.append(" WHERE deleted = 0 AND ");
		}
		searchQuery.append(" (item_code LIKE '%" + searchText + "%' OR item_name LIKE '%" + searchText + "%')");
	}
	else
	{
		if (!categorySelected)
		{
			searchQuery.append(" WHERE deleted = 0");
		}
	}
	ui.itemTableWidget->setSortingEnabled(false);
	QSqlQuery queryItems(searchQuery);
	displayItems(queryItems);
	ui.itemTableWidget->setSortingEnabled(true);
}

void ESManageSupplierItem::slotItemDoubleClicked(int row, int col)
{

}

void ESManageSupplierItem::displayItems(QSqlQuery& queryItems)
{
	int row = 0;
	while (queryItems.next())
	{
		row = ui.itemTableWidget->rowCount();
		ui.itemTableWidget->insertRow(row);

		QSqlQuery queryCategories("SELECT * FROM item_category WHERE itemcategory_id = " + queryItems.value(1).toString());
		if (queryCategories.next())
		{
			ui.itemTableWidget->setItem(row, 3, new QTableWidgetItem(queryCategories.value(2).toString()));
		}

		ui.itemTableWidget->setItem(row, 0, new QTableWidgetItem(queryItems.value(0).toString()));
		ui.itemTableWidget->setItem(row, 1, new QTableWidgetItem(queryItems.value(2).toString()));
		ui.itemTableWidget->setItem(row, 2, new QTableWidgetItem(queryItems.value(3).toString()));

		QSqlQuery queryMinQty("SELECT min_qty FROM stock WHERE item_id = " + queryItems.value(0).toString());
		if (queryMinQty.next())
		{
			ui.itemTableWidget->setItem(row, 4, new QTableWidgetItem(queryMinQty.value(0).toString()));
		}
		else
		{
			ui.itemTableWidget->setItem(row, 4, new QTableWidgetItem("N/A"));
		}
		ui.itemTableWidget->setItem(row, 5, new QTableWidgetItem(queryItems.value(5).toString()));
	}
}