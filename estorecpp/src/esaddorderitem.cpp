#include "esaddorderitem.h"
#include "esmanageorderitems.h"
#include "utility/esmainwindowholder.h"
#include "utility/esdbconnection.h"
#include <QMessageBox>
#include <QPushButton>
#include "utility/session.h"
#include "utility/utility.h"
#include "entities/SaleLineEdit.h"

AddOrderItem::AddOrderItem(QWidget *parent/* = 0*/)
{
	m_update = false;
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

	QStringList headerLabels2;
	headerLabels2.append("Supplier ID");
	headerLabels2.append("Supplier Code");
	headerLabels2.append("Supplier Name");

	ui.supplierTableWidget->setHorizontalHeaderLabels(headerLabels2);
	ui.supplierTableWidget->horizontalHeader()->setStretchLastSection(true);
	ui.supplierTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.supplierTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.supplierTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.supplierTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.supplierTableWidget->hideColumn(0);

	ui.dateEdit->setDisplayFormat("yyyy-MM-dd");
	ui.dateEdit->setDate(QDate::currentDate());

	ui.showAllSuppliers->setChecked(true);

	QObject::connect(ui.addOrderItemButton, SIGNAL(clicked()), this, SLOT(slotPlaceNewOrder()));
	QObject::connect(ui.searchTextBox, SIGNAL(textChanged(QString)), this, SLOT(slotSearch()));
	QObject::connect(ui.categoryComboBox, SIGNAL(activated(QString)), this, SLOT(slotSearch()));

	QObject::connect(ui.supplierTableWidget, SIGNAL(cellClicked(int, int)), this, SLOT(slotSupplierSelected(int, int)));
	QObject::connect(ui.itemTableWidget, SIGNAL(cellDoubleClicked(int, int)), this, SLOT(slotItemDoubleClicked(int, int)));

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

AddOrderItem::~AddOrderItem()
{

}

void AddOrderItem::setUpdate(bool update)
{
	m_update = update;
}

void AddOrderItem::slotPlaceNewOrder()
{
	int userId = ES::Session::getInstance()->getUser()->getId();
	QString userIdStr;
	userIdStr.setNum(userId);

	int row = ui.supplierTableWidget->currentRow();
	QTableWidgetItem* idCell = ui.supplierTableWidget->item(row, 0);
	if (!idCell)
		return;

	QString supplierId = idCell->text();
	QDateTime d = QDateTime::fromString(ui.dateEdit->text(), Qt::ISODate);
	
	// Add data to 'purchase_order' table
	QString q = "INSERT INTO purchase_order (user_id, supplier_id, order_date, comments) VALUES (" +
		userIdStr + "," + supplierId + ",'" + d.toString("yyyy-MM-dd") + "','" + ui.comments->toPlainText() + "')";

	QSqlQuery query;
	if (!query.exec(q))
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Critical);
		mbox.setText(QString("Something goes wrong: order cannot be saved"));
		mbox.exec();
	}

	QString purchaseOrderId = query.lastInsertId().value<QString>();

	// Add data to 'purchase_order_item' table
	int rowCount = ui.selectedItemTableWidget->rowCount();
	for (int i = 0; i < rowCount; ++i)
	{
		QTableWidgetItem* idCell = ui.selectedItemTableWidget->item(i, 0);
		if (!idCell) continue;

		QString itemId = idCell->text();

		ES::SaleLineEdit* le = NULL;
		le = static_cast<ES::SaleLineEdit*>(ui.selectedItemTableWidget->cellWidget(i, 4));
		if (!le) continue;

		QString price = le->text();

		le = static_cast<ES::SaleLineEdit*>(ui.selectedItemTableWidget->cellWidget(i, 5));
		if (!le) continue;

		QString quantity = le->text();

		QString qry = "INSERT INTO purchase_order_item (purchaseorder_id, item_id, qty, purchasing_price) VALUES (" + 
			purchaseOrderId + "," + itemId + "," + quantity + "," + price + ")";

		QSqlQuery insertQuery;
		if (!insertQuery.exec(qry))
		{
			QMessageBox mbox;
			mbox.setIcon(QMessageBox::Critical);
			mbox.setText(QString("Something goes wrong: order items cannot be saved"));
			mbox.exec();
		}
	}

	ESManageOrderItems* manageItems = new ESManageOrderItems(ES::MainWindowHolder::instance()->getMainWindow());
	ES::MainWindowHolder::instance()->getMainWindow()->setCentralWidget(manageItems);
	this->close();
	manageItems->show();

}

