#include "esaddorderitem.h"
#include "esmanageorderitems.h"
#include "utility/esmainwindowholder.h"
#include "utility/esdbconnection.h"
#include <QMessageBox>
#include <QPushButton>
#include "utility/session.h"
#include "utility/utility.h"

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

	QObject::connect(ui.addOrderItemButton, SIGNAL(clicked()), this, SLOT(slotPlaceNewOrder()));
	QObject::connect(ui.searchTextBox, SIGNAL(textChanged(QString)), this, SLOT(slotSearch()));
	QObject::connect(ui.categoryComboBox, SIGNAL(activated(QString)), this, SLOT(slotSearch()));

	QObject::connect(ui.itemTableWidget, SIGNAL(cellClicked(int, int)), this, SLOT(slotItemSelected(int, int)));
	QObject::connect(ui.supplierTableWidget, SIGNAL(cellClicked(int, int)), this, SLOT(slotSupplierSelected(int, int)));

	if (!ES::DbConnection::instance()->open())
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Critical);
		mbox.setText(QString("Cannot connect to the database : AddItem"));
		mbox.exec();
	}
	else
	{
		QSqlQuery queryCategory("SELECT * FROM item_category");
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
	QString itemCode = ui.itemCode->text();
	QString itemId = "-1";
	QSqlQuery query1("SELECT * FROM item WHERE item_code = '" + itemCode + "'");
	if (query1.next())
	{
		itemId = query1.value(0).toString();
	}
	else
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Warning);
		mbox.setText(QString("Invalid Item Code : This item does not exist"));
		mbox.exec();
		return;
	}

	int userId = ES::Session::getInstance()->getUser()->getId();
	QString userIdStr;
	userIdStr.setNum(userId);
	QString price = ui.unitPrice->text();
	if (price.isNull() || price.isEmpty())
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Critical);
		mbox.setText(QString("Price is empty"));
		mbox.exec();
		return;
	}
	QString qty = ui.qty->text();
	if (qty.isNull() || qty.isEmpty())
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Critical);
		mbox.setText(QString("Quantity is empty"));
		mbox.exec();
		return;
	}
	QString description = ui.itemDescription->toPlainText();

	QString q;
	if (m_update)
	{
		q = "Update stock_order SET item_id = '" + itemId + "', user_id = '" + userIdStr + "', description = '" + description + "' ,unit_price = '"+price+"', selling_price = '"+price+"' , quantity = '"+qty+"' WHERE order_id = " + m_orderId;
	}
	else
	{
		q = "INSERT INTO stock_order (item_id, user_id, unit_price, selling_price, quantity, description) VALUES (" +
			itemId + "," + userIdStr + "," + price + "," +price+"," +qty + ",'" + description + "')";
	}
	QSqlQuery query;
	if (query.exec(q))
	{
		this->close();
		ESManageOrderItems* manageItems = new ESManageOrderItems();
		ES::MainWindowHolder::instance()->getMainWindow()->setCentralWidget(manageItems);
		this->close();
		manageItems->show();
	}
	else
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Critical);
		mbox.setText(QString("Something goes wrong:: Item cannot be saved"));
		mbox.exec();
	}

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
	ui.itemCode->clear();
	ui.itemDescription->clear();

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

void AddOrderItem::slotItemSelected(int row, int col)
{
	QTableWidgetItem* idCell = ui.itemTableWidget->item(row, 0);
	if (!idCell)
		return;

	QString query("SELECT * FROM item WHERE deleted = 0 AND item_id = ");
	query.append(idCell->text());
	QSqlQuery itemQry(query);
	if (itemQry.next())
	{
		ui.itemCode->setText(itemQry.value("item_code").toString());
		ui.itemDescription->setText(itemQry.value("description").toString());
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
		//ui.unitPrice->clear();
		ui.unitPrice->setText("350");
	}
}
