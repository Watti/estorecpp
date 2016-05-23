#include "esmanagestockitems.h"
#include "esaddstockitem.h"
#include "utility/esdbconnection.h"
#include "utility/esmainwindowholder.h"
#include <QMessageBox>
#include <QPushButton>
#include "utility/utility.h"
#include "esaddmanualstockitemswidget.h"


ESManageStockItems::ESManageStockItems(QWidget *parent /*= 0*/)
: QWidget(parent)
{
	ui.setupUi(this);
	m_updateButtonSignalMapper = new QSignalMapper(this);
	m_removeButtonSignalMapper = new QSignalMapper(this);
	m_addToStockButtonSignalMapper = new QSignalMapper(this);

	QObject::connect(m_updateButtonSignalMapper, SIGNAL(mapped(QString)), this, SLOT(slotUpdate(QString)));
	QObject::connect(m_removeButtonSignalMapper, SIGNAL(mapped(QString)), this, SLOT(slotRemove(QString)));
	QObject::connect(ui.addItemToStockBtn, SIGNAL(clicked()), this, SLOT(slotAddToStock()));

	QStringList headerLabels;
	headerLabels.append("Stock ID");
	headerLabels.append("Item Code");
	headerLabels.append("Item Name");
	headerLabels.append("Category");
	headerLabels.append("Qty");
	headerLabels.append("Min.Qty");
	headerLabels.append("Unit");
	headerLabels.append("Unit Price");
	headerLabels.append("Discount");
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
			QTableWidgetItem* qtyItem = new QTableWidgetItem(query.value("qty").toString());
			qtyItem->setTextAlignment(Qt::AlignRight);
			ui.tableWidget->setItem(row, 4, qtyItem);
			QTableWidgetItem* minQtyItem = new QTableWidgetItem(query.value("min_qty").toString());
			minQtyItem->setTextAlignment(Qt::AlignRight);
			ui.tableWidget->setItem(row, 5, minQtyItem);			
			ui.tableWidget->setItem(row, 6, new QTableWidgetItem(query.value("unit").toString()));
			QTableWidgetItem* priceItem = new QTableWidgetItem(QString::number(query.value("selling_price").toDouble(), 'f', 2));
			priceItem->setTextAlignment(Qt::AlignRight);
			ui.tableWidget->setItem(row, 7, priceItem);
			QTableWidgetItem* discountItem = new QTableWidgetItem(QString::number(query.value("discount").toDouble(), 'f', 2));
			discountItem->setTextAlignment(Qt::AlignRight);
			ui.tableWidget->setItem(row, 8, discountItem);
			ui.tableWidget->setItem(row, 9, new QTableWidgetItem(query.value("description").toString()));

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
			ui.tableWidget->setCellWidget(row, 10, base);
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
			slotSearch();
		}
	}
}

void ESManageStockItems::addItemToStock(QString itemId)
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

void ESManageStockItems::slotAddToStock()
{
	ESAddManualStockItems* addManualStockItems = new ESAddManualStockItems(this);
	ES::MainWindowHolder::instance()->getMainWindow()->setCentralWidget(addManualStockItems);
	addManualStockItems->show();
}

