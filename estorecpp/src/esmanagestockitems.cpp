#include "esmanagestockitems.h"
#include "esaddstockitem.h"
#include "utility/esdbconnection.h"
#include "utility/esmainwindowholder.h"
#include <QMessageBox>
#include <QPushButton>
#include "utility/utility.h"


ESManageStockItems::ESManageStockItems(QWidget *parent /*= 0*/)
: QWidget(parent)
{
	ui.setupUi(this);
	m_updateButtonSignalMapper = new QSignalMapper(this);
	m_removeButtonSignalMapper = new QSignalMapper(this);
	m_addToStockButtonSignalMapper = new QSignalMapper(this);

	QObject::connect(m_updateButtonSignalMapper, SIGNAL(mapped(QString)), this, SLOT(slotUpdate(QString)));
	QObject::connect(m_removeButtonSignalMapper, SIGNAL(mapped(QString)), this, SLOT(slotRemove(QString)));
	QObject::connect(m_addToStockButtonSignalMapper, SIGNAL(mapped(QString)), this, SLOT(slotAddToStock(QString)));

	QStringList headerLabels;
	headerLabels.append("Item Code");
	headerLabels.append("Item Name");
	headerLabels.append("Category");
	headerLabels.append("Qty");
	headerLabels.append("Min.Qty");
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

	QObject::connect(ui.searchTextBox, SIGNAL(textChanged(QString)), this, SLOT(slotSearch()));
	QObject::connect(ui.categoryComboBox, SIGNAL(activated(QString)), this, SLOT(slotSearch()));
	QObject::connect(ui.inStockCheckBox, SIGNAL(stateChanged(int)), this, SLOT(slotSearch()));
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
		QStringList catogory;
		catogory.append(DEFAULT_DB_COMBO_VALUE);
		while (queryCategory.next())
		{
			catogory.append(queryCategory.value(2).toString());
		}
		ui.categoryComboBox->addItems(catogory);

		// populate with data
		displayStockItems();
	}

}

ESManageStockItems::~ESManageStockItems()
{

}

void ESManageStockItems::slotUpdate(QString itemId)
{
	AddStockItem* addStockItem = new AddStockItem(this);
	addStockItem->getUI().groupBox->setTitle("Update Stock Item");
	addStockItem->getUI().itemIDLabel->setText(itemId);
	addStockItem->setItemId(itemId);
	QSqlQuery query("SELECT selling_price, quantity FROM stock_order WHERE item_id = " + itemId);
	while (query.next())
	{
		QString price = query.value("selling_price").toString();
		addStockItem->getUI().itemPrice->setText(price);
		QString quantity = query.value("quantity").toString();
		addStockItem->getUI().qty->setText(quantity);
		bool isValid = false;
		double qtyDouble = quantity.toDouble(&isValid);
		if (isValid)
		{
			addStockItem->setExistingQuantityInMainStock(qtyDouble);
		}
	}

	QSqlQuery queryStock("SELECT * FROM stock where item_id = " + itemId);
	while (queryStock.next())
	{
		QString minqty = queryStock.value("min_qty").toString(), qty = queryStock.value("qty").toString();
		addStockItem->getUI().minQty->setText(minqty);
		addStockItem->getUI().qty->setText(qty);
		addStockItem->setExistingQuantityInStock(qty.toDouble());
	}
	addStockItem->setUpdate(true);
	ES::MainWindowHolder::instance()->getMainWindow()->setCentralWidget(addStockItem);
	addStockItem->show();
}

void ESManageStockItems::slotRemove(QString itemId)
{
	if (ES::Utility::verifyUsingMessageBox(this, "EStore", "Do you really want to remove this?"))
	{
		QString str("UPDATE stock SET deleted = 1 WHERE item_id = " + itemId);
		QSqlQuery q;
		if (q.exec(str))
		{
			while (ui.tableWidget->rowCount() > 0)
			{
				ui.tableWidget->removeRow(0);
			}
			displayStockItems();
		}
	}

}

