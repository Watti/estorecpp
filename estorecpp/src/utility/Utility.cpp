
#include "utility/utility.h"
#include "QMessageBox"
#include "QSqlQuery"
#include "QString"
#include "QVariant"

namespace ES
{
	Utility::Utility()
	{

	}

	Utility::~Utility()
	{

	}

	bool Utility::verifyUsingMessageBox(QWidget *parent, QString title, QString message)
	{
		QMessageBox::StandardButton reply;
		reply = QMessageBox::question(parent, title, message,
			QMessageBox::Yes | QMessageBox::No);
		if (reply == QMessageBox::Yes) {
			return true;
		}
		else
		{
			return false;
		}
	}

	float Utility::getTotalOutstanding(QString customerId)
	{
		float totalAmount;
		QString query;
		query.append("SELECT * FROM customer_outstanding WHERE customer_id = ");
		query.append(customerId);
		query.append(" AND settled = 0");

		QSqlQuery q(query);
		while (q.next())
		{
			QString paymentId = q.value("payment_id").toString();
			QSqlQuery qry("SELECT * FROM payment WHERE payment_id = " + paymentId);
			QString pm = q.value("payment_method").toString();
			float interest = 0;
			if (pm == "CREDIT")
			{
				QSqlQuery qq("SELECT * FROM credit WHERE credit_id = " + q.value("table_id").toString());
				if (qq.next())
				{
					interest = qq.value("interest").toFloat();
					float amount = qq.value("amount").toFloat();
					totalAmount += (amount * (100 + interest) / 100);
				}
			}
			else if (pm == "CHEQUE")
			{
				QSqlQuery qq("SELECT * FROM cheque WHERE cheque_id = " + q.value("table_id").toString());
				if (qq.next())
				{
					interest = qq.value("interest").toFloat();
					float amount = qq.value("amount").toFloat();
					totalAmount += (amount * (100 + interest) / 100);
				}
			}
		}
		return totalAmount;

	}

	float Utility::getOutstandingForBill(int billId)
	{
		float totalOutstanding = 0;
		QSqlQuery queryPayment("SELECT * FROM payment WHERE bill_id = " + QString::number(billId) + " AND valid = 1");
		while (queryPayment.next())
		{
			QString pId = queryPayment.value("payment_id").toString();
			QString type = queryPayment.value("payment_type").toString();
			if (type == "CHEQUE")
			{
				QSqlQuery queryCheque("SELECT * FROM cheque WHERE payment_id = " + pId);
				while (queryCheque.next())
				{
					float amount = queryCheque.value("amount").toFloat();
					float interest = queryCheque.value("interest").toFloat();
					totalOutstanding += (amount * (100 + interest) / 100);
				}
			}
			else if (type == "CREDIT")
			{
				QSqlQuery queryCheque("SELECT * FROM credit WHERE payment_id = " + pId);
				while (queryCheque.next())
				{
					float amount = queryCheque.value("amount").toFloat();
					float interest = queryCheque.value("interest").toFloat();
					totalOutstanding += (amount * (100 + interest) / 100);
				}
			}
		}
		return totalOutstanding;
	}

	void Utility::updateOutstandingAmount(QString customerId, double amount)
	{
		QSqlQuery q("SELECT * FROM customer_outstanding WHERE customer_id = " + customerId);
		if (q.next())
		{
			double currentOutstanding = q.value("current_outstanding").toDouble();
			currentOutstanding += amount;

			QSqlQuery qry;
			qry.prepare("UPDATE customer_outstanding SET current_outstanding = ? WHERE customer_id = ? ");
			qry.addBindValue(currentOutstanding);
			qry.addBindValue(customerId);
			if (!qry.exec())
			{
				QMessageBox mbox;
				mbox.setIcon(QMessageBox::Critical);
				mbox.setText(QString("Failed to add CUSTOMER OUTSTANDING info"));
				mbox.exec();
			}
		}
		else
		{
			QSqlQuery qry;
			qry.prepare("INSERT INTO customer_outstanding (customer_id, current_outstanding, settled, comments) VALUES (?, ?, 0, '')");
			qry.addBindValue(customerId);
			qry.addBindValue(amount);
			if (!qry.exec())
			{
				QMessageBox mbox;
				mbox.setIcon(QMessageBox::Critical);
				mbox.setText(QString("Failed to add CUSTOMER OUTSTANDING info"));
				mbox.exec();
			}
		}
	}

}