#include "essinglepayment.h"
#include <QMessageBox>
#include "utility\session.h"
#include "QSqlQuery"


ESSinglePayment::ESSinglePayment(ESAddBill* addBill, QWidget *parent /*= 0*/) : QWidget(parent), m_addBill(addBill)
{
	m_customerId = "-1";
	ui.setupUi(this);

	slotPaymentMethodSelected(ui.paymentMethodCombo->currentText());

	QObject::connect(ui.paymentMethodCombo, SIGNAL(activated(QString)), this, SLOT(slotPaymentMethodSelected(QString)));
	QObject::connect(ui.cashText, SIGNAL(textChanged(QString)), this, SLOT(slotCalculateBalance()));
	QObject::connect(ui.okBtn, SIGNAL(clicked()), this, SLOT(slotFinalizeBill()));
}

ESSinglePayment::~ESSinglePayment()
{

}

void ESSinglePayment::slotSearch()
{

}


void ESSinglePayment::slotCalculateBalance()
{
	bool isValid = false;
	double cash = 0, cardAmount = 0;
	if (!ui.cashText->text().isEmpty())
	{
		cash = ui.cashText->text().toDouble(&isValid);
		if (!isValid)
		{
			QMessageBox mbox;
			mbox.setIcon(QMessageBox::Warning);
			mbox.setText(QString("Invalid input - Cash"));
			mbox.exec();
			ui.cashText->clear();
			return;
		}
	}

	if (ui.paymentMethodCombo->currentText() == "LOYALTY CARD" || ui.paymentMethodCombo->currentText() == "CREDIT CARD")
	{
		/*if (!ui.cardAmountText->text().isEmpty())
		{
		cardAmount = ui.cardAmountText->text().toDouble(&isValid);

		if (!isValid)
		{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Warning);
		mbox.setText(QString("Invalid input - Card Amount"));
		mbox.exec();
		ui.cardAmountText->clear();
		return;
		}
		}*/
	}
	double amount = ui.totalBillLbl->text().toDouble();

	ui.balanceLbl->setText(QString::number(cash - (amount - cardAmount), 'f', 2));

	if (ui.cashText->text().isEmpty())
	{
		ui.balanceLbl->clear();
	}
}


void ESSinglePayment::slotFinalizeBill()
{
	if (!validate())
		return;

	QString billIdStr = ES::Session::getInstance()->getBillId();
	QString netAmountStr = ui.totalBillLbl->text();
	QString paymentType = ui.paymentMethodCombo->currentText();
	bool isValid = false;

	double netAmount = netAmountStr.toDouble(&isValid);
	if (!isValid || netAmount < 0)
	{
		return;
	}		
	int billId = billIdStr.toInt(&isValid);
	if (!isValid)
	{
		return;
	}

	if (paymentType == "CASH")
	{
		handleCashPayment(billId, netAmount);
	}
	else if (paymentType == "CREDIT")
	{
		handleCreditPayment(billId, netAmount);
	}
	else if (paymentType == "CHEQUE")
	{
		handleChequePayment(billId, netAmount);
	}
	else if (paymentType == "CREDIT CARD")
	{
	}
	else if (paymentType == "LOYALITY CARD")
	{
	}

	this->close();
}

bool ESSinglePayment::validate()
{
	return true;
}

void ESSinglePayment::handleCashPayment(int billId, double netAmount)
{
	QSqlQuery query;
	query.prepare("INSERT INTO payment (bill_id, total_amount, cash) VALUES (?, ?, 1)");
	query.addBindValue(billId);
	query.addBindValue(netAmount);
	if (query.exec())
	{
		int lastInsertedId = query.lastInsertId().toInt();
		QSqlQuery q;
		q.prepare("INSERT INTO cash (payment_id, amount) VALUES (?, ?)");
		q.addBindValue(lastInsertedId);
		q.addBindValue(netAmount);
		if (!q.exec())
		{
			QMessageBox mbox;
			mbox.setIcon(QMessageBox::Critical);
			mbox.setText(QString("Failed"));
			mbox.exec();
		}
		else
		{
			finishBill(netAmount, billId);
		}
	}
	else
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Critical);
		mbox.setText(QString("Failed"));
		mbox.exec();
	}
}

void ESSinglePayment::handleCreditPayment(int billId, double netAmount)
{
	QSqlQuery query;
	query.prepare("INSERT INTO payment (bill_id, total_amount, credit) VALUES (?, ?, 1)");
	query.addBindValue(billId);
	query.addBindValue(netAmount);
	if (query.exec())
	{
		int lastInsertedId = query.lastInsertId().toInt();
		QSqlQuery q;
		q.prepare("INSERT INTO credit (payment_id, amount, due_date) VALUES (?, ?, ?)");
		q.addBindValue(lastInsertedId);
		q.addBindValue(netAmount);
		q.addBindValue(ui.dateEdit->text());
		if (!q.exec())
		{
			QMessageBox mbox;
			mbox.setIcon(QMessageBox::Critical);
			mbox.setText(QString("Failed"));
			mbox.exec();
		}
		else
		{
			finishBill(netAmount, billId);
		}
	}
	else
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Critical);
		mbox.setText(QString("Failed"));
		mbox.exec();
	}
}

