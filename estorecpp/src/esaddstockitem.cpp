#include "esaddstockitem.h"
#include "utility/esdbconnection.h"
#include <QMessageBox>
#include <QPushButton>
#include "esmanagestockitems.h"
#include "utility/esmainwindowholder.h"
#include "esmainwindow.h"
#include "utility/utility.h"

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
			}
			if (quantity >= 0)
			{
				if ( quantity < m_existingQuantityInMainStock)
				{
					if (!ES::Utility::verifyUsingMessageBox(this, "EStore", "Do you really want to reduce the stock quantity?"))
					{
						return;
					}
				}
				double minQty = qtyStr.toDouble(&isValid);
				if (!isValid)
				{
					QMessageBox mbox;
					mbox.setIcon(QMessageBox::Warning);
					mbox.setText(QString("Invalid input - Minimum Quantity"));
					mbox.exec();
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
				discount.toDouble(&isValid);
				if (!isValid)
				{
					QMessageBox mbox;
					mbox.setIcon(QMessageBox::Warning);
					mbox.setText(QString("Invalid input - Discount"));
					mbox.exec();
					return;
				}
				if (isValid)
				{
					QString q;
					if (isUpdate())
					{
						q = "UPDATE stock SET qty = '" + qtyStr + "', min_qty = '" + minQtyStr + "' ,selling_price = '"+price+"', discount ='"+discount+"' WHERE stock_id = " + m_stockId;
					}
// 					else
// 					{
// 						q = "INSERT INTO stock  (item_id,  qty, min_qty, discount, deleted) VALUES(" + itemId + ", " + qtyStr + "," + minQtyStr + "," + discount + ", 0) ";
// 					}

					QSqlQuery query;
					if (query.exec(q))
					{
						double remainingQtyInMainStock = -1;
						if (isUpdate())
						{
							remainingQtyInMainStock =  quantity;
						}
						else
						{
							 remainingQtyInMainStock = quantity;
						}
						QString updateOrder("UPDATE stock SET qty = " + QString::number(remainingQtyInMainStock)+" WHERE stock_id = "+itemId);
						QSqlQuery qUpdateOrder;
						if (!qUpdateOrder.exec(updateOrder))
						{
							QMessageBox mbox;
							mbox.setIcon(QMessageBox::Critical);
							mbox.setText(QString("insertion error :: cannot reduce this quantity from the main stock order"));
							mbox.exec();
						}

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
				mbox.setText(QString("Invalid Quantity"));
				mbox.exec();
			}
		}
		else
		{
			QMessageBox mbox;
			mbox.setIcon(QMessageBox::Warning);
			mbox.setText(QString("Some fields are empty"));
			mbox.exec();
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

