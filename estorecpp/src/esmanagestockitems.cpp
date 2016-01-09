#include "esmanagestockitems.h"
#include "utility/esdbconnection.h"
#include <QMessageBox>
#include <QPushButton>


ESManageStockItems::ESManageStockItems(QWidget *parent /*= 0*/)
: QWidget(parent)
{
	ui.setupUi(this);

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
	ui.tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.prevBtn->setDisabled(true);
	ui.nextBtn->setDisabled(true);

	QObject::connect(ui.searchTextBox, SIGNAL(textChanged(QString)), this, SLOT(slotSearch(QString)));
	QObject::connect(ui.categoryComboBox, SIGNAL(activated(QString)), this, SLOT(slotSearch(QString)));
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

void ESManageStockItems::slotUpdate()
{

}

void ESManageStockItems::slotRemove(QString itemId)
{

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
		if (query.exec("UPDATE item SET deleted = 1 WHERE item_id = " + itemId))
		{
			while (ui.tableWidget->rowCount() > 0)
			{
				ui.tableWidget->removeRow(0);
			}
			displayStockItems();
		}
		else
		{

			QMessageBox mbox;
			mbox.setIcon(QMessageBox::Warning);
			mbox.setText(QString("Unable to remove selected item"));
			mbox.exec();
		}

	}

}

void ESManageStockItems::slotSearch(QString text)
{
	while (ui.tableWidget->rowCount() > 0)
	{
		ui.tableWidget->removeRow(0);
	}

	QString q = "SELECT * FROM item WHERE deleted = 0";

	if (ui.inStockCheckBox->isChecked())
	{
		q = "SELECT * FROM item";
		//q.append(" WHERE ");
	}

	if (!ui.categoryComboBox->currentText().isEmpty())
	{
		ui.categoryComboBox->currentText();
	}

	if (!text.isEmpty())
	{
		q = "SELECT * FROM item";
		q.append(" WHERE deleted = 0 AND ( item_code LIKE '%" + text + "%' OR item_name LIKE '%" + text + "%')");
	}

	QSqlQuery query1(q);
	while (query1.next())
	{
		QString itemId = query1.value(0).toString();
		QString itemCategoryId = query1.value(1).toString();

		int row = ui.tableWidget->rowCount();
		ui.tableWidget->insertRow(row);
		ui.tableWidget->setItem(row, 0, new QTableWidgetItem(query1.value(2).toString()));
		ui.tableWidget->setItem(row, 1, new QTableWidgetItem(query1.value(3).toString()));

		QSqlQuery query3("SELECT * FROM item_category WHERE itemcategory_id=" + itemCategoryId);
		if (query3.first())
		{
			ui.tableWidget->setItem(row, 2, new QTableWidgetItem(query3.value(2).toString()));
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

		ui.tableWidget->setItem(row, 3, new QTableWidgetItem(qty));
		ui.tableWidget->setItem(row, 4, new QTableWidgetItem(minQty));
		ui.tableWidget->setItem(row, 5, new QTableWidgetItem(query1.value(5).toString()));
		ui.tableWidget->setItem(row, 6, new QTableWidgetItem(unitPrice));
		ui.tableWidget->setItem(row, 7, new QTableWidgetItem(query1.value(6).toString()));

		if (qty == "Not in stock")
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
			ui.tableWidget->setCellWidget(row, 8, base);
			base->show();
		}
		else
		{
			QWidget* base = new QWidget(ui.tableWidget);
			QPushButton* updateBtn = new QPushButton("Update", base);
			updateBtn->setMaximumWidth(100);
			QPushButton* removeBtn = new QPushButton("Remove", base);
			removeBtn->setMaximumWidth(100);

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
		QSqlQuery query1("SELECT * FROM item WHERE deleted = 0");
		
		while (query1.next())
		{
			QStringList rowItems;
			QString itemId = query1.value(0).toString();
			QString itemCategoryId = query1.value(1).toString();
			bool deleted = query1.value(7).toBool();

// 			int row = ui.tableWidget->rowCount();
// 			ui.tableWidget->insertRow(row);
// 			ui.tableWidget->setItem(row, 0, new QTableWidgetItem(query1.value(2).toString()));
// 			ui.tableWidget->setItem(row, 1, new QTableWidgetItem(query1.value(3).toString()));

			rowItems.append(query1.value(2).toString());
			rowItems.append(query1.value(3).toString());
			QSqlQuery query3("SELECT * FROM item_category WHERE itemcategory_id=" + itemCategoryId);
			if (query3.first())
			{
				//ui.tableWidget->setItem(row, 2, new QTableWidgetItem(query3.value(2).toString()));
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

// 			ui.tableWidget->setItem(row, 3, new QTableWidgetItem(qty));
// 			ui.tableWidget->setItem(row, 4, new QTableWidgetItem(minQty));
// 			ui.tableWidget->setItem(row, 5, new QTableWidgetItem(query1.value(5).toString()));
// 			ui.tableWidget->setItem(row, 6, new QTableWidgetItem(unitPrice));
// 			ui.tableWidget->setItem(row, 7, new QTableWidgetItem(query1.value(6).toString()));

			rowItems.append(qty);
			rowItems.append(minQty);
			rowItems.append(query1.value(5).toString());
			rowItems.append(unitPrice);
			rowItems.append(query1.value(6).toString());
			bool inStock = false;

			if (qty == "Not in stock" && !deleted)
			{
				inStock = true;
// 				QWidget* base = new QWidget(ui.tableWidget);
// 				QPushButton* addToStockBtn = new QPushButton("Add To Stock", base);
// 				addToStockBtn->setMaximumWidth(120);
// 				addToStockBtn->setMinimumWidth(120);
// 				QHBoxLayout *layout = new QHBoxLayout;
// 				layout->setContentsMargins(0, 0, 0, 0);
// 				layout->addWidget(addToStockBtn);
// 				layout->insertStretch(1);
// 				base->setLayout(layout);
// 				ui.tableWidget->setCellWidget(row, 8, base);
// 				base->show();
			}
// 			else if (!deleted)
// 			{
// 				QWidget* base = new QWidget(ui.tableWidget);
// 				QPushButton* updateBtn = new QPushButton("Update", base);
// 				updateBtn->setMaximumWidth(100);
// 				QPushButton* removeBtn = new QPushButton("Remove", base);
// 				removeBtn->setMaximumWidth(100);
// 				
// 
// 				connect(removeBtn, SIGNAL(clicked()), &m_buttonSignalMapper, SLOT(map()));
// 				m_buttonSignalMapper.setMapping(removeBtn, itemId);
// 				connect(&m_buttonSignalMapper, SIGNAL(mapped(QString)), this, SLOT(slotRemove(QString)));
// 
// 
// 				QHBoxLayout *layout = new QHBoxLayout;
// 				layout->setContentsMargins(0, 0, 0, 0);
// 				layout->addWidget(updateBtn);
// 				layout->addWidget(removeBtn);
// 				layout->insertStretch(2);
// 				base->setLayout(layout);
// 				ui.tableWidget->setCellWidget(row, 8, base);
// 				base->show();
// 			}

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


		connect(removeBtn, SIGNAL(clicked()), &m_buttonSignalMapper, SLOT(map()));
		m_buttonSignalMapper.setMapping(removeBtn, itemId);
		connect(&m_buttonSignalMapper, SIGNAL(mapped(QString)), this, SLOT(slotRemove(QString)));


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