void ESSinglePayment::handleChequePayment(int billId, double netAmount)
{
	QSqlQuery query;
	query.prepare("INSERT INTO payment (bill_id, total_amount, cheque) VALUES (?, ?, 1)");
	query.addBindValue(billId);
	query.addBindValue(netAmount);
	if (query.exec())
	{
		int lastInsertedId = query.lastInsertId().toInt();
		QSqlQuery q;
		q.prepare("INSERT INTO cheque (payment_id, amount, cheque_number, bank, due_date) VALUES (?, ?, ?, ?, ?)");
		q.addBindValue(lastInsertedId);
		q.addBindValue(netAmount);
		q.addBindValue(ui.txt1->text());
		q.addBindValue(ui.txt2->text());
		q.addBindValue(ui.dateEdit->text());
		if (!q.exec())
		{
			QMessageBox mbox;
			mbox.setIcon(QMessageBox::Critical);
			mbox.setText(QString("Failed"));
			mbox.exec();
		}
		else
		{
			finishBill(netAmount, billId);
		}
	}
	else
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Critical);
		mbox.setText(QString("Failed"));
		mbox.exec();
	}
}

void ESSinglePayment::slotPaymentMethodSelected(QString pmMethod)
{
	if (pmMethod == "CREDIT")
	{
		ui.lbl1->hide();
		ui.lbl2->hide();
		ui.dateLbl->show(); // due date

		ui.txt1->hide();
		ui.txt2->hide();
		ui.dateEdit->show();

		ui.paymentType->setText("Amount :  ");
	}
	else if (pmMethod == "CHEQUE")
	{
		ui.lbl1->show(); // cheque no
		ui.lbl2->show(); // bank
		ui.dateLbl->show(); // due date

		ui.txt1->show();
		ui.txt2->show();
		ui.dateEdit->show();

		ui.lbl1->setText("Cheque No. :  ");
		ui.lbl2->setText("Bank :  ");

		ui.paymentType->setText("Amount :  ");
	}
	else if (pmMethod == "CREDIT CARD")
	{
		ui.lbl1->show(); // card no
		ui.lbl2->hide();
		ui.dateLbl->hide();

		ui.txt1->show();
		ui.txt2->hide();
		ui.dateEdit->hide();

		ui.lbl1->setText("Card No. :  ");
		ui.paymentType->setText("Amount :  ");
	}
	else if (pmMethod == "LOYALITY CARD")
	{
		ui.lbl1->show(); // card no
		ui.lbl2->hide();
		ui.dateLbl->hide();

		ui.txt1->show();
		ui.txt2->hide();
		ui.dateEdit->hide();

		ui.lbl1->setText("Card No. :  ");
		ui.paymentType->setText("Amount :  ");
	}
	else if (pmMethod == "CASH")
	{
		ui.lbl1->hide();
		ui.lbl2->hide();
		ui.dateLbl->hide();

		ui.txt1->hide();
		ui.txt2->hide();
		ui.dateEdit->hide();

		ui.paymentType->setText("Cash :  ");
	}

	m_selectedPM = pmMethod;
	resize(500, 1);
	adjustSize();
}

void ESSinglePayment::setCustomerId(QString customerId)
{
	m_customerId = customerId;
}

void ESSinglePayment::finishBill(double netAmount, int billId)
{
	QSqlQuery qq;
	qq.prepare("UPDATE bill SET amount = ?, customer_id = ?, status = 1 WHERE bill_id = ?");
	qq.addBindValue(netAmount);
	qq.addBindValue(m_customerId);
	qq.addBindValue(billId);
	if (!qq.exec())
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Critical);
		mbox.setText(QString("Failed"));
		mbox.exec();
	}
	else
	{
		// Update stock quantity
		QSqlQuery saleQuantityQuery;
		saleQuantityQuery.prepare("SELECT * FROM sale WHERE bill_id = ?");
		saleQuantityQuery.addBindValue(billId);
		if (saleQuantityQuery.exec())
		{
			while (saleQuantityQuery.next())
			{
				QString stockId = saleQuantityQuery.value("stock_id").toString();
				double quantity = saleQuantityQuery.value("quantity").toDouble();
				
				QSqlQuery q("SELECT * FROM stock WHERE stock_id = " + stockId);
				if (q.next())
				{
					double stockQuantity = q.value("qty").toDouble();
					double remainingQty = stockQuantity - quantity;
					QSqlQuery stockUpdateQuery;
					stockUpdateQuery.prepare("UPDATE stock SET qty = ? WHERE stock_id = ?");
					stockUpdateQuery.addBindValue(remainingQty);
					stockUpdateQuery.addBindValue(stockId);
					stockUpdateQuery.exec();
				}
				
			}
		}

		m_addBill->resetBill();
	}
}

