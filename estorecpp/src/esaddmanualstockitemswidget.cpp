#include "esaddmanualstockitemswidget.h"
#include "utility/esmainwindowholder.h"
#include "utility/esdbconnection.h"
#include "utility/utility.h"
#include "utility/session.h"
#include <QMessageBox>
#include "QString"
#include "easylogging++.h"

ESAddManualStockItems::ESAddManualStockItems(QWidget *parent /*= 0*/)
{
	ui.setupUi(this);
	ui.itemIdText->setVisible(false);

	QStringList headerLabels;
	headerLabels.append("Item ID");
	headerLabels.append("Item Code");
	headerLabels.append("Item Name");
	headerLabels.append("Category");
	headerLabels.append("Unit");
	headerLabels.append("Description");

	ui.tableWidget->setHorizontalHeaderLabels(headerLabels);
	ui.tableWidget->horizontalHeader()->setStretchLastSection(true);
	ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.tableWidget->hideColumn(0);
	ui.tableWidget->hideColumn(5);

	QObject::connect(ui.tableWidget, SIGNAL(cellClicked(int, int)), this, SLOT(slotItemSelected(int, int)));
	QObject::connect(ui.addToStockBtn, SIGNAL(clicked()), this, SLOT(slotAddToStock()));
	QObject::connect(ui.searchTextBox, SIGNAL(textChanged(QString)), this, SLOT(slotSearch()));
	QObject::connect(ui.categoryComboBox, SIGNAL(activated(QString)), this, SLOT(slotSearch()));
	if (ES::Session::getInstance()->isSecondDisplayOn())
	{
		ui.floorText->hide();
		ui.label_10->hide();
	}
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
		QString catCode = "select";
		int catId = -1;

		ui.categoryComboBox->addItem(catCode, catId);

		while (queryCategory.next())
		{
			catId = queryCategory.value("itemcategory_id").toInt();
			ui.categoryComboBox->addItem(queryCategory.value("itemcategory_name").toString(), catId);
		}

		slotSearch();
	}
}

ESAddManualStockItems::~ESAddManualStockItems()
{

}

void ESAddManualStockItems::slotSearch()
{
	while (ui.tableWidget->rowCount() > 0)
	{
		ui.tableWidget->removeRow(0);
	}

	QString text = ui.searchTextBox->text();
	QString selectedCategory = ui.categoryComboBox->currentText();

	QString q;
	q.append("SELECT item.item_id, item.item_code, item.item_name, item_category.itemcategory_name, item.unit, item.description FROM item JOIN item_category ON item.itemcategory_id = item_category.itemcategory_id WHERE item.deleted = 0 ");

	if (!text.isEmpty())
	{
		q.append("AND (item_category.itemcategory_code LIKE '%");
		q.append(text);
		// 		q.append("%' OR item.item_name LIKE '%");
		// 		q.append(text);
		// 		q.append("%' OR item.description LIKE '%");
		// 		q.append(text);
		q.append("%') ");
	}
	if (!selectedCategory.isEmpty() && selectedCategory != DEFAULT_DB_COMBO_VALUE)
	{
		q.append("AND item_category.itemcategory_name = '");
		q.append(selectedCategory);
		q.append("' ");
	}
	if (text.isEmpty())
	{
		q.append(" LIMIT 25");
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
		ui.tableWidget->setSortingEnabled(false);
		while (query.next())
		{
			int row = ui.tableWidget->rowCount();
			ui.tableWidget->insertRow(row);

			QString itemId = query.value("item_id").toString();

			ui.tableWidget->setItem(row, 0, new QTableWidgetItem(itemId));
			ui.tableWidget->setItem(row, 1, new QTableWidgetItem(query.value("item_code").toString()));
			ui.tableWidget->setItem(row, 2, new QTableWidgetItem(query.value("item_name").toString()));
			ui.tableWidget->setItem(row, 3, new QTableWidgetItem(query.value("itemcategory_name").toString()));
			ui.tableWidget->setItem(row, 4, new QTableWidgetItem(query.value("unit").toString()));
			ui.tableWidget->setItem(row, 5, new QTableWidgetItem(query.value("description").toString()));
		}
		ui.tableWidget->setSortingEnabled(true);
	}

}

