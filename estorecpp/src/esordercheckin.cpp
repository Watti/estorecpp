#include "esordercheckin.h"
#include "utility\esdbconnection.h"
#include <QMessageBox>
#include "QDateTime"
#include "utility\session.h"
#include <QSqlError>

ESOrderCheckIn::ESOrderCheckIn(QString orderId, QWidget *parent /*= 0*/)
:QWidget(parent), m_orderId(orderId)
{
	ui.setupUi(this);
	ui.itemIdText->setVisible(false);

	QStringList headerLabels;
	headerLabels.append("Item ID");
	headerLabels.append("Item Code");
	headerLabels.append("Item Name");
	headerLabels.append("Category");
	headerLabels.append("Order Price");
	headerLabels.append("Qty");
	headerLabels.append("Cur. Qty");
	headerLabels.append("Unit");

	ui.itemTableWidget->setHorizontalHeaderLabels(headerLabels);
	ui.itemTableWidget->horizontalHeader()->setStretchLastSection(true);
	ui.itemTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.itemTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.itemTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.itemTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.itemTableWidget->hideColumn(0);

	QObject::connect(ui.itemTableWidget, SIGNAL(cellClicked(int, int)), this, SLOT(slotItemSelected(int, int)));
	QObject::connect(ui.addToStockBtn, SIGNAL(clicked()), this, SLOT(slotAddToStock()));
	QObject::connect(ui.finalizeOrder, SIGNAL(clicked()), this, SLOT(slotFinalizeOrder()));

	if (!ES::DbConnection::instance()->open())
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Critical);
		mbox.setText(QString("Cannot connect to the database : AddItem"));
		mbox.exec();
	}
	else
	{
		slotSearch();
	}
}

ESOrderCheckIn::~ESOrderCheckIn()
{

}

void ESOrderCheckIn::slotAddToStock()
{
	if (ui.itemIdText->text().isEmpty())
	{
		return;
	}

	QString itemId = ui.itemIdText->text();
	double currentQty = ui.quantity->text().toDouble();
	double currentQtyInDB = -1;
	double remainingQtyInDB = 0.0;
	
	// To check whether further check-in is possible
	QSqlQuery stockPOQuery("SELECT * FROM stock_purchase_order_item WHERE purchaseorder_id = " + m_orderId + " AND item_id = " + itemId);
	if (stockPOQuery.next())
	{
		currentQtyInDB = stockPOQuery.value("checkin_qty").toDouble();
		remainingQtyInDB = stockPOQuery.value("remaining_qty").toDouble();
		if (currentQty > currentQtyInDB)
		{
			QMessageBox mbox;
			mbox.setIcon(QMessageBox::Critical);
			mbox.setText(QString("Current Quantity is lower than you entered."));
			mbox.exec();
			return;
		}
	}
	
	QString stockSellingPrice = "0.0";
	QString discount = ui.discount->text();
	QString stockId;
	
	int userId = ES::Session::getInstance()->getUser()->getId();
	QString userIdStr;
	userIdStr.setNum(userId);

	QSqlQuery itemStock("SELECT * FROM stock WHERE deleted = 0 AND item_id = " + itemId);
	if (itemStock.next())
	{
		stockId = itemStock.value("stock_id").toString();
		stockSellingPrice = itemStock.value("selling_price").toString();
		currentQty += itemStock.value("qty").toDouble();

		QString qtyStr;
		qtyStr.setNum(currentQty);
		QSqlQuery q("UPDATE stock SET qty = " + qtyStr + " WHERE stock_id = " + stockId);
	}
	else
	{
		QString qtyStr;
		qtyStr.setNum(currentQty);
		QString q("INSERT INTO stock (item_id, qty, selling_price, discount, user_id) VALUES (" + 
			itemId + "," + qtyStr + "," + stockSellingPrice + "," + discount + "," + userIdStr + ")");
		QSqlQuery query;
		if (query.exec(q))
		{
			stockId = query.lastInsertId().value<QString>();
		}
	}

	QString itemSellingPrice = ui.sellingPrice->text();
	double qty = 0;
	QSqlQuery queryOrderItems("SELECT * FROM purchase_order_item WHERE deleted = 0 AND purchaseorder_id = " + m_orderId + " AND item_id = " + itemId);
	if (queryOrderItems.next())
	{
		qty = queryOrderItems.value("qty").toDouble();
	}

	if (currentQtyInDB != -1)
	{
		double curQty = ui.quantity->text().toDouble();

		QSqlQuery query;
		query.prepare("UPDATE stock_purchase_order_item SET checkin_qty = ?, remaining_qty = ?, \
				selling_price = ? WHERE purchaseorder_id = ? AND item_id = ?");
		query.addBindValue(currentQtyInDB - curQty);
		query.addBindValue(remainingQtyInDB + curQty);
		query.addBindValue(itemSellingPrice);
		query.addBindValue(m_orderId);
		query.addBindValue(itemId);

		if (!query.exec())
		{
			QString errorMsg = "Something goes wrong: ";
			errorMsg.append(query.lastError().databaseText());
			QMessageBox mbox;
			mbox.setIcon(QMessageBox::Critical);
			mbox.setText(errorMsg);
			mbox.exec();
		}
	}
	else
	{
		double availableQty = qty - ui.quantity->text().toDouble();
		
		QSqlQuery query;
		query.prepare("INSERT INTO stock_purchase_order_item (purchaseorder_id, item_id, stock_id, qty, checkin_qty, \
				remaining_qty, selling_price) VALUES (?, ?, ?, ?, ?, ?, ?)");
		query.addBindValue(m_orderId);
		query.addBindValue(itemId);
		query.addBindValue(stockId);
		query.addBindValue(qty);
		query.addBindValue(availableQty);
		query.addBindValue(ui.quantity->text());
		query.addBindValue(itemSellingPrice);

		if (!query.exec())
		{
			QString errorMsg = "Something goes wrong: ";
			errorMsg.append(query.lastError().databaseText());
			QMessageBox mbox;
			mbox.setIcon(QMessageBox::Critical);
			mbox.setText(errorMsg);
			mbox.exec();
		}
	}

	slotSearch();
}

