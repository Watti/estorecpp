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
	headerLabels.append("Stock ID");
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
	ui.tableWidget->hideColumn(0);
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
		QSqlQuery queryCategory("SELECT * FROM item_category WHERE deleted = 0");
		QStringList catogory;
		catogory.append(DEFAULT_DB_COMBO_VALUE);
		while (queryCategory.next())
		{
			catogory.append(queryCategory.value("itemcategory_name").toString());
		}
		ui.categoryComboBox->addItems(catogory);

		slotSearch();
	}

}

ESManageStockItems::~ESManageStockItems()
{

}

void ESManageStockItems::slotSearch()
{
	while (ui.tableWidget->rowCount() > 0)
	{
		ui.tableWidget->removeRow(0);
	}

	QString text = ui.searchTextBox->text();
	QString selectedCategory = ui.categoryComboBox->currentText();

	QString q;
	q.append("SELECT * FROM stock JOIN item ON stock.item_id = item.item_id ");
	q.append("JOIN item_category ON item.itemcategory_id = item_category.itemcategory_id ");
	q.append("WHERE item.deleted = 0 ");

	if (!text.isEmpty())
	{
		q.append("AND (item.item_code LIKE '%");
		q.append(text);
		q.append("%' OR item.item_name LIKE '%");
		q.append(text);
		q.append("%' OR item.description LIKE '%");
		q.append(text);
		q.append("%') ");
	}
	if (!selectedCategory.isEmpty() && selectedCategory != DEFAULT_DB_COMBO_VALUE)
	{
		q.append("AND item_category.itemcategory_name = '");
		q.append(selectedCategory);
		q.append("' ");
	}
	
	QSqlQuery query;
	if (!query.exec(q))
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Critical);
		mbox.setText(QString("Something went wrong: Cannot acquire stock data"));
		mbox.exec();
	}
	else
	{
		while (query.next())
		{
			int row = ui.tableWidget->rowCount();
			ui.tableWidget->insertRow(row);

			QString stockId = query.value("stock_id").toString();

			ui.tableWidget->setItem(row, 0, new QTableWidgetItem(stockId));
			ui.tableWidget->setItem(row, 1, new QTableWidgetItem(query.value("item_code").toString()));
			ui.tableWidget->setItem(row, 2, new QTableWidgetItem(query.value("item_name").toString()));
			ui.tableWidget->setItem(row, 3, new QTableWidgetItem(query.value("itemcategory_name").toString()));
			ui.tableWidget->setItem(row, 4, new QTableWidgetItem(query.value("qty").toString()));
			ui.tableWidget->setItem(row, 5, new QTableWidgetItem(query.value("min_qty").toString()));
			ui.tableWidget->setItem(row, 6, new QTableWidgetItem(query.value("unit").toString()));
			ui.tableWidget->setItem(row, 7, new QTableWidgetItem(query.value("selling_price").toString()));
			ui.tableWidget->setItem(row, 8, new QTableWidgetItem(query.value("description").toString()));

			QWidget* base = new QWidget(ui.tableWidget);
			QPushButton* updateBtn = new QPushButton("Update", base);
			updateBtn->setMaximumWidth(100);
			QPushButton* removeBtn = new QPushButton("Remove", base);
			removeBtn->setMaximumWidth(100);

			m_updateButtonSignalMapper->setMapping(updateBtn, stockId);
			QObject::connect(updateBtn, SIGNAL(clicked()), m_updateButtonSignalMapper, SLOT(map()));

			m_removeButtonSignalMapper->setMapping(removeBtn, stockId);
			QObject::connect(removeBtn, SIGNAL(clicked()), m_removeButtonSignalMapper, SLOT(map()));

			QHBoxLayout *layout = new QHBoxLayout;
			layout->setContentsMargins(0, 0, 0, 0);
			layout->addWidget(updateBtn);
			layout->addWidget(removeBtn);
			layout->insertStretch(2);
			base->setLayout(layout);
			ui.tableWidget->setCellWidget(row, 9, base);
			base->show();
		}
	}

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

