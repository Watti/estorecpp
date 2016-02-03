#include "esaddstockitem.h"
#include "utility/esdbconnection.h"
#include <QMessageBox>
#include <QPushButton>
#include "esmanagestockitems.h"
#include "utility/esmainwindowholder.h"


AddStockItem::AddStockItem(QWidget *parent /*= 0*/)
: QWidget(parent), existingQuantity(0)
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
	if (existingQuantity > 0)
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
			if (quantity <= existingQuantity)
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
					QSqlQuery query;
					QString q("INSERT INTO stock  (item_id,  qty, min_qty, deleted) VALUES(" + itemId + ", " + qtyStr + "," + minQtyStr + ", 0) ");
					if (query.exec(q))
					{
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
				mbox.setText(QString("Inserted quantity value exceeds the existing quantity"));
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

double AddStockItem::getExistingQuantity() const
{
	return existingQuantity;
}

void AddStockItem::setExistingQuantity(double val)
{
	existingQuantity = val;
}