void ESManageStockItems::slotSearch()
{
	while (ui.tableWidget->rowCount() > 0)
	{
		ui.tableWidget->removeRow(0);
	}

	QString text = ui.searchTextBox->text();
	bool inStockChecked = false, comboSelected = false;
	QString selectedCategory = "";
	if (text.isEmpty())
	{
		if (ui.inStockCheckBox->isChecked())
		{
			inStockChecked = true;
		}
		if (!ui.categoryComboBox->currentText().isEmpty())
		{
			selectedCategory = ui.categoryComboBox->currentText();
			if (selectedCategory != DEFAULT_DB_COMBO_VALUE)
			{
				comboSelected = true;
			}
		}
		if (comboSelected && inStockChecked)
		{
			//TODO
			QString q = "SELECT i.item_id, i.item_name, i.item_code, i.unit, i.description, i.itemcategory_id, s.qty, s.min_qty FROM item as i, stock as s WHERE s.item_id =  i.item_id AND i.deleted = 0 AND s.qty > 0";
			QSqlQuery query(q);
			while (query.next())
			{
				QStringList rowItems;
				QString itemId = query.value("item_id").toString(), description = query.value("description").toString(),
					code = query.value("item_code").toString(), itemName = query.value("item_name").toString(),
					unit = query.value("unit").toString(), categoryName = "", /*itemPriceId = query.value("itemprice_id").toString(),*/
					itemCategoryId = query.value("itemcategory_id").toString(), qty = query.value("qty").toString(),
					minQty = query.value("min_qty").toString();

				QSqlQuery query1("SELECT itemcategory_name FROM item_category WHERE itemcategory_id =" + itemCategoryId);
				if (query1.first())
				{
					categoryName = query1.value("itemcategory_name").toString();
					if (categoryName == selectedCategory)
					{
						QSqlQuery query2("SELECT selling_price, discount_type FROM stock_order WHERE item_id =" + itemId);
						if (query2.first())
						{
							QString unitPrice = query2.value("selling_price").toString(), discount = query2.value("discount_type").toString();

							rowItems.append(code);
							rowItems.append(itemName);
							rowItems.append(categoryName);
							rowItems.append(qty);
							rowItems.append(minQty);
							rowItems.append(unit);
							rowItems.append(unitPrice);
							rowItems.append(description);
							displayStockTableRow(rowItems, itemId, inStockChecked);
						}
					}
				}
			}
		}
		else if (comboSelected)
		{
			slotCombo(selectedCategory);
		}
		else if (inStockChecked)
		{
			slotInStock(1);
		}
		else
		{
			displayStockItems();
		}
	}
	else
	{
		if (ui.inStockCheckBox->isChecked())
		{
			inStockChecked = true;
		}
		if (!ui.categoryComboBox->currentText().isEmpty())
		{
			selectedCategory = ui.categoryComboBox->currentText();
			if (selectedCategory != DEFAULT_DB_COMBO_VALUE)
			{
				comboSelected = true;
			}
		}
		if (!text.isEmpty())
		{
			QString q = "SELECT * FROM item";
			q.append(" WHERE deleted = 0 AND ( item_code LIKE '%" + text + "%' OR item_name LIKE '%" + text + "%')");

			QSqlQuery query1(q);
			while (query1.next())
			{
				QStringList rowItems;
				QString itemId = query1.value(0).toString(), description = "", code = "", itemName = "", categoryName = "";
				QString itemCategoryId = query1.value(1).toString();
				code = query1.value(2).toString();
				itemName = query1.value(3).toString();

				QSqlQuery query3("SELECT * FROM item_category WHERE itemcategory_id=" + itemCategoryId);
				if (query3.first())
				{
					description = query3.value(2).toString();
					categoryName = query3.value("itemcategory_name").toString();
				}

				QString qty = DEFAULT_DB_NUMERICAL_TO_DISPLAY;
				QString minQty = DEFAULT_DB_NUMERICAL_TO_DISPLAY;
				QString unitPrice = DEFAULT_DB_NUMERICAL_TO_DISPLAY;

				QSqlQuery query2("SELECT * FROM stock WHERE item_id=" + itemId);
				if (query2.first())
				{
					qty = query2.value(3).toString();
					minQty = query2.value(4).toString();

					QSqlQuery query4("SELECT * from stock_order WHERE item_id=" + itemId);
					if (query4.first())
					{
						unitPrice = query4.value("selling_price").toString();
					}
				}

				rowItems.append(code);
				rowItems.append(itemName);
				rowItems.append(description);
				rowItems.append(qty);
				rowItems.append(minQty);
				rowItems.append(query1.value(5).toString());
				rowItems.append(unitPrice);
				rowItems.append(query1.value(6).toString());

				bool inStock = false;
				if (qty != DEFAULT_DB_NUMERICAL_TO_DISPLAY && qty.toInt() >= 0)
				{
					inStock = true;
				}
				if (inStockChecked || comboSelected)
				{
					if (inStockChecked && comboSelected)
					{
						if ((qty != DEFAULT_DB_NUMERICAL_TO_DISPLAY && qty.toInt() > 0) && selectedCategory == categoryName)
						{
							displayStockTableRow(rowItems, itemId, inStock);
						}
					}
					else if (inStockChecked || comboSelected)
					{
						if (inStockChecked && qty != DEFAULT_DB_NUMERICAL_TO_DISPLAY && qty > 0)
						{
							displayStockTableRow(rowItems, itemId, inStock);
						}
						if (comboSelected && selectedCategory == categoryName)
						{
							displayStockTableRow(rowItems, itemId, inStock);
						}
					}
				}
				else
				{
					displayStockTableRow(rowItems, itemId, inStock);
				}
			}
		}
		else
		{
			displayStockItems();
		}
	}

}

