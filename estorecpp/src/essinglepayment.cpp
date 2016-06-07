#include "essinglepayment.h"
#include <QMessageBox>
#include "utility\session.h"
#include "QSqlQuery"
#include "QPrintPreviewDialog"
#include "utility\esmainwindowholder.h"
#include "KDReportsTextElement.h"

ESSinglePayment::ESSinglePayment(ESAddBill* addBill, QWidget *parent /*= 0*/) : QWidget(parent), m_addBill(addBill)
{
	m_customerId = "-1";
	ui.setupUi(this);

	ui.cashBtn->setChecked(true);
	ui.lbl1->hide();
	ui.lbl2->hide();
	ui.dateLbl->hide();

	ui.txt1->hide();
	ui.txt2->hide();
	ui.dateEdit->hide();

	ui.paymentType->setText("Cash :  ");

	QObject::connect(ui.cashBtn, SIGNAL(clicked()), this, SLOT(slotPaymentMethodSelected()));
	QObject::connect(ui.creditBtn, SIGNAL(clicked()), this, SLOT(slotPaymentMethodSelected()));
	QObject::connect(ui.chequeBtn, SIGNAL(clicked()), this, SLOT(slotPaymentMethodSelected()));
	QObject::connect(ui.creditCardBtn, SIGNAL(clicked()), this, SLOT(slotPaymentMethodSelected()));
	QObject::connect(ui.loyalityCardBtn, SIGNAL(clicked()), this, SLOT(slotPaymentMethodSelected()));
	
	QObject::connect(ui.cashText, SIGNAL(textChanged(QString)), this, SLOT(slotCalculateBalance()));
	QObject::connect(ui.txt2, SIGNAL(textChanged(QString)), this, SLOT(slotInterestChanged()));
	QObject::connect(ui.okBtn, SIGNAL(clicked()), this, SLOT(slotFinalizeBill()));

	resize(500, 1);
	adjustSize();
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

	//if (ui.paymentMethodCombo->currentText() == "LOYALTY CARD" || ui.paymentMethodCombo->currentText() == "CREDIT CARD")
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
	QString paymentType = "CASH";// ui.paymentMethodCombo->currentText();
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
		handleCreditCardPayment(billId, netAmount);
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
	query.prepare("INSERT INTO payment (bill_id, total_amount, payment_type_id) VALUES (?, ?, 1)");
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
	query.prepare("INSERT INTO payment (bill_id, total_amount, payment_type_id) VALUES (?, ?, 2)");
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
	query.prepare("INSERT INTO payment (bill_id, total_amount, payment_type_id) VALUES (?, ?, 3)");
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

void ESSinglePayment::slotPaymentMethodSelected()
{
	if (ui.creditBtn == sender())
	{
		ui.lbl1->hide();
		ui.lbl2->show(); // interest
		ui.dateLbl->show(); // due date

		ui.txt1->hide();
		ui.txt2->show();
		ui.dateEdit->show();

		ui.lbl2->setText("Interest % :  ");
		ui.paymentType->setText("Amount :  ");

		ui.cashBtn->setChecked(false);
		ui.chequeBtn->setChecked(false);
		ui.creditCardBtn->setChecked(false);
		ui.loyalityCardBtn->setChecked(false);
	}
	else if (ui.chequeBtn == sender())
	{
		ui.lbl1->show(); // cheque no
		ui.lbl2->show(); // bank
		ui.dateLbl->show(); // due date

		ui.txt1->show();
		ui.txt2->show();
		ui.dateEdit->show();

		ui.lbl1->setText("Cheque No. :  ");
		ui.lbl2->setText("Bank :  ");

		ui.cashBtn->setChecked(false);
		ui.creditBtn->setChecked(false);
		ui.creditCardBtn->setChecked(false);
		ui.loyalityCardBtn->setChecked(false);

		ui.paymentType->setText("Amount :  ");
	}
	else if (ui.creditCardBtn == sender())
	{
		ui.lbl1->show(); // card no
		ui.lbl2->hide();
		ui.dateLbl->hide();

		ui.txt1->show();
		ui.txt2->hide();
		ui.dateEdit->hide();

		ui.lbl1->setText("Card No. :  ");
		ui.paymentType->setText("Amount :  ");

		ui.cashBtn->setChecked(false);
		ui.chequeBtn->setChecked(false);
		ui.creditBtn->setChecked(false);
		ui.loyalityCardBtn->setChecked(false);
	}
	else if (ui.loyalityCardBtn == sender())
	{
		ui.lbl1->show(); // card no
		ui.lbl2->hide();
		ui.dateLbl->hide();

		ui.txt1->show();
		ui.txt2->hide();
		ui.dateEdit->hide();

		ui.lbl1->setText("Card No. :  ");
		ui.paymentType->setText("Amount :  ");

		ui.cashBtn->setChecked(false);
		ui.chequeBtn->setChecked(false);
		ui.creditCardBtn->setChecked(false);
		ui.creditBtn->setChecked(false);
	}
	else if (ui.cashBtn == sender())
	{
		ui.lbl1->hide();
		ui.lbl2->hide();
		ui.dateLbl->hide();

		ui.txt1->hide();
		ui.txt2->hide();
		ui.dateEdit->hide();

		ui.paymentType->setText("Cash :  ");

		ui.creditBtn->setChecked(false);
		ui.chequeBtn->setChecked(false);
		ui.creditCardBtn->setChecked(false);
		ui.loyalityCardBtn->setChecked(false);
	}

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

					// stock quantity has been updated, match the update of the stock quantity 
					// with the PO items in stock_purchase_order_item for profit calculation
					// if multiple records are present first one will be updated
					double qty = quantity;
					QSqlQuery qq("SELECT * FROM stock_purchase_order_item WHERE stock_id = " + stockId);
					while (qq.next())
					{
						QString id = qq.value("stock_po_item_id").toString();
						double currentQty = qq.value("remaining_qty").toDouble();
						if (qty <= currentQty)
						{
							QSqlQuery qqq("UPDATE stock_purchase_order_item SET remaining_qty = " + 
								QString::number(currentQty - qty, 'f', 2) + " WHERE stock_po_item_id = " + id);
							break;
						}
						else
						{
							QSqlQuery qqq("UPDATE stock_purchase_order_item SET remaining_qty = 0 WHERE stock_po_item_id = " + id);
							qty -= currentQty;
						}
					}
				}
				
			}
		}

		m_addBill->resetBill();

		if (ui.doPrintCB->isChecked())
		{
			printBill();
		}
	}
}

