#include "esaddstockitem.h"
#include "utility/esdbconnection.h"
#include <QMessageBox>
#include <QPushButton>


AddStockItem::AddStockItem(QWidget *parent /*= 0*/)
: QWidget(parent)
{
	ui.setupUi(this);
	QObject::connect(ui.addItemButton, SIGNAL(clicked()), this, SLOT(slotAddStockItem()));
}

AddStockItem::~AddStockItem()
{

}

void AddStockItem::slotAddStockItem()
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
		double minQty = qtyStr.toDouble(&isValid);
		if (!isValid)
		{
			QMessageBox mbox;
			mbox.setIcon(QMessageBox::Warning);
			mbox.setText(QString("Invalid input - Minimum Quantity"));
			mbox.exec();
		}
		if (ES::DbConnection::instance()->open())
		{
			QString itemId = ui.itemIDLabel->text();
			QString  price = ui.itemPrice->text();
			price.toDouble(&isValid);
			if (isValid)
			{
				for (auto it : m_priceMap.keys())
				{
					if (m_priceMap.value(it) == price)
					{
						QString priceId = it;
						QSqlQuery query;
						QString q("INSERT INTO stock  (item_id, itemprice_id , qty, min_qty, deleted) VALUES(" + itemId + ", " + priceId + ", " + qtyStr + "," + minQtyStr + ", 0) ");
						if (!query.exec(q))
						{
							QMessageBox mbox;
							mbox.setIcon(QMessageBox::Critical);
							mbox.setText(QString("insertion error"));
							mbox.exec();
						}
					}
				}
			}
		}
		else
		{
			QMessageBox mbox;
			mbox.setIcon(QMessageBox::Critical);
			mbox.setText(QString("Database Connection Error : AddStockItem::slotAddStockItem"));
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

void AddStockItem::addToPriceMap(QString key, QString value)
{
	m_priceMap.insert(key, value);
}
