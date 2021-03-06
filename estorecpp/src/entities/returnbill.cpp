#include "entities/returnbill.h"
#include <QtGui>
#include <QSqlQuery>
#include <QMessageBox>
#include "QPushButton"
#include "utility/session.h"

ES::ReturnBill::ReturnBill()
{
	m_started = false;
	m_billId = -1;
	m_oldBillId = -1;
	m_hasInterest = false;
	m_interest = 0.0;
	m_subTotal = 0.0;
	m_total = 0.0;
	m_newInterest = 0.0;
	m_newSubTotal = 0.0;
	m_newTotal = 0.0;

	m_returnItemsIDGenerator = 0;
	m_newItemsIDGenerator = 0;
}

ES::ReturnBill::~ReturnBill()
{
	end();
}

bool ES::ReturnBill::start()
{
	QSqlQuery insertBillQuery;

	insertBillQuery.prepare("INSERT INTO bill (user_id) VALUES(?)");
	insertBillQuery.addBindValue(ES::Session::getInstance()->getUser()->getId());

	QSqlDatabase::database().transaction();
	if (insertBillQuery.exec())
	{
		m_billId = (insertBillQuery.lastInsertId()).value<int>();
		m_started = true;
	}
	bool success = QSqlDatabase::database().commit();
	return success;
}

void ES::ReturnBill::end()
{
	// todo - delete records from db
	m_started = false;
	m_billId = -1;
	m_oldBillId = -1;
	m_hasInterest = false;
	m_interest = 0.0;
	m_subTotal = 0.0;
	m_total = 0.0;
	m_newInterest = 0.0;
	m_newSubTotal = 0.0;
	m_newTotal = 0.0;
	m_returnItems.clear();
	m_newItems.clear();

	m_returnItemsIDGenerator = 0;
	m_newItemsIDGenerator = 0;
}

bool ES::ReturnBill::addReturnItem(QString oldBillId, QString itemCode)
{
	// Check for the same bill id
	if (m_oldBillId == -1)
	{
		m_oldBillId = oldBillId.toInt();
	}
	else
	{
		if (m_oldBillId != oldBillId.toInt())
		{
			return false;
		}
	}

	// Check if the bill has an interest
	QSqlQuery pQ("SELECT payment_type FROM payment WHERE bill_id = " + oldBillId);
	if (pQ.next())
	{
		QString pm = pQ.value("payment_type").toString();
		if (pm == "CREDIT" || pm == "CHEQUE")
		{
			m_hasInterest = true;
		}
	}

	// Try to find the item information
	int itemId = -1;
	QString itemName = "-1";
	QSqlQuery q("SELECT item_id, item_name FROM item WHERE deleted = 0 AND item_code = '" + itemCode + "'");
	if (q.next())
	{
		itemId = q.value("item_id").toInt();
		itemName = q.value("item_name").toString();
	}
	else
	{
		return false;
	}

	// Now get the sale information from the 'sale' table
	QSqlQuery q2("SELECT stock_id FROM stock WHERE item_id = " + QString::number(itemId));
	if (q2.next())
	{
		QString str("SELECT b.* FROM sale b JOIN stock s ON b.stock_id = s.stock_id AND s.stock_id = ");
		str.append(q2.value("stock_id").toString());
		str.append(" WHERE b.bill_id = ");
		str.append(oldBillId);
		str.append(" AND b.deleted = 0");

		ReturnItemInfo bi;
		QSqlQuery q3(str);
		if (q3.next())
		{
			bi.itemCode = itemCode;
			bi.itemName = itemName;

			QString qtyStr = q3.value("quantity").toString();
			double quantity = qtyStr.toDouble();
			bi.quantity = quantity;
			
			double itemPrice = q3.value("item_price").toDouble();
			bi.itemPrice = itemPrice;

			double discount = q3.value("discount").toDouble();
			double paidPrice = itemPrice - (itemPrice * discount / 100.0);
			bi.paidPrice = paidPrice;

			double returnPrice = paidPrice * quantity;
			bi.returnPrice = returnPrice;

			QDate d = q3.value("date").toDate();
			bi.date = d.toString("yyyy-MM-dd");

			bi.billedQuantity = quantity;
		}
		else
		{
			return false;
		}
		m_returnItems[m_returnItemsIDGenerator++] = bi;
	}
	else
	{
		return false;
	}

	calculateTotal();
	return true;
}

