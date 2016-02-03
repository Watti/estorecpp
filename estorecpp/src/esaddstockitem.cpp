#include "esaddstockitem.h"
#include "utility/esdbconnection.h"
#include <QMessageBox>
#include <QPushButton>
#include "esmanagestockitems.h"
#include "utility/esmainwindowholder.h"


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
	if (m_existingQuantityInMainStock > 0)
	{
		QString qtyStr = ui.qty->text();
		QString minQtyStr = ui.minQty->text();
		QString priceStr = ui.itemPrice->text();

		if (!qtyStr.isEmpty() && !minQtyStr.isEmpty() && !priceStr.isEmpty())
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
			if (quantity >= 0 && quantity <= m_existingQuantityInMainStock)
			{
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
				price.toDouble(&isValid);
				if (isValid)
				{
					QString q;
					if (isUpdate())
					{
						q = "UPDATE stock SET qty = '" + qtyStr + "', min_qty = '" + minQtyStr + "' WHERE item_id = " + m_itemId;
					}
					else
					{
						q = "INSERT INTO stock  (item_id,  qty, min_qty, deleted) VALUES(" + itemId + ", " + qtyStr + "," + minQtyStr + ", 0) ";
					}

					QSqlQuery query;
					if (query.exec(q))
					{
						double remainingQtyInMainStock = -1;
						if (isUpdate())
						{
							remainingQtyInMainStock = m_existingQuantityInMainStock + (m_existingQuantityInStock - quantity);
						}
						else
						{
							 remainingQtyInMainStock = m_existingQuantityInMainStock - quantity;
						}
						QString updateOrder("UPDATE stock_order SET quantity = " + QString::number(remainingQtyInMainStock)+" WHERE item_id = "+itemId);
						QSqlQuery qUpdateOrder;
						if (!qUpdateOrder.exec(updateOrder))
						{
							QMessageBox mbox;
							mbox.setIcon(QMessageBox::Critical);
							mbox.setText(QString("insertion error :: cannot reduce this quantity from the main stock order"));
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
				mbox.setText(QString("Inserted quantity value exceeds the existing quantity or invalid quantity"));
				mbox.exec();
			}
		}
		else
		{
			QMessageBox mbox;
			mbox.setIcon(QMessageBox::Warning);
			mbox.setText(QString("Fields are Empty"));
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
	m_itemId = val;
}

double AddStockItem::getExistingQuantityInStock() const
{
	return m_existingQuantityInStock;
}

void AddStockItem::setExistingQuantityInStock(double val)
{
	m_existingQuantityInStock = val;
}