void ESSinglePayment::slotInterestChanged()
{
	double interest = ui.txt2->text().toDouble();
	double netAmout = ui.netAmountLbl->text().toDouble();

	double totalBill = netAmout + netAmout * (interest / 100.0);

	ui.totalBillLbl->setText(QString::number(totalBill, 'f', 2));
}

void ESSinglePayment::printBill()
{
	KDReports::Report report;

	KDReports::TextElement titleElement("JIRA TASK LIST");
	titleElement.setPointSize(15);
	report.addElement(titleElement, Qt::AlignHCenter);

	QPrinter printer;
	printer.setPaperSize(QPrinter::A4);

	printer.setFullPage(false);
	printer.setOrientation(QPrinter::Portrait);
	report.print(&printer);
}

void ESSinglePayment::handleCreditCardPayment(int billId, double netAmount)
{

	QSqlQuery query;
	query.prepare("INSERT INTO payment (bill_id, total_amount, payment_type_id) VALUES (?, ?, 4)");
	query.addBindValue(billId);
	query.addBindValue(netAmount);
	if (query.exec())
	{
		int lastInsertedId = query.lastInsertId().toInt();
		QSqlQuery q;
		q.prepare("INSERT INTO credit_card (payment_id, amount, card_no) VALUES (?, ?, ?)");
		q.addBindValue(lastInsertedId);
		q.addBindValue(netAmount);
		q.addBindValue(ui.txt1->text());
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