void ES::ReturnBill::removeReturnItem(QString rowId)
{
	std::map<int, ReturnItemInfo>::iterator iter = m_returnItems.find(rowId.toInt());
	if (iter != m_returnItems.end())
	{
		m_returnItems.erase(iter);
	}
	calculateTotal();
}

void ES::ReturnBill::addNewItem(QString stockId)
{
	QSqlQuery queryStock("SELECT item_id, discount, selling_price FROM stock WHERE stock_id = " + stockId);
	if (queryStock.next())
	{
		QString itemId = queryStock.value("item_id").toString();
		double discount = queryStock.value("discount").toDouble();
		double itemPrice = queryStock.value("selling_price").toDouble();
		
		NewItemInfo ni;
		ni.stockId = stockId.toLong();
		ni.itemPrice = itemPrice;
		ni.discount = discount;
		ni.quantity = 0;
#if NETWORK_SYSTEM
			QString qSession = "INSERT INTO bill_session (stock_id, bill_id, qty, item_id, sale_id) VALUES(" + stockId + ", " +
				QString::number(m_billId) + ", 0 ," + itemId + ", -1)";
			QSqlDatabase::database().transaction();
			QSqlQuery querySession(qSession);
			bool success = QSqlDatabase::database().commit();
		if (!success)
		{
			QMessageBox mbox;
			mbox.setIcon(QMessageBox::Warning);
			mbox.setText(QString("Database server is busy ! You may have to add this item again"));
			mbox.exec();
		}
#endif
		QSqlQuery itemQ("SELECT * FROM item WHERE item_id=" + itemId);
		if (itemQ.next())
		{
			ni.itemCode = itemQ.value("item_code").toString();
			ni.itemName = itemQ.value("item_name").toString();
			ni.wCost = itemQ.value("w_cost").toDouble();
		}
		m_newItems[m_newItemsIDGenerator++] = ni;
	}
	calculateTotal();
}

void ES::ReturnBill::removeNewItem(int rowId)
{
	qDebug() << rowId << " removed...\n";
	std::map<int, NewItemInfo>::iterator iter = m_newItems.find(rowId);
	if (iter != m_newItems.end())
	{
#if NETWORK_SYSTEM
		QSqlQuery queryDeleteBillSession("DELETE FROM bill_session WHERE bill_id = " + QString::number(m_billId) + " AND stock_id = " + QString::number(iter->second.stockId));
#endif
		m_newItems.erase(iter);
	}
	for (std::map<int, NewItemInfo>::iterator it = m_newItems.begin(), ite = m_newItems.end(); it != ite; ++it)
	{
		NewItemInfo& ni = it->second;
		qDebug() << it->first << " : " << ni.itemCode << ", " << ni.quantity;
	}
	calculateTotal();
}

void ES::ReturnBill::commit()
{

}

void ES::ReturnBill::cancel()
{
	// todo - set bill status 'cancel'
}

const std::map<int, ES::ReturnBill::ReturnItemInfo>& ES::ReturnBill::getReturnItemTable() const
{
	return m_returnItems;
}

const std::map<int, ES::ReturnBill::NewItemInfo>& ES::ReturnBill::getNewItemTable() const
{
	return m_newItems;
}

double ES::ReturnBill::getSubTotal()
{
	return m_subTotal;
}

double ES::ReturnBill::getTotal()
{
	return m_total;
}

void ES::ReturnBill::calculateTotal()
{
	double total = 0.0;

	for (std::map<int, ReturnItemInfo>::iterator it = m_returnItems.begin(), ite = m_returnItems.end(); it != ite; ++it)
	{
		const ReturnItemInfo& bi = it->second;
		total += bi.returnPrice;
	}

	m_subTotal = total;

	double netTotal = total;
	if (m_hasInterest)
	{
		netTotal = netTotal + (netTotal * m_interest * 0.01);
	}

	m_total = netTotal;

	//////////////////////////////////////////////////////////////////////////

	double newTotal = 0.0;
	for (std::map<int, NewItemInfo>::iterator it = m_newItems.begin(), ite = m_newItems.end(); it != ite; ++it)
	{
		const NewItemInfo& ni = it->second;
		newTotal += (ni.itemPrice * ni.quantity);
	}

	m_newSubTotal = newTotal;

	double newNetTotal = newTotal;
	newNetTotal = newNetTotal + (newNetTotal * m_newInterest * 0.01);
	m_newTotal = newNetTotal;
}

