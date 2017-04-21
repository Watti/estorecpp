#include "esaddstockitem.h"
#include "utility/esdbconnection.h"
#include <QMessageBox>
#include <QPushButton>
#include "esmanagestockitems.h"
#include "utility/esmainwindowholder.h"
#include "esmainwindow.h"
#include "utility/utility.h"
#include "utility/session.h"
#include "easylogging++.h"

AddStockItem::AddStockItem(QWidget *parent /*= 0*/)
: QWidget(parent), m_existingQuantityInMainStock(0), m_update(false), m_existingQuantityInStock(0)
{
	ui.setupUi(this);
	QObject::connect(ui.addItemButton, SIGNAL(clicked()), this, SLOT(slotAddStockItem()));

	if (!ES::DbConnection::instance()->open())
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Critical);
		mbox.setText(QString("Database Connection Error : AddStockItem"));
		mbox.exec();
	}
	if (ES::Session::getInstance()->isSecondDisplayOn())
	{
		ui.floorTxt->hide();
		ui.floorLbl->hide();
	}
}

AddStockItem::~AddStockItem()
{

}

void AddStockItem::slotAddStockItem()
{
	if (m_existingQuantityInMainStock >= 0)
	{
		QString qtyStr = ui.qty->text();
		QString minQtyStr = ui.minQty->text();
		QString priceStr = ui.itemPrice->text();
		QString purchasingPrice = ui.purchasingPrice->text();
		QString floorNo = ui.floorTxt->text();
		double newQty = 0;
		if (!qtyStr.isEmpty() && !minQtyStr.isEmpty() && !priceStr.isEmpty() && !purchasingPrice.isEmpty())
		{
			bool isValid = false;
			double quantity = qtyStr.toDouble(&isValid);
			if (!isValid)
			{
				QMessageBox mbox;
				mbox.setIcon(QMessageBox::Warning);
				mbox.setText(QString("Invalid input - Quantity"));
				mbox.exec();
				return;
			}
			double tempNewQty = m_existingQuantityInMainStock + quantity;
			if (tempNewQty < 0)
			{
				QMessageBox mbox;
				mbox.setIcon(QMessageBox::Warning);
				mbox.setText(QString("Invalid - Quantity"));
				mbox.exec();
				return;
			}
			double minQty = qtyStr.toDouble(&isValid);
			if (!isValid)
			{
				QMessageBox mbox;
				mbox.setIcon(QMessageBox::Warning);
				mbox.setText(QString("Invalid input - Minimum Quantity"));
				mbox.exec();
				return;
			}
			QString itemId = ui.itemIDLabel->text();
			QString  price = ui.itemPrice->text();
			QString discount = ui.discount->text();
			price.toDouble(&isValid);
			if (!isValid)
			{
				QMessageBox mbox;
				mbox.setIcon(QMessageBox::Warning);
				mbox.setText(QString("Invalid input - Price"));
				mbox.exec();
				return;
			}
			isValid = false;
			purchasingPrice.toDouble(&isValid);
			if (!isValid)
			{
				QMessageBox mbox;
				mbox.setIcon(QMessageBox::Warning);
				mbox.setText(QString("Invalid input - Purchasing price"));
				mbox.exec();
				return;
			}
			if (purchasingPrice > price)
			{
				QMessageBox mbox;
				mbox.setIcon(QMessageBox::Warning);
				mbox.setText(QString("Invalid input - Purchasing price cannot be higher than the selling price"));
				mbox.exec();
				return;
			}
			discount.toDouble(&isValid);
			if (!isValid)
			{
				QMessageBox mbox;
				mbox.setIcon(QMessageBox::Warning);
				mbox.setText(QString("Invalid input - Discount"));
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
			if (isValid)
			{
				QString itemNameStr = "";
				QString q;
				if (isUpdate())
				{
					QString visible("1");
					if (ui.invisibleCB->isChecked())
					{
						visible = "0";
					}
					QSqlQuery queryStock("SELECT qty FROM stock WHERE stock_id = " + m_stockId);
					double currentStockQty = 0.0;

					if (queryStock.first())
					{
						currentStockQty = queryStock.value("qty").toDouble();
						newQty = currentStockQty + quantity;
						q = "UPDATE stock SET qty = '" + QString::number(newQty) + "', min_qty = '" + minQtyStr + "' ,selling_price = '" + price + "', discount ='" + discount + "', visible = '" + visible + "', floor = '" + floorNo + "' WHERE stock_id = " + m_stockId;
					}
					QSqlQuery queryGetWCost("SELECT w_cost, item_name FROM item WHERE item_id = " + itemId);
					if (queryGetWCost.first())
					{
						itemNameStr = queryGetWCost.value("item_name").toString();
						double oldWCost = queryGetWCost.value("w_cost").toDouble();
						if (oldWCost < 0)
						{
							QSqlQuery updateWCost("UPDATE item SET w_cost = " + purchasingPrice + " WHERE item_id = " + itemId);
						}
						else
						{
							if (quantity > 0)
							{
								double a = oldWCost * currentStockQty;
								double b = purchasingPrice.toDouble() * quantity;
								double wCost = (a + b) / (currentStockQty + quantity);
								QSqlQuery updateWCost("UPDATE item SET w_cost = " + QString::number(wCost, 'f', 2) + " WHERE item_id = " + itemId);
							}
						}
					}
				}

				QSqlQuery query;
				if (query.exec(q))
				{
					double remainingQtyInMainStock = -1;
					if (isUpdate())
					{
						remainingQtyInMainStock = newQty;
					}
					else
					{
						remainingQtyInMainStock = newQty;
					}
					QString updateOrder("UPDATE stock SET qty = " + QString::number(remainingQtyInMainStock) + " WHERE stock_id = " + m_stockId);
					QSqlQuery qUpdateOrder;
					if (!qUpdateOrder.exec(updateOrder))
					{
						QMessageBox mbox;
						mbox.setIcon(QMessageBox::Critical);
						mbox.setText(QString("insertion error :: cannot reduce this quantity from the main stock order"));
						mbox.exec();
					}
					int userId = ES::Session::getInstance()->getUser()->getId();
					QString userIdStr;
					userIdStr.setNum(userId);
					QString qStockAuditStr("INSERT INTO stock_audit (userId, stockId, newQty, prevQty, itemId) VALUES ('");
					qStockAuditStr.append(userIdStr).append("(', '").append(m_stockId).append("','").append(qtyStr).append
						("','").append(ui.currentQty->text()).append("','").append(itemId).append("')");
					QSqlQuery qAuditQuery(qStockAuditStr);

					QString logError("[ManualStockUpdate] [Stock updated item = ");
					logError.append(itemNameStr).append(" By Adding = ");
					logError.append(qtyStr);
					ES::Session::getInstance()->getUser()->getName();
					logError.append(", User = ").append(ES::Session::getInstance()->getUser()->getName());
					LOG(INFO) << logError.toLatin1().data();

					QSqlQuery qSelectPO("SELECT * FROM stock_purchase_order_item WHERE  purchaseorder_id = -1 AND stock_id = " + m_stockId + " AND item_id = " + itemId);
					if (qSelectPO.next())
					{
						QString qUpdateStockPOStr("UPDATE stock_purchase_order_item SET  purchasing_price = " + purchasingPrice +
							" WHERE purchaseorder_id = -1 AND stock_id = " + m_stockId + " AND item_id = " + itemId);
						QSqlQuery updateStockPOQuery;
						if (!updateStockPOQuery.exec(qUpdateStockPOStr))
						{
							QMessageBox mbox;
							mbox.setIcon(QMessageBox::Critical);
							mbox.setText(QString("Purchasing price update error !"));
							mbox.exec();
						}
					}
					else
					{
						//no entry is available it has to be added to the database
						QString qStockPOStr("INSERT INTO stock_purchase_order_item (purchaseorder_id, item_id, selling_price, purchasing_price, stock_id) VALUES (-1, " +
							itemId + ", " + price + ", " + purchasingPrice + ", " + m_stockId + ")");
						QSqlQuery queryStockPO;
						if (!queryStockPO.exec(qStockPOStr))
						{
							QMessageBox mbox;
							mbox.setIcon(QMessageBox::Critical);
							mbox.setText(QString("purchasing price insertion error :: slotAddStockItem"));
							mbox.exec();
						}
					}

					ESManageStockItems* manageStock = new ESManageStockItems();
					ES::MainWindowHolder::instance()->getMainWindow()->setCentralWidget(manageStock);
					this->close();
					manageStock->show();
				}
				else
				{
					QMessageBox mbox;
					mbox.setIcon(QMessageBox::Critical);
					mbox.setText(QString("insertion error :: slotAddStockItem"));
					mbox.exec();
				}
			}
		}
		else
		{
			QMessageBox mbox;
			mbox.setIcon(QMessageBox::Warning);
			mbox.setText(QString("Some fields are empty"));
			mbox.exec();
			return;
		}
	}
	else
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Warning);
		mbox.setText(QString("This item does not exist in the current stock"));
		mbox.exec();
	}
}

double AddStockItem::getExistingQuantityInMainStock() const
{
	return m_existingQuantityInMainStock;
}

void AddStockItem::setExistingQuantityInMainStock(double val)
{
	m_existingQuantityInMainStock = val;
}

void AddStockItem::setUpdate(bool val)
{
	m_update = val;
}

bool AddStockItem::isUpdate() const
{
	return m_update;
}

void AddStockItem::setItemId(QString val)
{
	m_stockId = val;
}

double AddStockItem::getExistingQuantityInStock() const
{
	return m_existingQuantityInStock;
}

void AddStockItem::setExistingQuantityInStock(double val)
{
	m_existingQuantityInStock = val;
}

