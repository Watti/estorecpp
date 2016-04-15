#include "esordercheckin.h"
#include "utility\esdbconnection.h"
#include <QMessageBox>
#include "QDateTime"
#include "utility\session.h"

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
	headerLabels.append("Purchasing Price");
	headerLabels.append("Qty");
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

				QString itemId = queryOrderItems.value("item_id").toString();
				ui.itemTableWidget->setItem(row, 0, new QTableWidgetItem(itemId));
				QTableWidgetItem* priceItm = new QTableWidgetItem(QString::number(queryOrderItems.value("purchasing_price").toDouble(), 'f', 2));
				priceItm->setTextAlignment(Qt::AlignRight);
				ui.itemTableWidget->setItem(row, 4, priceItm);
				QTableWidgetItem* qtyItem = new QTableWidgetItem(queryOrderItems.value("qty").toString());
				qtyItem->setTextAlignment(Qt::AlignRight);
				ui.itemTableWidget->setItem(row, 5, qtyItem);
				
				QSqlQuery queryItem("SELECT * FROM item WHERE deleted = 0 AND item_id = " + itemId);
				if (queryItem.next())
				{
					ui.itemTableWidget->setItem(row, 1, new QTableWidgetItem(queryItem.value("item_code").toString()));
					ui.itemTableWidget->setItem(row, 2, new QTableWidgetItem(queryItem.value("item_name").toString()));

					QSqlQuery queryCat("SELECT * FROM item_category WHERE deleted = 0 AND itemcategory_id = " + queryItem.value("itemcategory_id").toString());
					if (queryCat.next())
					{
						ui.itemTableWidget->setItem(row, 3, new QTableWidgetItem(queryCat.value("itemcategory_name").toString()));
					}
					ui.itemTableWidget->setItem(row, 6, new QTableWidgetItem(queryItem.value("unit").toString()));
				}
			}
		}
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
	QString sellingPrice = ui.sellingPrice->text();
	QString stockId;
	double currentQty = ui.quantity->text().toDouble();
	int userId = ES::Session::getInstance()->getUser()->getId();
	QString userIdStr;
	userIdStr.setNum(userId);

	QSqlQuery itemStock("SELECT * FROM stock WHERE deleted = 0 AND item_id = " + itemId);
	if (itemStock.next())
	{
		stockId = itemStock.value("stock_id").toString();
		sellingPrice = itemStock.value("selling_price").toString();
		currentQty += itemStock.value("qty").toDouble();

		QString qtyStr;
		qtyStr.setNum(currentQty);
		QSqlQuery q("UPDATE stock SET qty = " + qtyStr + " WHERE stock_id = " + stockId);
	}
	else
	{
		QString qtyStr;
		qtyStr.setNum(currentQty);
		QString q("INSERT INTO stock (item_id, qty, selling_price, user_id) VALUES (" + 
			itemId + "," + qtyStr + "," + sellingPrice + "," + userIdStr + ")");
		QSqlQuery query;
		if (query.exec(q))
		{
			stockId = query.lastInsertId().value<QString>();
		}
	}

	QString q("INSERT INTO stock_purchase_order_item (purchaseorder_id, item_id, stock_id, qty, current_qty) VALUES (" +
		m_orderId + "," + itemId + "," + stockId + "," + ui.quantity->text() + "," + ui.quantity->text() + ")");
	QSqlQuery query;
	if (!query.exec(q))
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Critical);
		mbox.setText(QString("Something goes wrong: order check-in failed"));
		mbox.exec();
	}
	else
	{
		QSqlQuery query("UPDATE purchase_order SET checked_in = 1 WHERE purchaseorder_id = " + m_orderId);

		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Information);
		mbox.setText(QString("Success"));
		mbox.exec();
	}

}

void ESOrderCheckIn::slotRemoveFromStock()
{

}

void ESOrderCheckIn::slotHoldOrder()
{

}

void ESOrderCheckIn::slotFinalizeOrder()
{

}

void ESOrderCheckIn::slotSearch()
{

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

	ui.sellingPrice->setText(idCell->text());

	idCell = ui.itemTableWidget->item(row, 5);
	if (!idCell)
		return;

	ui.quantity->setText(idCell->text());
}
