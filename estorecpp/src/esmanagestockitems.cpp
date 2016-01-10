#include "esmanagestockitems.h"
#include "utility/esdbconnection.h"
#include <QMessageBox>
#include <QPushButton>


ESManageStockItems::ESManageStockItems(QWidget *parent /*= 0*/)
: QWidget(parent)
{
	ui.setupUi(this);
	m_updateButtonSignalMapper = new QSignalMapper(this);
	m_removeButtonSignalMapper = new QSignalMapper(this);

	QObject::connect(m_updateButtonSignalMapper, SIGNAL(mapped(QString)), this, SLOT(slotUpdate(QString)));
	QObject::connect(m_removeButtonSignalMapper, SIGNAL(mapped(QString)), this, SLOT(slotRemove(QString)));

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
		catogory.append("select");
		while (queryCategory.next())
		{
			catogory.append(queryCategory.value(2).toString());
		}
		ui.categoryComboBox->addItems(catogory);
	}
	// populate with data
	displayStockItems();

}

ESManageStockItems::~ESManageStockItems()
{

}

void ESManageStockItems::slotUpdate(QString itemId)
{
	QMessageBox mbox;
	mbox.setIcon(QMessageBox::Critical);
	mbox.setText(QString("slotUpdate : ") + itemId);
	mbox.exec();
}

