#include "esmanagesupplieritem.h"
#include "utility/esmainwindowholder.h"
#include "utility/esdbconnection.h"
#include "utility/utility.h"
#include <QMessageBox>
#include "entities/SaleLineEdit.h"

ESManageSupplierItem::ESManageSupplierItem(QString supplierId, QWidget *parent /*= 0*/) : QWidget(parent)
{
	m_supplierId = supplierId;

	ui.setupUi(this);

	m_removeButtonSignalMapper = new QSignalMapper(this);

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
	headerLabels1.append("Actions");

	ui.selectedItemTableWidget->setHorizontalHeaderLabels(headerLabels1);
	ui.selectedItemTableWidget->horizontalHeader()->setStretchLastSection(true);
	ui.selectedItemTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.selectedItemTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.selectedItemTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.selectedItemTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.selectedItemTableWidget->hideColumn(0);

	QObject::connect(ui.searchTextBox, SIGNAL(textChanged(QString)), this, SLOT(slotSearch()));
	QObject::connect(ui.categoryComboBox, SIGNAL(activated(QString)), this, SLOT(slotSearch()));
	QObject::connect(ui.itemTableWidget, SIGNAL(cellDoubleClicked(int, int)), this, SLOT(slotItemDoubleClicked(int, int)));
	QObject::connect(ui.addSupplierItemBtn, SIGNAL(clicked()), this, SLOT(slotAddSupplierItems()));
	QObject::connect(m_removeButtonSignalMapper, SIGNAL(mapped(QString)), this, SLOT(slotRemove(QString)));

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
	QTableWidgetItem* idCell = ui.itemTableWidget->item(row, 0);
	if (!idCell)
		return;

	QSqlQuery queryItems("SELECT * FROM item WHERE deleted = 0 AND item_id = " + idCell->text());
	if (queryItems.next())
	{
		for (int i = 0; i < ui.selectedItemTableWidget->rowCount(); i++)
		{
			QTableWidgetItem* cell = ui.selectedItemTableWidget->item(i, 0);
			if (cell->text() == queryItems.value("item_id").toString())
			{
				ui.selectedItemTableWidget->setCurrentCell(i, 4);
				ES::SaleLineEdit* le = static_cast<ES::SaleLineEdit*>(ui.selectedItemTableWidget->cellWidget(i, 4));
				if (le) le->setFocus();
				return;
			}
		}

		int row = ui.selectedItemTableWidget->rowCount();
		ui.selectedItemTableWidget->insertRow(row);

		ui.selectedItemTableWidget->setItem(row, 0, new QTableWidgetItem(queryItems.value("item_id").toString()));
		ui.selectedItemTableWidget->setItem(row, 1, new QTableWidgetItem(queryItems.value("item_code").toString()));
		ui.selectedItemTableWidget->setItem(row, 2, new QTableWidgetItem(queryItems.value("item_name").toString()));

		QSqlQuery queryCategories("SELECT * FROM item_category WHERE itemcategory_id = " + queryItems.value("itemcategory_id").toString());
		if (queryCategories.next())
		{
			ui.selectedItemTableWidget->setItem(row, 3, new QTableWidgetItem(queryCategories.value("itemcategory_name").toString()));
		}

		ES::SaleLineEdit* lePrice = new ES::SaleLineEdit(idCell->text(), row);
		ui.selectedItemTableWidget->setCellWidget(row, 4, lePrice);
		lePrice->setFocus();

		QWidget* base = new QWidget(ui.selectedItemTableWidget);
		QPushButton* removeBtn = new QPushButton("Remove", base);
		removeBtn->setMaximumWidth(100);

		QObject::connect(removeBtn, SIGNAL(clicked()), m_removeButtonSignalMapper, SLOT(map()));
		m_removeButtonSignalMapper->setMapping(removeBtn, queryItems.value("item_id").toString());

		QHBoxLayout *layout = new QHBoxLayout;
		layout->setContentsMargins(0, 0, 0, 0);
		layout->addWidget(removeBtn);
		layout->insertStretch(2);
		base->setLayout(layout);
		ui.selectedItemTableWidget->setCellWidget(row, 5, base);
		base->show();
	}

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

void ESManageSupplierItem::slotRemove(QString id)
{
	for (int i = 0; i < ui.selectedItemTableWidget->rowCount(); i++)
	{
		QTableWidgetItem* cell = ui.selectedItemTableWidget->item(i, 0);
		if (cell->text() == id)
		{
			ui.selectedItemTableWidget->removeRow(i);
			slotSearch();
			return;
		}
	}
}

void ESManageSupplierItem::slotAddSupplierItems()
{
	for (int i = 0; i < ui.selectedItemTableWidget->rowCount(); i++)
	{
		QTableWidgetItem* idCell = ui.selectedItemTableWidget->item(i, 0);
		QWidget* priceCell = ui.selectedItemTableWidget->cellWidget(i, 4);
		ES::SaleLineEdit* le = static_cast<ES::SaleLineEdit*>(priceCell);

		QString q("INSERT INTO supplier_item (supplier_id, item_id, purchasing_price) VALUES (");
		q.append(m_supplierId);
		q.append(", ");
		q.append(idCell->text());
		q.append(", ");
		q.append(le->text());
		q.append(")");

		QSqlQuery query;
		if (query.exec(q))
		{
			QMessageBox mbox;
			mbox.setIcon(QMessageBox::Information);
			mbox.setText(QString("Success"));
			mbox.exec();

			while (ui.selectedItemTableWidget->rowCount() > 0)
			{
				ui.selectedItemTableWidget->removeRow(0);
			}
		}
	}
}