void ESManageStockItems::displayStockItems()
{
	QSqlQuery itemQuery("SELECT * FROM item WHERE deleted = 0");

	while (itemQuery.next())
	{
		QStringList rowItems;
		QString itemId = itemQuery.value(0).toString();
		QString itemCategoryId = itemQuery.value(1).toString();

		rowItems.append(itemQuery.value(2).toString());
		rowItems.append(itemQuery.value(3).toString());


		QString unitPrice = DEFAULT_DB_NUMERICAL_TO_DISPLAY;
		QSqlQuery stockOrderQuery("SELECT * from stock_order WHERE item_id=" + itemId);
		if (stockOrderQuery.first())
		{
			unitPrice = stockOrderQuery.value("selling_price").toString();

			QSqlQuery categoryQuery("SELECT * FROM item_category WHERE itemcategory_id=" + itemCategoryId);
			if (categoryQuery.first())
			{
				rowItems.append(categoryQuery.value(2).toString());
			}

			QString qty = DEFAULT_DB_NUMERICAL_TO_DISPLAY;
			QString minQty = DEFAULT_DB_NUMERICAL_TO_DISPLAY;

			QSqlQuery stockQuery("SELECT * FROM stock WHERE item_id=" + itemId + " AND deleted = 0");

			bool inStock = false;
			if (stockQuery.first())
			{
				qty = stockQuery.value("qty").toString();
				minQty = stockQuery.value("min_qty").toString();

				if (qty.toDouble() >= 0)
				{
					inStock = true;
				}
			}
			rowItems.append(qty);
			rowItems.append(minQty);
			rowItems.append(itemQuery.value(5).toString());
			rowItems.append(unitPrice);
			rowItems.append(itemQuery.value(6).toString());

			displayStockTableRow(rowItems, itemId, inStock);

		}
	}
}

void ESManageStockItems::displayStockTableRow(StockTableRow rowContent, QString itemId, bool inStock)
{
	int i = 0;
	int row = 0;
	row = ui.tableWidget->rowCount();
	ui.tableWidget->insertRow(row);
	for (auto col : rowContent)
	{
		QTableWidgetItem* tableItem = NULL;
		if (!inStock)
		{
			tableItem = new QTableWidgetItem(col);
			tableItem->setBackgroundColor(QColor(245, 208, 169));
		}
		else
		{
			tableItem = new QTableWidgetItem(col);
		}
		ui.tableWidget->setItem(row, i++, tableItem);
	}
	if (!inStock)
	{
		QWidget* base = new QWidget(ui.tableWidget);
		QPushButton* addToStockBtn = new QPushButton("Add To Stock", base);

		m_addToStockButtonSignalMapper->setMapping(addToStockBtn, itemId);
		QObject::connect(addToStockBtn, SIGNAL(clicked()), m_addToStockButtonSignalMapper, SLOT(map()));

		addToStockBtn->setMaximumWidth(120);
		addToStockBtn->setMinimumWidth(120);
		QHBoxLayout *layout = new QHBoxLayout;
		layout->setContentsMargins(0, 0, 0, 0);
		layout->addWidget(addToStockBtn);
		layout->insertStretch(1);
		base->setLayout(layout);
		ui.tableWidget->setCellWidget(row, i, base);
		base->show();
	}
	else
	{
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
		ui.tableWidget->setCellWidget(row, i, base);
		base->show();
	}
}