void ESAddManualStockItems::slotItemSelected(int row, int col)
{
	ui.minQty->setText("0");
	ui.discount->setText("0");
	ui.sellingPrice->setText("0");
	ui.qty->setText("0");
	ui.purchasingPrice->setText("0");

	QTableWidgetItem* idCell = ui.tableWidget->item(row, 0);
	if (!idCell)
		return;

	QString itemId = idCell->text();
	QSqlQuery queryItem("SELECT * FROM item WHERE deleted = 0 AND item_id = " + itemId);
	if (queryItem.next())
	{
		ui.itemCode->setText(queryItem.value("item_code").toString());
	}

	//QSqlQuery queryStock("SELECT * FROM stock WHERE deleted = 0 AND item_id = " + itemId);
	QSqlQuery queryStock("SELECT * FROM stock WHERE item_id = " + itemId);
	QString stockId = "";
	bool success = false;
	if (queryStock.next())
	{
		success = true;
		ui.minQty->setText(queryStock.value("min_qty").toString());
		ui.discount->setText(queryStock.value("discount").toString());
		ui.sellingPrice->setText(queryStock.value("selling_price").toString());
		ui.currentQty->setText(queryStock.value("qty").toString());
		stockId = queryStock.value("stock_id").toString();
	}
	if (success)
	{
		QSqlQuery qStockPO("SELECT purchasing_price FROM stock_purchase_order_item WHERE purchaseorder_id = -1 AND item_id = " + itemId + " AND stock_id = " + stockId);
		if (qStockPO.next())
		{
			ui.purchasingPrice->setText(qStockPO.value("purchasing_price").toString());
		}
	}
	ui.itemIdText->setText(itemId);
}

