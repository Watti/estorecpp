#include "esordercheckin.h"
#include "utility\esdbconnection.h"
#include <QMessageBox>
#include "QDateTime"
#include "utility\session.h"
#include <QSqlError>
#include "QString"
#include <iostream>
#include <fstream>
#include "QMessageBox"
#include "QScopedPointer"
#include "entities\tabletextwidget.h"
#include "QTableWidget"
namespace
{
	QString convertToQTYFormat(QString text, int row, int col, QTableWidget* table)
	{
		bool isValid = false;
		double qty = text.toDouble(&isValid);

		QTableWidgetItem* item = table->item(row, 0);

		if (!isValid)
		{
			qty = 0;
		}
		return QString::number(qty);

	}
}
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
	headerLabels.append("Ordered Qty");
	headerLabels.append("Received Qty");
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
	QObject::connect(ui.generateGRN, SIGNAL(clicked()), this, SLOT(slotGenerateGRN())); 
	QObject::connect(ui.itemTableWidget, SIGNAL(cellDoubleClicked(int, int)), this, SLOT(slotCellDoubleClicked(int, int)));

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
	double currentQtyInDB = -1.0;
	double remainingQtyInDB = 0.0;
	double oldPurchasingPrice = 0.0;
	
	// To check whether further check-in is possible
	QSqlQuery stockPOQuery("SELECT * FROM stock_purchase_order_item WHERE purchaseorder_id = " + m_orderId + " AND item_id = " + itemId);
	if (stockPOQuery.next())
	{
		currentQtyInDB = stockPOQuery.value("checkin_qty").toDouble();
		remainingQtyInDB = stockPOQuery.value("remaining_qty").toDouble();
		oldPurchasingPrice = stockPOQuery.value("purchasing_price").toDouble();
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

	bool updateWCost = true;

	QSqlQuery itemStock("SELECT * FROM stock WHERE deleted = 0 AND item_id = " + itemId);
	if (itemStock.next())
	{
		stockId = itemStock.value("stock_id").toString();
		stockSellingPrice = itemStock.value("selling_price").toString();
		double oldQty = itemStock.value("qty").toDouble();
		currentQty += oldQty;

		QString qtyStr;
		qtyStr.setNum(currentQty);
		QSqlQuery q("UPDATE stock SET qty = " + qtyStr + " WHERE stock_id = " + stockId);

		//double a = oldPurchasingPrice * oldQty;
		//double b = ui.purchasingPrice->text().toDouble() * ui.quantity->text().toDouble();
		//double wCost = (a + b) / (oldQty + ui.quantity->text().toDouble());
		//QSqlQuery updateWCost("UPDATE item SET w_cost = " + QString::number(wCost, 'f', 2) + " WHERE item_id = " + itemId);
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
		QSqlQuery updateWCostQuery("UPDATE item SET w_cost = " + QString::number(ui.purchasingPrice->text().toDouble(), 'f', 2) + " WHERE item_id = " + itemId);
		updateWCost = false;
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
				remaining_qty, purchasing_price, selling_price) VALUES (?, ?, ?, ?, ?, ?, ?, ?)");
		query.addBindValue(m_orderId);
		query.addBindValue(itemId);
		query.addBindValue(stockId);
		query.addBindValue(qty);
		query.addBindValue(availableQty);
		query.addBindValue(ui.quantity->text());
		query.addBindValue(ui.purchasingPrice->text());
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

	if (updateWCost)
	{
		QSqlQuery queryOrderItems("SELECT * FROM stock_purchase_order_item WHERE item_id = " + itemId);
		double costTotal = 0;
		double qtyTotal = 0;
		while (queryOrderItems.next())
		{
			double curQty = queryOrderItems.value("remaining_qty").toDouble();
			costTotal += queryOrderItems.value("purchasing_price").toDouble() * curQty;
			qtyTotal += curQty;
		}

		if (qtyTotal > 0)
		{
			double wCost = costTotal / qtyTotal;
			QSqlQuery updateWCost("UPDATE item SET w_cost = " + QString::number(wCost, 'f', 2) + " WHERE item_id = " + itemId);
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

void ESOrderCheckIn::slotGenerateGRN()
{
	QSqlQuery qPO("SELECT * FROM supplier, purchase_order WHERE purchase_order.supplier_id = supplier.supplier_id AND  purchase_order.purchaseorder_id = "+m_orderId);
	if (qPO.next())
	{
		QString supplier = qPO.value("supplier_name").toString();
		QString dateTimeStr = QDateTime::currentDateTime().toString(QLatin1String("yyyyMMdd-hhmmss"));
		QString pathToFile = ES::Session::getInstance()->getReportPath();
		std::ofstream stream;
		QString filename = pathToFile.append("\\Good Received Note-");
		filename.append(dateTimeStr).append(".csv");
		std::string s(filename.toStdString());
		stream.open(s, std::ios::out | std::ios::app);
		stream << ",,Good Received Note" << "\n\n";
		stream << "Supplier : "<<supplier.toLatin1().toStdString()<<",,, PO NO :"<<m_orderId.toLatin1().toStdString()<<"\n";
		stream << "Date Time : ," << dateTimeStr.toLatin1().toStdString() << "\n";
		stream << "Item , Price, Order Qty, Delivered Qty" << "\n";

		int itemCount = ui.itemTableWidget->rowCount();
		for (int i = 0; i < itemCount; i++)
		{
			QString itemName = ui.itemTableWidget->item(i, 2)->text();
			QString  price = ui.itemTableWidget->item(i, 4)->text();
			QString  orderedQty = ui.itemTableWidget->item(i, 5)->text();
			QString  recQty = ui.itemTableWidget->item(i, 6)->text();
			stream << itemName.toLatin1().toStdString() << "," << price.toLatin1().toStdString() << "," << orderedQty.toLatin1().toStdString() << "," << recQty.toLatin1().toStdString() << "\n";
		}
		stream.close();
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Information);
		mbox.setText(QString("Good Received Note has been saved in : ").append(filename));
		mbox.exec();
	}
}

void ESOrderCheckIn::slotCellDoubleClicked(int row, int col)
{
	if (col == 6)
	{
		QTableWidgetItem* item = ui.itemTableWidget->item(row, col);
		QString receivedQty = (item) ? item->text() : "";
		bool valid = false;
		receivedQty.toDouble(&valid);
		if (!valid)
		{
			receivedQty = "0";
		}
		TableTextWidget* textWidget = new TableTextWidget(ui.itemTableWidget, row, col, ui.itemTableWidget);
		textWidget->setText(receivedQty);
		textWidget->setTextFormatterFunc(convertToQTYFormat);
		textWidget->selectAll();
		ui.itemTableWidget->setCellWidget(row, col, textWidget);
	}
}