void ESManageStockItems::slotRemove(QString itemId)
{
	QMessageBox mbox;
	mbox.setIcon(QMessageBox::Critical);
	mbox.setText(QString("slotRemove : ") + itemId);
	mbox.exec();

	if (!ES::DbConnection::instance()->open())
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Critical);
		mbox.setText(QString("Cannot connect to the database : ESManageStockItems::slotRemove "));
		mbox.exec();
	}
	else
	{
		QSqlQuery query;
		int id = itemId.toInt(); 

 		if (query.exec("UPDATE stock SET qty = 0, min_qty = 0 WHERE item_id = " + itemId))
		{
			QString itemPriceId = query.value("itemprice_id").toString();
			QString q("UPDATE item_price SET unit_price = 0, discount_type = 0 WHERE itemprice_id = (SELECT itemprice_id from stock where item_id = " + itemId + ")");
			QSqlQuery query1;
			if (query1.exec(q))
			{
				while (ui.tableWidget->rowCount() > 0)
				{
					ui.tableWidget->removeRow(0);
				}
				displayStockItems();
			}
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
			if (selectedCategory != "select")
			{
				comboSelected = true;
			}
		}
		if (comboSelected && inStockChecked)
		{
			//TODO
// 			QString q = "SELECT * FROM item, stock, item_category WHERE stock.item_id =  item.item_id, item.itemcategory_id = item_category.itemcategory_id AND stock.qty > 0 AND item_category.itemcategory_name ="+selectedCategory;
// 			QSqlQuery query1(q);
// 			while (query1.next())
// 			{
// 				QStringList rowItems;
// 				QString itemId = query1.value(0).toString(), description = "", code = "", itemName = "", categoryName = "";
// 				QString itemCategoryId = query1.value(1).toString();
// 				code = query1.value(2).toString();
// 				itemName = query1.value(3).toString();
// 
// 				QSqlQuery query3("SELECT * FROM item_category WHERE itemcategory_id=" + itemCategoryId);
// 				if (query3.first())
// 				{
// 					description = query3.value(2).toString();
// 					categoryName = query3.value("itemcategory_name").toString();
// 				}
// 			}
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
			if (selectedCategory != "select")
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

				QString qty = "";
				QString minQty = "";
				QString unitPrice = "";

				QSqlQuery query2("SELECT * FROM stock WHERE item_id=" + itemId);
				if (query2.first())
				{
					qty = query2.value(3).toString();
					minQty = query2.value(4).toString();

					QSqlQuery query4("SELECT * from item_price WHERE itemprice_id=" + query2.value(2).toString());
					if (query4.first())
					{
						unitPrice = query4.value(2).toString();
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

				bool inStock = true;
				if (qty == "" || qty.toInt() == 0)
				{
					inStock = false;
				}
				if (inStockChecked || comboSelected)
				{
					if (inStockChecked && comboSelected)
					{
						if ((qty != "" && qty.toInt() > 0) && selectedCategory == categoryName)
						{
							displayStockTableRow(rowItems, itemId, inStock);
						}
					}
					else if (inStockChecked || comboSelected)
					{
						if (inStockChecked && qty != "" && qty > 0)
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
	if (!ES::DbConnection::instance()->open())
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Critical);
		mbox.setText(QString("Cannot connect to the database : ESManageStockItems::displayStockItems "));
		mbox.exec();
	}
	else
	{
		QSqlQuery query1("SELECT * FROM item");

		while (query1.next())
		{
			QStringList rowItems;
			QString itemId = query1.value(0).toString();
			QString itemCategoryId = query1.value(1).toString();
			/*bool deleted = query1.value(7).toBool();*/

			rowItems.append(query1.value(2).toString());
			rowItems.append(query1.value(3).toString());
			QSqlQuery query3("SELECT * FROM item_category WHERE itemcategory_id=" + itemCategoryId);
			if (query3.first())
			{
				rowItems.append(query3.value(2).toString());
			}

			QString qty = "Not in stock";
			QString minQty = "Not in stock";
			QString unitPrice = "Not in stock";

			QSqlQuery query2("SELECT * FROM stock WHERE item_id=" + itemId);
			if (query2.first())
			{
				qty = query2.value(3).toString();
				minQty = query2.value(4).toString();

				QSqlQuery query4("SELECT * from item_price WHERE itemprice_id=" + query2.value(2).toString());
				if (query4.first())
				{
					unitPrice = query4.value(2).toString();
				}
			}

			rowItems.append(qty);
			rowItems.append(minQty);
			rowItems.append(query1.value(5).toString());
			rowItems.append(unitPrice);
			rowItems.append(query1.value(6).toString());
			bool inStock = true;

			if (qty == "Not in stock" || (qty.toInt() == 0))
			{
				inStock = false;
			}
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
		ui.tableWidget->setItem(row, i++, new QTableWidgetItem(col));
	}
	if (!inStock)
	{
		QWidget* base = new QWidget(ui.tableWidget);
		QPushButton* addToStockBtn = new QPushButton("Add To Stock", base);
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
	if (!ES::DbConnection::instance()->open())
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Critical);
		mbox.setText(QString("Cannot connect to the database : ESManageStockItems::slotCombo "));
		mbox.exec();
	}
	else
	{
		QString q("SELECT * FROM item, item_category WHERE item.itemcategory_id = item_category.itemcategory_id AND item_category.itemcategory_name = '" + text + "'");

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
			QString qty = "", minQty = "", unitPrice = "";
			QSqlQuery query1("SELECT * from item_price WHERE itemprice_id=" + itemId);
			if (query1.first())
			{
				unitPrice = query1.value("unit_price").toString();

				QSqlQuery query2("SELECT * FROM stock WHERE item_id=" + itemId);
				if (query2.first())
				{
					qty = query2.value("qty").toString();
					minQty = query2.value("min_qty").toString();
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
			bool inStock = true;

			if (qty == "")
			{
				inStock = false;
			}
			displayStockTableRow(rowItems, itemId, inStock);
		}
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
		if (!ES::DbConnection::instance()->open())
		{
			QMessageBox mbox;
			mbox.setIcon(QMessageBox::Critical);
			mbox.setText(QString("Cannot connect to the database : ESManageStockItems::slotCombo "));
			mbox.exec();
		}
		else
		{
			QString text;
			QString q("SELECT * FROM item");

			QSqlQuery query(q);
			while (query.next())
			{
				QString itemId = query.value("item_id").toString();
				QString code = query.value("item_code").toString();
				QString name = query.value("item_name").toString();
				QString categoryName = query.value("itemcategory_name").toString();
				QString description = query.value("description").toString();
				QString unit = query.value("unit").toString();
				QString qty = "", minQty = "", unitPrice = "";
				QSqlQuery query1("SELECT * from stock WHERE item_id=" + itemId);
				QStringList rowItems;
				if (query1.first())
				{
					qty = query1.value("qty").toString();
					minQty = query1.value("min_qty").toString();
					if ((qty != "" && qty.toInt() > 0) && (minQty != "" && minQty.toInt() > 0))
					{
						QString itemPriceId = query1.value("itemprice_id").toString();
						QSqlQuery query2("SELECT * FROM item_price WHERE itemprice_id=" + itemPriceId);
						if (query2.first())
						{
							unitPrice = query1.value("unit_price").toString();

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

// 					QSqlQuery query2("SELECT * FROM stock WHERE item_id=" + itemId);
// 					if (query2.first())
// 					{
// 						qty = query2.value("qty").toString();
// 						minQty = query2.value("min_qty").toString();
// 						if ((qty != "" && qty.toInt() > 0) && (minQty != "" && minQty.toInt() > 0))
// 						{
// 							rowItems.append(code);
// 							rowItems.append(name);
// 							rowItems.append(categoryName);
// 							rowItems.append(qty);
// 							rowItems.append(minQty);
// 							rowItems.append(unit);
// 							rowItems.append(unitPrice);
// 							rowItems.append(description);
// 							displayStockTableRow(rowItems, itemId, true);
// 						}
// 					}
				}
			}
		}
	}
	else
	{
		displayStockItems();
	}
}