QString AddOrderItem::getOrderId() const
{
	return m_orderId;
}

void AddOrderItem::setOrderId(QString val)
{
	m_orderId = val;
}

void AddOrderItem::slotSearch()
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

	while (ui.supplierTableWidget->rowCount() > 0)
	{
		ui.supplierTableWidget->removeRow(0);
	}

	QSqlQuery querySuppliers("SELECT * FROM supplier WHERE deleted = 0");
	int row = 0;
	while (querySuppliers.next())
	{
		row = ui.supplierTableWidget->rowCount();
		ui.supplierTableWidget->insertRow(row);

		ui.supplierTableWidget->setItem(row, 0, new QTableWidgetItem(querySuppliers.value(0).toString()));
		ui.supplierTableWidget->setItem(row, 1, new QTableWidgetItem(querySuppliers.value(1).toString()));
		ui.supplierTableWidget->setItem(row, 2, new QTableWidgetItem(querySuppliers.value(2).toString()));
	}

}

void AddOrderItem::displayItems(QSqlQuery& queryItems)
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

void AddOrderItem::slotSupplierSelected(int row, int col)
{
	QTableWidgetItem* idCell = ui.supplierTableWidget->item(row, 0);
	if (!idCell)
		return;

	QString query("SELECT * FROM supplier WHERE deleted = 0 AND supplier_id = ");
	query.append(idCell->text());
	QSqlQuery supplierQry(query);
	if (supplierQry.next())
	{
		ui.supplierCode->setText(supplierQry.value("supplier_code").toString());
		ui.supplierName->setText(supplierQry.value("supplier_name").toString());
	}

	int rowCount = ui.selectedItemTableWidget->rowCount();
	for (int i = 0; i < rowCount; i++)
	{
		ES::SaleLineEdit* le = static_cast<ES::SaleLineEdit*>(ui.selectedItemTableWidget->cellWidget(i, 4));
		if (le)
		{
			le->setText("325.50");
			le->setReadOnly(true);
		}
	}
}

void AddOrderItem::slotItemDoubleClicked(int row, int col)
{
	QTableWidgetItem* idCell = ui.itemTableWidget->item(row, 0);
	if (!idCell)
		return;
	
	QSqlQuery queryItems("SELECT * FROM item WHERE item_id = " + idCell->text());
	if (queryItems.next())
	{
		for (int i = 0; i < ui.selectedItemTableWidget->rowCount(); i++)
		{
			QTableWidgetItem* cell = ui.selectedItemTableWidget->item(i, 0);
			if (cell->text() == queryItems.value("item_id").toString())
			{
				ui.selectedItemTableWidget->setCurrentCell(i, 5);
				ES::SaleLineEdit* le = static_cast<ES::SaleLineEdit*>(ui.selectedItemTableWidget->cellWidget(i, 5));
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

		QList<QTableWidgetItem*> selected = ui.supplierTableWidget->selectedItems();
		if (!selected.empty() && selected.first())
		{
			int r = ui.supplierTableWidget->row(selected.first());
			int s = selected.size();
			lePrice->setText("1000");
		}

		ES::SaleLineEdit* leQty = new ES::SaleLineEdit(idCell->text(), row);
		ui.selectedItemTableWidget->setCellWidget(row, 5, leQty);
		leQty->setFocus();

	}
}
