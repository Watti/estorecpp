#include "entities/returnbill.h"
#include <QtGui>
#include <QSqlQuery>
#include <QMessageBox>
#include "QPushButton"

ES::ReturnBill::ReturnBill()
{
	m_started = false;
	m_billId = -1;
	m_oldBillId = -1;
	m_hasInterest = false;
	m_interest = 0.0;
	m_subTotal = 0.0;
	m_total = 0.0;
}

ES::ReturnBill::~ReturnBill()
{
	end();
}

void ES::ReturnBill::start()
{
	m_started = true;
	// todo - generate bill id
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
	m_returnItems.clear();
	m_newItems.clear();
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
	QSqlQuery q("SELECT item_id, item_name FROM item WHERE item_code = '" + itemCode + "'");
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

		QStringList sl;
		QSqlQuery q3(str);
		if (q3.next())
		{
			sl.push_back(itemCode);
			sl.push_back(itemName);

			QString qtyStr = q3.value("quantity").toString();
			double quantity = qtyStr.toDouble();
			sl.push_back(qtyStr);
			
			double itemPrice = q3.value("item_price").toDouble();
			sl.push_back(QString::number(itemPrice, 'f', 2));

			double discount = q3.value("discount").toDouble();
			double paidPrice = itemPrice - (itemPrice * discount / 100.0);
			sl.push_back(QString::number(paidPrice, 'f', 2));

			double returnPrice = paidPrice * quantity;
			sl.push_back(QString::number(returnPrice, 'f', 2));

			QDate d = q3.value("date").toDate();
			sl.push_back(d.toString("yyyy-MM-dd"));
			sl.push_back(QString::number(quantity));
		}
		m_returnItems[m_returnItems.size()] = sl;
	}

	calculateTotal();
	return true;
}

void ES::ReturnBill::addNewItem()
{

}

void ES::ReturnBill::commit()
{

}

void ES::ReturnBill::cancel()
{
	// todo - set bill status 'cancel'
}

const std::map<int, QStringList>& ES::ReturnBill::getReturnItemTable() const
{
	return m_returnItems;
}

const std::map<int, QStringList>& ES::ReturnBill::getNewItemTable() const
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

	for (std::map<int,QStringList>::iterator it = m_returnItems.begin(), ite = m_returnItems.end(); it != ite; ++it)
	{
		const QStringList& sl = it->second;
		total += sl[5].toDouble();
	}

	m_subTotal = total;

	double netTotal = total;
	if (m_hasInterest)
	{
		netTotal = netTotal + (netTotal * m_interest * 0.01);
	}

	m_total = netTotal;
}

void ES::ReturnBill::setInterest(QString interest)
{
	if (!interest.isEmpty())
	{
		m_interest = interest.toDouble();
	}
	calculateTotal();
}

void ES::ReturnBill::removeReturnItem(QString rowId)
{
	std::map<int, QStringList>::iterator iter = m_returnItems.find(rowId.toInt());
	if (iter != m_returnItems.end())
	{
		m_returnItems.erase(iter);
	}
	calculateTotal();
}

void ES::ReturnBill::removeNewItem(QString rowId)
{

}