void ESAddManualStockItems::slotAddToStock()
{
	if (ui.itemIdText->text().isEmpty())
	{
		return;
	}
	QString floorNo = ui.floorText->text();
	QString itemId = ui.itemIdText->text();
	QString newSellingPrice = ui.sellingPrice->text();
	if (newSellingPrice == nullptr || newSellingPrice.isEmpty())
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Warning);
		mbox.setText(QString("Selling price cannot be empty"));
		mbox.exec();
		return;
	}
	bool isValid = false;
	double sellingPrice = newSellingPrice.toDouble(&isValid);
	if (!isValid)
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Warning);
		mbox.setText(QString("Invalid input - Selling Price"));
		mbox.exec();
	}
	QString discount = ui.discount->text();
	if (discount == nullptr || discount.isEmpty())
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Warning);
		mbox.setText(QString("Selling price cannot be empty"));
		mbox.exec();
		return;
	}

	isValid = false;
	discount.toDouble(&isValid);
	if (!isValid)
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Warning);
		mbox.setText(QString("Invalid input - Discount"));
		mbox.exec();
	}
	if (ui.qty->text() == nullptr || ui.qty->text().isEmpty())
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Warning);
		mbox.setText(QString("Quantity cannot be empty"));
		mbox.exec();
		return;
	}
	isValid = false;
	double newlyAddedQty = ui.qty->text().toDouble(&isValid);
	if (!isValid)
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Warning);
		mbox.setText(QString("Invalid input - Quantity"));
		mbox.exec();
	}
	if (ui.minQty->text() == nullptr || ui.minQty->text().isEmpty())
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Warning);
		mbox.setText(QString("Min quantity cannot be empty"));
		mbox.exec();
		return;
	}
	isValid = false;
	ui.minQty->text().toDouble(&isValid);
	if (!isValid)
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Warning);
		mbox.setText(QString("Invalid input - Min Quantity"));
		mbox.exec();
	}
	QString purchasingPriceStr = ui.purchasingPrice->text();

	if (purchasingPriceStr == nullptr || purchasingPriceStr.isEmpty())
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Warning);
		mbox.setText(QString("Purchasing price cannot be empty"));
		mbox.exec();
		return;
	}
	isValid = false;
	purchasingPriceStr.toDouble(&isValid);
	if (!isValid)
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Warning);
		mbox.setText(QString("Invalid input - Purchasing Price"));
		mbox.exec();
	}
	double costOfItem = purchasingPriceStr.toDouble();
	if (sellingPrice < costOfItem)
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Warning);
		mbox.setText(QString("Invalid input - Purchasing price cannot be higher than the selling price"));
		mbox.exec();
		return;
	}
	if (!floorNo.isEmpty())
	{
		isValid = false;
		int floorNoInt = floorNo.toInt(&isValid);
		if (!isValid)
		{
			QMessageBox mbox;
			mbox.setIcon(QMessageBox::Warning);
			mbox.setText(QString("Invalid input - Floor Number"));
			mbox.exec();
			return;
		}
	}
	else
	{
		floorNo = "0";
	}

	int userId = ES::Session::getInstance()->getUser()->getId();
	QString userIdStr;
	userIdStr.setNum(userId);
	bool success = true;
	QSqlQuery itemStock("SELECT * FROM stock WHERE item_id = " + itemId);

	QString stockId = "-1";
	if (itemStock.next())
	{
		//item is already available in the stock
		stockId = itemStock.value("stock_id").toString();
		QString currentSellingPrice = itemStock.value("selling_price").toString();
		float currQty = itemStock.value("qty").toFloat();

		//currentQty += itemStock.value("qty").toDouble();
		double currentQty = newlyAddedQty + ui.currentQty->text().toDouble();
		QString itemNameStr = "";
		if (ui.isWeightedAverage->isChecked())
		{
			QSqlQuery queryGetWCost("SELECT w_cost, item_name FROM item WHERE item_id = " + itemId);

			if (queryGetWCost.first())
			{
				double oldWCost = queryGetWCost.value("w_cost").toDouble();
				itemNameStr = queryGetWCost.value("item_name").toString();
				if (oldWCost < 0)
				{
					QSqlQuery updateWCost("UPDATE item SET w_cost = " + purchasingPriceStr + " WHERE item_id = " + itemId);
				}
				else
				{
					if (currQty > 0)
					{
						double a = oldWCost * currQty;
						double b = purchasingPriceStr.toDouble() * newlyAddedQty;
						double wCost = (a + b) / (currQty + newlyAddedQty);
						QSqlQuery updateWCost("UPDATE item SET w_cost = " + QString::number(wCost, 'f', 2) + " WHERE item_id = " + itemId);
					}
				}
			}
		}
		else
		{
			QSqlQuery updateWCost("UPDATE item SET w_cost = " + purchasingPriceStr + " WHERE item_id = " + itemId);
		}
		QString qtyStr;
		qtyStr.setNum(currentQty);
		QString q("UPDATE stock SET  qty = " + qtyStr + " , selling_price = " + newSellingPrice + " , discount = "
			+ discount + " , min_qty =  " + ui.minQty->text() + " , floor = "+floorNo+" WHERE stock_id = " + stockId);
		QSqlQuery query;
		if (!query.exec(q))
		{
			success = false;
			QMessageBox mbox;
			mbox.setIcon(QMessageBox::Critical);
			mbox.setText(QString("Something goes wrong: stock update failed"));
			mbox.exec();

			QString logError("[ESAddManualStockItems::slotAddToStock] Stock update has been failed query = ");
			logError.append(q);
			LOG(ERROR) << logError.toLatin1().data();
		}
		if (success)
		{
			QString qStockAuditStr("INSERT INTO stock_audit (userId, stockId, newQty, prevQty, itemId) VALUES ('");
			qStockAuditStr.append(userIdStr).append("', '").append(stockId).append("','").append(QString::number(newlyAddedQty)).append
				("','").append(ui.currentQty->text()).append("','").append(itemId).append("')");
			QSqlQuery qAuditQuery(qStockAuditStr);

			QString logError("[ManualStockUpdate] [Stock updated item = ");
			logError.append(itemNameStr).append(" By Adding = ");
			logError.append(QString::number(newlyAddedQty));
			ES::Session::getInstance()->getUser()->getName();
			logError.append(", User = ").append(ES::Session::getInstance()->getUser()->getName());
			LOG(INFO) << logError.toLatin1().data();
			QSqlQuery qSelectPO("SELECT * FROM stock_purchase_order_item WHERE  purchaseorder_id = -1 AND stock_id = " + stockId + " AND item_id = " + itemId);
			if (qSelectPO.next())
			{
				QString qUpdateStockPOStr("UPDATE stock_purchase_order_item SET  purchasing_price = " + purchasingPriceStr +
					" WHERE purchaseorder_id = -1 AND stock_id = " + stockId + " AND item_id = " + itemId);
				QSqlQuery updateStockPOQuery;
				if (!updateStockPOQuery.exec(qUpdateStockPOStr))
				{
					success = false;
					QMessageBox mbox;
					mbox.setIcon(QMessageBox::Critical);
					mbox.setText(QString("Something goes wrong: update purchasing price failed"));
					mbox.exec();

					QString logError("[ESAddManualStockItems::slotAddToStock] stock_purchase_order_item update has been failed query = ");
					logError.append(q);
					LOG(ERROR) << logError.toLatin1().data();
				}
			}
			else
			{
				QString qStockPOStr("INSERT INTO stock_purchase_order_item (purchaseorder_id, item_id, selling_price, purchasing_price, stock_id) VALUES (-1, " +
					itemId + ", " + newSellingPrice + ", " + purchasingPriceStr + ", "+ stockId + ")");
				QSqlQuery queryStockPO;
				if (queryStockPO.exec(qStockPOStr))
				{
					QMessageBox mbox;
					mbox.setIcon(QMessageBox::Information);
					mbox.setText(QString("Update is success"));
					mbox.exec();
					ui.minQty->setText("");
					ui.discount->setText("");
					ui.sellingPrice->setText("");
					ui.qty->setText("");
					ui.itemCode->setText("");
					ui.purchasingPrice->setText("");
				}
				else
				{
					QMessageBox mbox;
					mbox.setIcon(QMessageBox::Warning);
					mbox.setText(QString("Purchasing price update error!"));
					mbox.exec();
				}
			}
		}
		if (success)
		{
			QMessageBox mbox;
			mbox.setIcon(QMessageBox::Information);
			mbox.setText(QString("Operation is success"));
			mbox.exec();
		}
	}
	else
	{
		if (ui.isWeightedAverage->isChecked())
		{
			QSqlQuery queryGetWCost("SELECT w_cost FROM item WHERE item_id = " + itemId);
			if (queryGetWCost.first())
			{
				double oldWCost = queryGetWCost.value("w_cost").toDouble();
				if (oldWCost < 0)
				{
					QSqlQuery updateWCost("UPDATE item SET w_cost = " + purchasingPriceStr + " WHERE item_id = " + itemId);
				}
				else
				{
					if (newlyAddedQty > 0)
					{
						double wCost = purchasingPriceStr.toDouble();
						QSqlQuery updateWCost("UPDATE item SET w_cost = " + QString::number(wCost, 'f', 2) + " WHERE item_id = " + itemId);
					}
				}
			}
		}
		else
		{
			double wCost = purchasingPriceStr.toDouble();
			QSqlQuery updateWCost("UPDATE item SET w_cost = " + QString::number(wCost, 'f', 2) + " WHERE item_id = " + itemId);
		}
		QString qtyStr;
		qtyStr.setNum(newlyAddedQty);
		QString q("INSERT INTO stock (item_id, qty, min_qty, selling_price, discount, floor, user_id) VALUES (" +
			itemId + "," + qtyStr + "," + ui.minQty->text() + "," + newSellingPrice + "," + discount + "," + floorNo + ", " + userIdStr + ")");
		QSqlQuery query;
		if (!query.exec(q))
		{
			success = false;
			QMessageBox mbox;
			mbox.setIcon(QMessageBox::Critical);
			mbox.setText(QString("Something goes wrong: stock update failed"));
			mbox.exec();
			QString logError("[ESAddManualStockItems::slotAddToStock] Stock insert has been failed query =");
			logError.append(q);

			LOG(ERROR) << logError.toLatin1().data();
		}

		stockId = query.lastInsertId().toString();
		if (success)
		{
			QString qStockPOStr("INSERT INTO stock_purchase_order_item (purchaseorder_id, item_id, selling_price, purchasing_price, stock_id) VALUES (-1, " +
				itemId + ", " + newSellingPrice + ", " + purchasingPriceStr + ", " + stockId + ")");
			QSqlQuery queryStockPO;
			if (queryStockPO.exec(qStockPOStr))
			{
				QMessageBox mbox;
				mbox.setIcon(QMessageBox::Information);
				mbox.setText(QString("Item has been added to the stock"));
				mbox.exec();
				ui.minQty->setText("");
				ui.discount->setText("");
				ui.sellingPrice->setText("");
				ui.qty->setText("");
				ui.itemCode->setText("");
				ui.purchasingPrice->setText("");
			}
			else
			{
				QMessageBox mbox;
				mbox.setIcon(QMessageBox::Warning);
				mbox.setText(QString("Purchasing price update error!"));
				mbox.exec();
			}
		}
		if (success)
		{

			QMessageBox mbox;
			mbox.setIcon(QMessageBox::Information);
			mbox.setText(QString("Operation is success"));
			mbox.exec();
		}
	}

}
