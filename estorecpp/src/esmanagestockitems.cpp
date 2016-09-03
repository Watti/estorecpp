#include "esmanagestockitems.h"
#include "esaddstockitem.h"
#include "utility/esdbconnection.h"
#include "utility/esmainwindowholder.h"
#include <QMessageBox>
#include <QPushButton>
#include "utility/utility.h"
#include "esaddmanualstockitemswidget.h"
#include "esmainwindow.h"
#include "utility/session.h"

ESManageStockItems::ESManageStockItems(QWidget *parent /*= 0*/)
: QWidget(parent), m_startingLimit(0), m_pageOffset(15), m_nextCounter(0), m_maxNextCount(0)
{
	ui.setupUi(this);
	m_updateButtonSignalMapper = new QSignalMapper(this);
	m_removeButtonSignalMapper = new QSignalMapper(this);
	m_addToStockButtonSignalMapper = new QSignalMapper(this);

	QObject::connect(m_updateButtonSignalMapper, SIGNAL(mapped(QString)), this, SLOT(slotUpdate(QString)));
	QObject::connect(m_removeButtonSignalMapper, SIGNAL(mapped(QString)), this, SLOT(slotRemove(QString)));
	QObject::connect(ui.addItemToStockBtn, SIGNAL(clicked()), this, SLOT(slotAddToStock()));
	QObject::connect(ui.nextBtn, SIGNAL(clicked()), this, SLOT(slotNext()));
	QObject::connect(ui.prevBtn, SIGNAL(clicked()), this, SLOT(slotPrev()));

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
	ui.tableWidget->hideColumn(9);
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

	QString q, qRecordCountStr;
	

	if (ES::Session::getInstance()->getUser()->getType() == ES::User::SENIOR_MANAGER ||
		ES::Session::getInstance()->getUser()->getType() == ES::User::DEV)
	{
		q.append("SELECT * FROM stock JOIN item ON stock.item_id = item.item_id ");
		q.append("JOIN item_category ON item.itemcategory_id = item_category.itemcategory_id ");
		q.append("WHERE item.deleted = 0");

		qRecordCountStr.append("SELECT COUNT(*) as c FROM stock JOIN item ON stock.item_id = item.item_id ");
		qRecordCountStr.append("JOIN item_category ON item.itemcategory_id = item_category.itemcategory_id ");
		qRecordCountStr.append("WHERE item.deleted = 0 ");
	}
	else
	{
		//hiding the invisible stock items
		q.append("SELECT * FROM stock JOIN item ON stock.item_id = item.item_id ");
		q.append("JOIN item_category ON item.itemcategory_id = item_category.itemcategory_id ");
		q.append("WHERE item.deleted = 0 AND stock.visible = 1");

		qRecordCountStr.append("SELECT COUNT(*) as c FROM stock JOIN item ON stock.item_id = item.item_id ");
		qRecordCountStr.append("JOIN item_category ON item.itemcategory_id = item_category.itemcategory_id ");
		qRecordCountStr.append("WHERE item.deleted = 0 AND stock.visible = 1");
	}

	//qRecordCountStr.append("WHERE item.deleted = 0 AND stock.deleted = 0");


	if (!text.isEmpty())
	{
		q.append(" AND (item.item_code LIKE '%");
		q.append(text);
		q.append("%' OR item.item_name LIKE '%");
		q.append(text);
		q.append("%' OR item.description LIKE '%");
		q.append(text);
		q.append("%') ");

		qRecordCountStr.append(" AND (item.item_code LIKE '%");
		qRecordCountStr.append(text);
		qRecordCountStr.append("%' OR item.item_name LIKE '%");
		qRecordCountStr.append(text);
		qRecordCountStr.append("%' OR item.description LIKE '%");
		qRecordCountStr.append(text);
		qRecordCountStr.append("%') ");

	}
	if (!selectedCategory.isEmpty() && selectedCategory != DEFAULT_DB_COMBO_VALUE)
	{
		q.append(" AND item_category.itemcategory_name = '");
		q.append(selectedCategory);
		q.append("' ");

		qRecordCountStr.append(" AND item_category.itemcategory_name = '");
		qRecordCountStr.append(selectedCategory);
		qRecordCountStr.append("' ");
	}

	QSqlQuery queryRecordCount(qRecordCountStr);
	if (queryRecordCount.next())
	{
		m_totalRecords = queryRecordCount.value("c").toInt();
	}
	//pagination start
	q.append(" LIMIT ").append(QString::number(m_startingLimit));
	q.append(" , ").append(QString::number(m_pageOffset));
	//pagination end

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

		//pagination start
		m_maxNextCount = m_totalRecords / m_pageOffset;
		if (m_maxNextCount > m_nextCounter)
		{
			ui.nextBtn->setEnabled(true);
		}
		int currentlyShowdItemCount = (m_nextCounter +1 )*m_pageOffset;
		if (currentlyShowdItemCount>=m_totalRecords)
		{
			ui.nextBtn->setDisabled(true);
		}
		//pagination end

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

			QPushButton* updateBtn = new QPushButton(base);
			updateBtn->setIcon(QIcon("icons/update.png"));
			updateBtn->setIconSize(QSize(24, 24));
			updateBtn->setMaximumWidth(100);
			
			QPushButton* removeBtn = new QPushButton(base);
			removeBtn->setIcon(QIcon("icons/delete.png"));
			removeBtn->setIconSize(QSize(24, 24));
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

void ESManageStockItems::slotUpdate(QString stockId)
{
	AddStockItem* addStockItem = new AddStockItem(this);
	addStockItem->getUI().groupBox->setTitle("Update Stock Item");
	addStockItem->setItemId(stockId);
	addStockItem->getUI().addItemButton->setText("Update");

	if (ES::Session::getInstance()->getUser()->getType() == ES::User::SENIOR_MANAGER ||
		ES::Session::getInstance()->getUser()->getType() == ES::User::DEV)
	{
		addStockItem->getUI().invisibleCB->show();
		addStockItem->getUI().inVisibleLbl->show();
	}
	else
	{
		addStockItem->getUI().invisibleCB->hide();
		addStockItem->getUI().inVisibleLbl->hide();
	}

	QSqlQuery query("SELECT * FROM stock WHERE stock_id = " + stockId);
	QString itemId = "";
	while (query.next())
	{
		QString price = query.value("selling_price").toString();
		itemId = query.value("item_id").toString();
		int visible = query.value("visible").toInt();
		if (visible == 0)
		{
			addStockItem->getUI().invisibleCB->setChecked(true);
		}
		QString discount = query.value("discount").toString();
		addStockItem->getUI().itemIDLabel->setText(itemId);
		addStockItem->getUI().itemPrice->setText(price);
		addStockItem->getUI().discount->setText(discount);


		QString quantity = query.value("qty").toString();
		addStockItem->getUI().qty->setText(quantity);
		bool isValid = false;
		double qtyDouble = quantity.toDouble(&isValid);
		if (isValid)
		{
			addStockItem->setExistingQuantityInMainStock(qtyDouble);
		}
		QString minqty = query.value("min_qty").toString(), qty = query.value("qty").toString();
		addStockItem->getUI().minQty->setText(minqty);
		addStockItem->getUI().qty->setText(qty);
		addStockItem->setExistingQuantityInStock(qty.toDouble());
		QSqlQuery queryPO("SELECT purchasing_price FROM stock_purchase_order_item WHERE stock_id = " + stockId + "AND item_id =" + itemId);
		if (queryPO.next())
		{
			QString purchasedPrice = queryPO.value("purchasing_price").toString();
			addStockItem->getUI().purchasingPrice->setText(purchasedPrice);
		}
		else
		{
			addStockItem->getUI().purchasingPrice->setText("N/A");
		}
	}

	QString qStockPO("SELECT purchasing_price FROM stock_purchase_order_item WHERE purchaseorder_id = -1 AND stock_id = " +
		stockId + " AND item_id = " + itemId);
	QSqlQuery selectStockPOQuery;
	if (selectStockPOQuery.exec(qStockPO) && selectStockPOQuery.next())
	{
		addStockItem->getUI().purchasingPrice->setText(selectStockPOQuery.value("purchasing_price").toString());
	}
	addStockItem->setUpdate(true);
	ES::MainWindowHolder::instance()->getMainWindow()->setCentralWidget(addStockItem);
	addStockItem->show();
}

void ESManageStockItems::slotRemove(QString stockId)
{
	if (ES::Utility::verifyUsingMessageBox(this, "Progex", "Do you really want to remove this?"))
	{
		//QString str("UPDATE stock SET deleted = 1 WHERE stock_id = " + stockId);
		QString str("DELETE FROM stock WHERE stock_id = " + stockId);
		QSqlQuery q;
		if (q.exec(str))
		{
			slotSearch();
		}
	}
}

// void ESManageStockItems::addItemToStock(QString itemId)
// {
// 	AddStockItem* addStockItem = new AddStockItem(this);
// 	addStockItem->getUI().groupBox->setTitle("Add Stock Item");
// 	addStockItem->getUI().itemIDLabel->setText(itemId);
// 	addStockItem->setItemId(itemId);
// 	QSqlQuery query("SELECT selling_price, quantity FROM stock_order WHERE item_id = " + itemId);
// 	while (query.next())
// 	{
// 		QString price = query.value("selling_price").toString();
// 		addStockItem->getUI().itemPrice->setText(price);
// 		QString quantity = query.value("quantity").toString();
// 		addStockItem->getUI().qty->setText(quantity);
// 		bool isValid = false;
// 		double qtyDouble = quantity.toDouble(&isValid);
// 		if (isValid)
// 		{
// 			addStockItem->setExistingQuantityInMainStock(qtyDouble);
// 		}
// 	}
// 	QSqlQuery queryStock("SELECT * FROM stock where item_id = " + itemId);
// 	while (queryStock.next())
// 	{
// 		QString minqty = queryStock.value("min_qty").toString(), qty = queryStock.value("qty").toString();
// 		addStockItem->getUI().minQty->setText(minqty);
// 		addStockItem->getUI().qty->setText(qty);
// 		addStockItem->setExistingQuantityInStock(qty.toDouble());
// 	}
// 	ES::MainWindowHolder::instance()->getMainWindow()->setCentralWidget(addStockItem);
// 	addStockItem->show();
// }

void ESManageStockItems::slotAddToStock()
{
	ESAddManualStockItems* addManualStockItems = new ESAddManualStockItems(this);
	ES::MainWindowHolder::instance()->getMainWindow()->setCentralWidget(addManualStockItems);
	addManualStockItems->show();
}

void ESManageStockItems::slotNext()
{
	if (m_nextCounter < m_maxNextCount)
	{
		m_nextCounter++;
		ui.prevBtn->setEnabled(true);
		m_startingLimit += m_pageOffset;
	}
	slotSearch();
}

void ESManageStockItems::slotPrev()
{
	if (m_nextCounter == 1)
	{
		ui.prevBtn->setDisabled(true);
	}
	if (m_nextCounter > 0)
	{
		m_nextCounter--;
		m_startingLimit -= m_pageOffset;
		ui.nextBtn->setEnabled(true);
	}
	slotSearch();
}