void ESOrderCheckIn::slotRemoveFromStock()
{

}

void ESOrderCheckIn::slotHoldOrder()
{

}

void ESOrderCheckIn::slotFinalizeOrder()
{
	QSqlQuery poItemsQry("SELECT item_id FROM purchase_order_item WHERE deleted = 0 AND purchaseorder_id = " + m_orderId);
	QSqlQuery stockPoItemQry("SELECT item_id FROM stock_purchase_order_item WHERE purchaseorder_id = " + m_orderId);
	bool canCheckedIn = true;
	if (poItemsQry.size() != stockPoItemQry.size())
	{
		canCheckedIn = false;
	}
	else
	{
		while (stockPoItemQry.next())
		{
			if (stockPoItemQry.value("checkin_qty").toDouble() > 0)
			{
				canCheckedIn = false;
				break;
			}
		}
	}

	if (canCheckedIn)
	{
		QSqlQuery query("UPDATE purchase_order SET checked_in = 1 WHERE purchaseorder_id = " + m_orderId);
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Information);
		mbox.setText(QString("Successfully finalized"));
		mbox.exec();
	}
	else
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Critical);
		mbox.setText(QString("Some items are not added to the Stock yet. Cannot be finalized"));
		mbox.exec();
	}
}

void ESOrderCheckIn::slotSearch()
{
	while (ui.itemTableWidget->rowCount() > 0)
	{
		ui.itemTableWidget->removeRow(0);
	}

	QSqlQuery queryCategory("SELECT * FROM item_category WHERE deleted = 0");
	QString catCode = "select";
	int catId = -1;

	ui.categoryComboBox->addItem(catCode, catId);

	while (queryCategory.next())
	{
		catId = queryCategory.value("itemcategory_id").toInt();
		ui.categoryComboBox->addItem(queryCategory.value("itemcategory_name").toString(), catId);
	}

	// Fill order data
	QSqlQuery queryOrder("SELECT * FROM purchase_order WHERE deleted = 0 AND purchaseorder_id = " + m_orderId);
	if (queryOrder.next())
	{
		ui.lblOrderId->setText(m_orderId);
		ui.lblOrderDate->setText(queryOrder.value("order_date").toString());

		QSqlQuery querySupplier("SELECT * FROM supplier WHERE deleted = 0 AND supplier_id = " + queryOrder.value("supplier_id").toString());
		if (querySupplier.next())
		{
			ui.lblSupplierName->setText(querySupplier.value("supplier_name").toString());
		}

		ui.lblArrivedDate->setText(QDate::currentDate().toString("yyyy-MM-dd"));

		// Fill order items
		QSqlQuery queryOrderItems("SELECT * FROM purchase_order_item WHERE deleted = 0 AND purchaseorder_id = " + m_orderId);
		int row = 0;
		while (queryOrderItems.next())
		{
			row = ui.itemTableWidget->rowCount();
			ui.itemTableWidget->insertRow(row);
			QColor red(245, 169, 169);
			bool empty = false;

			QString itemId = queryOrderItems.value("item_id").toString();
			QString qty = queryOrderItems.value("qty").toString();

			QSqlQuery stockPOQuery("SELECT * FROM stock_purchase_order_item WHERE purchaseorder_id = " + m_orderId + " AND item_id = " + itemId);
			if (stockPOQuery.next())
			{
				QString currentQty = stockPOQuery.value("checkin_qty").toString();
				QTableWidgetItem* curQtyItem = new QTableWidgetItem(currentQty);
				curQtyItem->setTextAlignment(Qt::AlignRight);
				ui.itemTableWidget->setItem(row, 6, curQtyItem);
				if (currentQty.toDouble() == 0.0)
				{
					empty = true;
					curQtyItem->setBackgroundColor(red);
				}
			}
			else
			{
				QTableWidgetItem* qtyItem = new QTableWidgetItem(qty);
				qtyItem->setTextAlignment(Qt::AlignRight);
				ui.itemTableWidget->setItem(row, 6, qtyItem);
			}
			
			ui.itemTableWidget->setItem(row, 0, new QTableWidgetItem(itemId));
			QTableWidgetItem* priceItm = new QTableWidgetItem(QString::number(queryOrderItems.value("order_price").toDouble(), 'f', 2));
			priceItm->setTextAlignment(Qt::AlignRight);
			ui.itemTableWidget->setItem(row, 4, priceItm);
			if (empty) priceItm->setBackgroundColor(red);

			
			QTableWidgetItem* qtyItem = new QTableWidgetItem(qty);
			qtyItem->setTextAlignment(Qt::AlignRight);
			ui.itemTableWidget->setItem(row, 5, qtyItem);
			if (empty) qtyItem->setBackgroundColor(red);
			
			QSqlQuery queryItem("SELECT * FROM item WHERE deleted = 0 AND item_id = " + itemId);
			if (queryItem.next())
			{
				QTableWidgetItem* item1 = new QTableWidgetItem(queryItem.value("item_code").toString());
				if (empty) item1->setBackgroundColor(red);
				ui.itemTableWidget->setItem(row, 1, item1);

				QTableWidgetItem* item2 = new QTableWidgetItem(queryItem.value("item_name").toString());
				if (empty) item2->setBackgroundColor(red);
				ui.itemTableWidget->setItem(row, 2, item2);

				QSqlQuery queryCat("SELECT * FROM item_category WHERE deleted = 0 AND itemcategory_id = " + queryItem.value("itemcategory_id").toString());
				if (queryCat.next())
				{
					QTableWidgetItem* item3 = new QTableWidgetItem(queryCat.value("itemcategory_name").toString());
					if (empty) item3->setBackgroundColor(red);
					ui.itemTableWidget->setItem(row, 3, item3);
				}

				QTableWidgetItem* item4 = new QTableWidgetItem(queryItem.value("unit").toString());
				if (empty) item4->setBackgroundColor(red);
				ui.itemTableWidget->setItem(row, 7, item4);
			}
		}
	}
}

void ESOrderCheckIn::slotItemSelected(int row, int col)
{
	QTableWidgetItem* idCell = ui.itemTableWidget->item(row, 0);
	if (!idCell)
		return;

	QString itemId = idCell->text();
	QSqlQuery queryItem("SELECT * FROM item WHERE deleted = 0 AND item_id = " + itemId);
	if (queryItem.next())
	{
		ui.itemCode->setText(queryItem.value("item_code").toString());
		ui.itemIdText->setText(itemId);
	}

	idCell = ui.itemTableWidget->item(row, 4);
	if (!idCell)
		return;

	ui.purchasingPrice->setText(idCell->text());

	idCell = ui.itemTableWidget->item(row, 6);
	if (!idCell)
		return;

	ui.quantity->setText(idCell->text());

	QSqlQuery queryStockPOItem("SELECT * FROM stock_purchase_order_item WHERE item_id = " +
		itemId + " AND purchaseorder_id = " + m_orderId);

	if (queryStockPOItem.next())
	{
		double price = queryStockPOItem.value("selling_price").toDouble();
		ui.sellingPrice->setText(QString::number(price, 'f', 2));
	}
}