void ESManageStockItems::slotCombo(QString text)
{
	while (ui.tableWidget->rowCount() > 0)
	{
		ui.tableWidget->removeRow(0);
	}
	QString q("SELECT * FROM item, item_category WHERE item.itemcategory_id = item_category.itemcategory_id AND (item.deleted = 0) AND item_category.itemcategory_name = '" + text + "'");

	QSqlQuery query(q);
	while (query.next())
	{
		QStringList rowItems;
		QString itemId = query.value("item_id").toString();
		QString code = query.value("item_code").toString();
		QString name = query.value("item_name").toString();
		QString categoryName = query.value("itemcategory_name").toString();
		QString description = query.value("description").toString();
		QString unit = query.value("unit").toString();
		QString qty = DEFAULT_DB_NUMERICAL_TO_DISPLAY, minQty = DEFAULT_DB_NUMERICAL_TO_DISPLAY, unitPrice = DEFAULT_DB_NUMERICAL_TO_DISPLAY;
		QSqlQuery query1("SELECT * from stock_order WHERE item_id=" + itemId);

		bool inStock = false;
		if (query1.first())
		{
			unitPrice = query1.value("selling_price").toString();
			QSqlQuery query2("SELECT * FROM stock WHERE item_id = " + itemId + " AND deleted = 0");
			if (query2.first())
			{
				qty = query2.value("qty").toString();
				minQty = query2.value("min_qty").toString();

				if (qty != DEFAULT_DB_NUMERICAL_TO_DISPLAY && qty.toInt() >= 0)
				{
					inStock = true;
				}
			}
		}

		rowItems.append(code);
		rowItems.append(name);
		rowItems.append(categoryName);
		rowItems.append(qty);
		rowItems.append(minQty);
		rowItems.append(unit);
		rowItems.append(unitPrice);
		rowItems.append(description);
		displayStockTableRow(rowItems, itemId, inStock);
	}
}

void ESManageStockItems::slotInStock(int checked)
{
	while (ui.tableWidget->rowCount() > 0)
	{
		ui.tableWidget->removeRow(0);
	}

	if (checked > 0)
	{
		QString text;
		QString q("SELECT * FROM item WHERE deleted = 0");

		QSqlQuery query(q);
		while (query.next())
		{
			QString itemId = query.value("item_id").toString();
			QString code = query.value("item_code").toString();
			QString name = query.value("item_name").toString();
			QString categoryName = query.value("itemcategory_name").toString();
			QString description = query.value("description").toString();
			QString unit = query.value("unit").toString();
			QString qty = DEFAULT_DB_NUMERICAL_TO_DISPLAY, minQty = DEFAULT_DB_NUMERICAL_TO_DISPLAY, unitPrice = DEFAULT_DB_NUMERICAL_TO_DISPLAY;
			QSqlQuery query1("SELECT * from stock WHERE item_id = " + itemId + " AND deleted = 0");
			QStringList rowItems;
			if (query1.first())
			{
				qty = query1.value("qty").toString();
				minQty = query1.value("min_qty").toString();
				if ((qty != DEFAULT_DB_NUMERICAL_TO_DISPLAY && qty.toInt() > 0) && (minQty != DEFAULT_DB_NUMERICAL_TO_DISPLAY && minQty.toInt() > 0))
				{
					QSqlQuery query2("SELECT * FROM stock_order WHERE item_id=" + itemId);
					if (query2.first())
					{
						unitPrice = query1.value("selling_price").toString();

						rowItems.append(code);
						rowItems.append(name);
						rowItems.append(categoryName);
						rowItems.append(qty);
						rowItems.append(minQty);
						rowItems.append(unit);
						rowItems.append(unitPrice);
						rowItems.append(description);
						displayStockTableRow(rowItems, itemId, true);
					}
				}
			}
		}
	}
	else
	{
		displayStockItems();
	}
}

void ESManageStockItems::slotAddToStock(QString itemId)
{
	AddStockItem* addStockItem = new AddStockItem(this);
	addStockItem->getUI().groupBox->setTitle("Add Stock Item");
	addStockItem->getUI().itemIDLabel->setText(itemId);
	addStockItem->setItemId(itemId);
	QSqlQuery query("SELECT selling_price, quantity FROM stock_order WHERE item_id = " + itemId);
	while (query.next())
	{
		QString price = query.value("selling_price").toString();
		addStockItem->getUI().itemPrice->setText(price);
		QString quantity = query.value("quantity").toString();
		addStockItem->getUI().qty->setText(quantity);
		bool isValid = false;
		double qtyDouble = quantity.toDouble(&isValid);
		if (isValid)
		{
			addStockItem->setExistingQuantityInMainStock(qtyDouble);
		}
	}
	QSqlQuery queryStock("SELECT * FROM stock where item_id = " + itemId);
	while (queryStock.next())
	{
		QString minqty = queryStock.value("min_qty").toString(), qty = queryStock.value("qty").toString();
		addStockItem->getUI().minQty->setText(minqty);
		addStockItem->getUI().qty->setText(qty);
		addStockItem->setExistingQuantityInStock(qty.toDouble());
	}
	ES::MainWindowHolder::instance()->getMainWindow()->setCentralWidget(addStockItem);
	addStockItem->show();
}