void ES::ReturnBill::setInterest(QString interest)
{
	if (!interest.isEmpty())
	{
		m_interest = interest.toDouble();
	}
	else
	{
		m_interest = 0;
	}
	calculateTotal();
}

bool ES::ReturnBill::updateItemQuantity(long rowId, QString qtyStr, double& billedQty, double& returnPrice)
{
	std::map<int, ReturnItemInfo>::iterator iter = m_returnItems.find(rowId);
	if (iter != m_returnItems.end())
	{
		ReturnItemInfo& bi = iter->second;
		double qty = qtyStr.toDouble();

		if (bi.billedQuantity < qty || qty <= 0.0)
		{
			billedQty = bi.quantity;
			return false;
		}

		bi.quantity = qty;
		bi.returnPrice = bi.paidPrice * qty;
		returnPrice = bi.returnPrice;
		calculateTotal();
		
		return true;
	}
	return false;
}

bool ES::ReturnBill::updateNewItemQuantity(long rowId, QString qtyStr)
{
	std::map<int, NewItemInfo>::iterator iter = m_newItems.find(rowId);
	if (iter != m_newItems.end())
	{
		NewItemInfo& ni = iter->second;
		bool valid = false;
		double requestedQty = qtyStr.toDouble(&valid);
		if (!valid)
		{
			return false;
		}

		QSqlQuery query("SELECT qty FROM stock WHERE stock_id = " + QString::number(ni.stockId));
		if (query.first())
		{
			double currentQty = query.value("qty").toDouble();
			double sessionQty = 0;
#if NETWORK_SYSTEM
			QSqlQuery querySession("SELECT SUM(qty) as sessionQty FROM bill_session WHERE stock_id = " + QString::number(ni.stockId) + " AND bill_id <>" + QString::number(m_billId));
			if (querySession.next())
			{
				sessionQty = querySession.value("sessionQty").toDouble();
			}
#endif // NETWORK_SYSTEM
			double availableQty = currentQty - sessionQty;
			if (requestedQty > availableQty)
			{
				requestedQty = availableQty;
				QString txt = QString::number(availableQty);
				QMessageBox mbox;
				mbox.setIcon(QMessageBox::Critical);
				mbox.setText(QString("Low stock"));
				mbox.exec(); 
				//return false;
			}
		}

		ni.quantity = requestedQty;
		calculateTotal();
#if NETWORK_SYSTEM
		QSqlQuery qUpdateBillSession("UPDATE bill_session SET qty =" + QString::number(requestedQty) + " WHERE sale_id = -1 AND bill_id = " + QString::number(m_billId)+" AND stock_id = "+QString::number(ni.stockId));
#endif
		return true;
	}
	return false;
}

void ES::ReturnBill::setNewInterest(QString interest)
{
	if (!interest.isEmpty())
	{
		m_newInterest = interest.toDouble();
	}
	else
	{
		m_newInterest = 0;
	}
	calculateTotal();
}

double ES::ReturnBill::getNewSubTotal()
{
	return m_newSubTotal;
}

double ES::ReturnBill::getNewTotal()
{
	return m_newTotal;
}

bool ES::ReturnBill::updateNewItemPrice(long rowId, QString priceStr)
{
	std::map<int, NewItemInfo>::iterator iter = m_newItems.find(rowId);
	if (iter != m_newItems.end())
	{
		NewItemInfo& ni = iter->second;
		bool valid = false;
		double requestedPrice = priceStr.toDouble(&valid);
		if (!valid)
		{
			return false;
		}
		ni.itemPrice = requestedPrice;
		calculateTotal();
		return true;
	}
	return false;
}
