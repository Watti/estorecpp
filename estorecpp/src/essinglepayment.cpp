#include "essinglepayment.h"
#include <QMessageBox>
#include "utility\session.h"
#include "esmainwindow.h"
#include "QSqlQuery"
#include "QPrintPreviewDialog"
#include "utility\esmainwindowholder.h"
#include "KDReportsTextElement.h"
#include "KDReportsTableElement.h"
#include "KDReportsCell.h"
#include "KDReportsHtmlElement.h"
#include "QShortcut"
#include "KDReportsHeader.h"
#include "QMainWindow"

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

	ui.label_4->hide();
	ui.lineEdit->hide();
	ui.interestPercentageCB->hide();

	ui.paymentType->setText("Cash :  ");

	ui.cashBtn->setIcon(QIcon("icons/cash_payment1.png"));
	ui.cashBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	ui.cashBtn->setText("CASH");
	ui.cashBtn->setIconSize(QSize(48, 48));

	ui.creditBtn->setIcon(QIcon("icons/credit_payment.png"));
	ui.creditBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	ui.creditBtn->setText("CREDIT");
	ui.creditBtn->setIconSize(QSize(48, 48));

	ui.creditCardBtn->setIcon(QIcon("icons/creditcard_payment.png"));
	ui.creditCardBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	ui.creditCardBtn->setText("CARD");
	ui.creditCardBtn->setIconSize(QSize(48, 48));

	ui.chequeBtn->setIcon(QIcon("icons/cheque_payment.png"));
	ui.chequeBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	ui.chequeBtn->setText("CHEQUE");
	ui.chequeBtn->setIconSize(QSize(48, 48));

	ui.loyalityCardBtn->setIcon(QIcon("icons/loyalty_payment.png"));
	ui.loyalityCardBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	ui.loyalityCardBtn->setText("LOYALTY");
	ui.loyalityCardBtn->setIconSize(QSize(48, 48));

	QObject::connect(ui.cashBtn, SIGNAL(clicked()), this, SLOT(slotPaymentMethodSelected()));
	QObject::connect(ui.creditBtn, SIGNAL(clicked()), this, SLOT(slotPaymentMethodSelected()));
	QObject::connect(ui.chequeBtn, SIGNAL(clicked()), this, SLOT(slotPaymentMethodSelected()));
	QObject::connect(ui.creditCardBtn, SIGNAL(clicked()), this, SLOT(slotPaymentMethodSelected()));
	QObject::connect(ui.loyalityCardBtn, SIGNAL(clicked()), this, SLOT(slotPaymentMethodSelected()));

	QObject::connect(ui.cashText, SIGNAL(textChanged(QString)), this, SLOT(slotCalculateBalance()));
	QObject::connect(ui.lineEdit, SIGNAL(textChanged(QString)), this, SLOT(slotInterestChanged()));
	QObject::connect(ui.okBtn, SIGNAL(clicked()), this, SLOT(slotFinalizeBill()));
	//QObject::connect(ui.discountText, SIGNAL(textChanged(QString)), this, SLOT(slotDiscountPercentage()));

	new QShortcut(QKeySequence(Qt::Key_Escape), this, SLOT(close()));
	//resize(400, 1);
	adjustSize();
	ui.dateEdit->setDate(QDate::currentDate());
	m_paymentMethod = "CASH";
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
	double cash = 0;
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

	double amount = ui.totalBillLbl->text().toDouble();

	ui.balanceLbl->setText(QString::number(cash - amount, 'f', 2));

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
	QString netAmountStr = ui.netAmountLbl->text();
	//QString paymentType = "CASH";// ui.paymentMethodCombo->currentText();
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

	if (m_paymentMethod == "CASH")
	{
		handleCashPayment(billId, netAmount);
	}
	else if (m_paymentMethod == "CREDIT")
	{
		handleCreditPayment(billId, netAmount);
	}
	else if (m_paymentMethod == "CHEQUE")
	{
		handleChequePayment(billId, netAmount);
	}
	else if (m_paymentMethod == "CARD")
	{
		handleCreditCardPayment(billId, netAmount);
	}
	else if (m_paymentMethod == "LOYALITY")
	{
		handleLoyaltyPayment(billId, netAmount);
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
	query.prepare("INSERT INTO payment (bill_id, total_amount, payment_type) VALUES (?, ?, 'CASH')");
	query.addBindValue(billId);
	query.addBindValue(QString::number(m_initialNetAmount));
	if (query.exec())
	{
		int lastInsertedId = query.lastInsertId().toInt();
		QSqlQuery q;
		q.prepare("INSERT INTO cash (payment_id, amount) VALUES (?, ?)");
		q.addBindValue(lastInsertedId);
		q.addBindValue(QString::number(m_initialNetAmount));
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
	query.prepare("INSERT INTO payment (bill_id, total_amount, payment_type) VALUES (?, ?, 'CREDIT')");
	query.addBindValue(billId);
	query.addBindValue(QString::number(m_initialNetAmount));
	if (query.exec())
	{
		int lastInsertedId = query.lastInsertId().toInt();
		QSqlQuery q;
		q.prepare("INSERT INTO credit (payment_id, amount, interest, due_date) VALUES (?, ?, ?, ?)");
		q.addBindValue(lastInsertedId);
		q.addBindValue(QString::number(m_initialNetAmount));
		q.addBindValue(ui.lineEdit->text());
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
	query.prepare("INSERT INTO payment (bill_id, total_amount, payment_type) VALUES (?, ?, 'CHEQUE')");
	query.addBindValue(billId);
	query.addBindValue(QString::number(m_initialNetAmount));
	if (query.exec())
	{
		int lastInsertedId = query.lastInsertId().toInt();
		QSqlQuery q;
		q.prepare("INSERT INTO cheque (payment_id, amount, interest, cheque_number, bank, due_date) VALUES (?, ?, ?, ?, ?, ?)");
		q.addBindValue(lastInsertedId);
		q.addBindValue(QString::number(m_initialNetAmount));
		q.addBindValue(ui.lineEdit->text());
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

void ESSinglePayment::handleCreditCardPayment(int billId, double netAmount)
{

	QSqlQuery query;
	query.prepare("INSERT INTO payment (bill_id, total_amount, payment_type) VALUES (?, ?, 'CARD')");
	query.addBindValue(billId);
	query.addBindValue(QString::number(m_initialNetAmount));
	if (query.exec())
	{
		int lastInsertedId = query.lastInsertId().toInt();
		QSqlQuery q;
		q.prepare("INSERT INTO card (payment_id, amount, interest, card_no) VALUES (?, ?, ?, ?)");
		q.addBindValue(lastInsertedId);
		q.addBindValue(QString::number(m_initialNetAmount));
		q.addBindValue(ui.lineEdit->text());
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

void ESSinglePayment::handleLoyaltyPayment(int billId, double netAmount)
{

	QSqlQuery query;
	query.prepare("INSERT INTO payment (bill_id, total_amount, payment_type) VALUES (?, ?, 'LOYALTY')");
	query.addBindValue(billId);
	query.addBindValue(QString::number(m_initialNetAmount));
	if (query.exec())
	{
		int lastInsertedId = query.lastInsertId().toInt();
		QSqlQuery q;
		q.prepare("INSERT INTO loyalty (payment_id, amount, interest, card_no) VALUES (?, ?, ?, ?)");
		q.addBindValue(lastInsertedId);
		q.addBindValue(QString::number(m_initialNetAmount));
		q.addBindValue(ui.lineEdit->text());
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

void ESSinglePayment::slotPaymentMethodSelected()
{
	if (ui.creditBtn == sender())
	{
		m_paymentMethod = "CREDIT";
		ui.lbl1->hide();
		//ui.lbl2->show(); // interest
		ui.dateLbl->show(); // due date
		ui.label_6->hide();
		ui.balanceLbl->hide();
		ui.netAmountLbl->setText(QString::number(m_initialNetAmount, 'f', 2));
		ui.txt1->hide();
		//ui.txt2->show();
		ui.dateEdit->show();


		//ui.lbl2->setText("Interest % :  ");
		ui.paymentType->setText("Amount :  ");

		ui.cashBtn->setChecked(false);
		ui.chequeBtn->setChecked(false);
		ui.creditCardBtn->setChecked(false);
		ui.loyalityCardBtn->setChecked(false);

		ui.label_4->show();
		ui.lineEdit->show();
		ui.lineEdit->setText("0");//interest
		ui.interestPercentageCB->show();
	}
	else if (ui.chequeBtn == sender())
	{
		m_paymentMethod = "CHEQUE";
		ui.lbl1->show(); // cheque no
		ui.lbl2->show(); // bank
		ui.dateLbl->show(); // due date
		ui.label_6->hide();
		ui.balanceLbl->hide();
		ui.netAmountLbl->setText(QString::number(m_initialNetAmount, 'f', 2));

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

		ui.label_4->show();
		ui.lineEdit->show();
		ui.lineEdit->setText("0");//interest
		ui.interestPercentageCB->show();
	}
	else if (ui.creditCardBtn == sender())
	{
		m_paymentMethod = "CARD";
		ui.lbl1->show(); // card no
		ui.lbl2->hide();
		ui.dateLbl->hide();
		ui.label_6->hide();
		ui.balanceLbl->hide();

		ui.txt1->show();
		ui.txt2->hide();
		ui.dateEdit->hide();
		ui.netAmountLbl->setText(QString::number(m_initialNetAmount, 'f', 2));

		ui.lbl1->setText("Card No. :  ");
		ui.paymentType->setText("Amount :  ");

		ui.cashBtn->setChecked(false);
		ui.chequeBtn->setChecked(false);
		ui.creditBtn->setChecked(false);
		ui.loyalityCardBtn->setChecked(false);

		ui.label_4->show();
		ui.lineEdit->show();
		ui.lineEdit->setText("0");//interest
		ui.interestPercentageCB->show();
	}
	else if (ui.loyalityCardBtn == sender())
	{
		m_paymentMethod = "LOYALTY";
		ui.lbl1->show(); // card no
		ui.lbl2->hide();
		ui.dateLbl->hide();
		ui.label_6->hide();
		ui.balanceLbl->hide();
		ui.netAmountLbl->setText(QString::number(m_initialNetAmount, 'f', 2));

		ui.txt1->show();
		ui.txt2->hide();
		ui.dateEdit->hide();

		ui.lbl1->setText("Card No. :  ");
		ui.paymentType->setText("Amount :  ");

		ui.cashBtn->setChecked(false);
		ui.chequeBtn->setChecked(false);
		ui.creditCardBtn->setChecked(false);
		ui.creditBtn->setChecked(false);

		ui.label_4->show();
		ui.lineEdit->show();
		ui.lineEdit->setText("0");//interest
		ui.interestPercentageCB->show();
	}
	else if (ui.cashBtn == sender())
	{
		m_paymentMethod = "CASH";
		ui.lbl1->hide();
		ui.lbl2->hide();
		ui.dateLbl->hide();
		ui.label_6->show();
		ui.balanceLbl->show();

		ui.txt1->hide();
		ui.txt2->hide();
		ui.dateEdit->hide();
		ui.netAmountLbl->setText(QString::number(m_initialNetAmount, 'f', 2));

		ui.paymentType->setText("Cash :  ");

		ui.creditBtn->setChecked(false);
		ui.chequeBtn->setChecked(false);
		ui.creditCardBtn->setChecked(false);
		ui.loyalityCardBtn->setChecked(false);

		ui.label_4->hide();
		ui.lineEdit->hide();
		ui.interestPercentageCB->hide();
	}

	//resize(400, 1);
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
	qq.addBindValue(QString::number(m_initialNetAmount));
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
			printBill(billId, netAmount);
		}
	}
	this->close();
}

void ESSinglePayment::slotInterestChanged()
{
	double interest = ui.lineEdit->text().toDouble();
	double netAmout = ui.netAmountLbl->text().toDouble();

	double totalBill = netAmout + netAmout * (interest / 100.0);

	ui.netAmountLbl->setText(QString::number(totalBill, 'f', 2));
}

void ESSinglePayment::printBill(int billId, float total)
{
	QString billStr("SELECT user_id FROM bill WHERE bill_id = " + QString::number(billId));
	QString userName = "";
	QSqlQuery queryBill(billStr);
	if (queryBill.next())
	{
		QString userId = queryBill.value("user_id").toString();
		QString qStrUser("SELECT display_name FROM user WHERE user_id = " + userId);
		QSqlQuery queryUser(qStrUser);
		if (queryUser.next())
		{
			userName = queryUser.value("display_name").toString();
		}
	}

	QString payamentStr;
	QSqlQuery queryPaymentType("SELECT * FROM payment WHERE bill_id = " + QString::number(billId));
	QString paymentTypes = "";
	float totalPayingAmount = 0;
	while (queryPaymentType.next())
	{
		//paymentTypes.append(queryPaymentType.value("payment_type").toString());
		QString paymentType = queryPaymentType.value("payment_type").toString();
		QString paymentId = queryPaymentType.value("payment_id").toString();
		if (paymentType == "CARD")
		{
			QSqlQuery queryCard("SELECT * FROM card WHERE payment_id = " + paymentId);
			if (queryCard.next())
			{
				float interest = queryCard.value("interest").toFloat();
				float amount = queryCard.value("amount").toFloat();
				total = amount;
				amount = amount + (amount * interest) / 100;
				totalPayingAmount += amount;
				payamentStr.append(paymentType + "(" + QString::number(total, 'f', 2) + " +" + QString::number(interest, 'f', 2) + "%): " + QString::number(amount, 'f', 2));
			}
		}
		else if (paymentType == "CHEQUE")
		{
			QSqlQuery query("SELECT * FROM cheque WHERE payment_id = " + paymentId);
			if (query.next())
			{
				float interest = query.value("interest").toFloat();
				float amount = query.value("amount").toFloat();
				total = amount;
				amount = amount + (amount * interest) / 100;
				totalPayingAmount += amount;
				payamentStr.append(paymentType + "(" + QString::number(total, 'f', 2) + " +" + QString::number(interest, 'f', 2) + "%): " + QString::number(amount, 'f', 2));
			}
		}
		else if (paymentType == "CREDIT")
		{
			QSqlQuery query("SELECT * FROM credit WHERE payment_id = " + paymentId);
			if (query.next())
			{
				float interest = query.value("interest").toFloat();
				float amount = query.value("amount").toFloat();
				total = amount;
				amount = amount + (amount * interest) / 100;
				totalPayingAmount += amount;
				payamentStr.append(paymentType + "(" + QString::number(total, 'f', 2) + " +" + QString::number(interest, 'f', 2) + "%): " + QString::number(amount, 'f', 2));
			}
		}
		else if (paymentType == "CASH")
		{
			QSqlQuery query("SELECT * FROM cash WHERE payment_id = " + paymentId);
			if (query.next())
			{
				float amount = query.value("amount").toFloat();
				total = amount;
				totalPayingAmount += amount;
				payamentStr.append(paymentType + " : " + QString::number(amount, 'f', 2));
			}
		}
		else if (paymentType == "LOYALTY")
		{
		}
	}

	KDReports::Report report;

	QString dateStr = "Date : ";
	dateStr.append(QDateTime::currentDateTime().toString("yyyy-MM-dd"));
	QString timeStr = "Time : ";
	timeStr.append(QDateTime::currentDateTime().toString("hh : mm"));
	QString billIdStr("Bill No : " + QString::number(billId));
	/*
	PUJITHA ENTERPRISES (PVT) LTD
	No. 154, Kurugala, Padukka
	Phone :  077-4784430 / 077-4784437
	email :rapprasanna4@gmail.com


	HIRUNA MARKETING (PVT) LTD
	No.374, High level Road, Meegoda
	*/
	KDReports::TextElement titleElement("PUJITHA ENTERPRISES (PVT) LTD");
	titleElement.setPointSize(14);
	titleElement.setBold(true);
	report.addElement(titleElement, Qt::AlignHCenter);

	KDReports::TextElement addressElement("No. 154, Kurugala, Padukka.");
	addressElement.setPointSize(10);
	addressElement.setBold(false);
	report.addElement(addressElement, Qt::AlignHCenter);

	KDReports::TextElement telElement("Phone :  077-4784430 / 077-4784437");
	telElement.setPointSize(10);
	telElement.setBold(false);
	report.addElement(telElement, Qt::AlignHCenter);

	KDReports::TextElement emailElement("email : rapprasanna4@gmail.com");
	emailElement.setPointSize(10);
	emailElement.setBold(false);
	report.addElement(emailElement, Qt::AlignHCenter);

	KDReports::TableElement infoTableElement;
	infoTableElement.setHeaderRowCount(2);
	infoTableElement.setHeaderColumnCount(2);
	infoTableElement.setBorder(0);
	infoTableElement.setWidth(100, KDReports::Percent);
	
	{
		KDReports::Cell& billIdCell = infoTableElement.cell(0, 0);
		KDReports::TextElement t(billIdStr);
		t.setPointSize(10);
		billIdCell.addElement(t, Qt::AlignLeft);
	}{
		KDReports::Cell& userNameCell = infoTableElement.cell(1, 0);
		KDReports::TextElement t("Cashier : " + userName);
		t.setPointSize(10);
		userNameCell.addElement(t, Qt::AlignLeft);
	}{
		KDReports::Cell& dateCell = infoTableElement.cell(0, 1);
		KDReports::TextElement t(dateStr);
		t.setPointSize(10);
		dateCell.addElement(t, Qt::AlignRight);
	}{
		KDReports::Cell& timeCell = infoTableElement.cell(1, 1);
		KDReports::TextElement t(timeStr);
		t.setPointSize(10);
		timeCell.addElement(t, Qt::AlignRight);
	}
	
	report.addElement(infoTableElement);

	QString querySaleStr("SELECT * FROM sale WHERE bill_id = " + QString::number(billId) + " AND deleted = 0");
	QSqlQuery querySale(querySaleStr);

	//columns (item_code, Description, UnitPrice, Discount, Qty, Sub Total)

	KDReports::TableElement tableElement;
	//tableElement.setHeaderRowCount(5);
	tableElement.setHeaderColumnCount(6);
	tableElement.setBorder(0);
	tableElement.setWidth(100, KDReports::Percent);

	KDReports::Cell& cICode = tableElement.cell(0, 0);
	KDReports::TextElement tICode("Item Code");
	tICode.setPointSize(11);
	tICode.setBold(true);
	cICode.addElement(tICode, Qt::AlignLeft);

	KDReports::Cell& cIName = tableElement.cell(0, 1);
	KDReports::TextElement tEItem("Item");
	tEItem.setBold(true);
	tEItem.setPointSize(11);
	cIName.addElement(tEItem, Qt::AlignLeft);

	KDReports::Cell& cPrice = tableElement.cell(0, 2);
	KDReports::TextElement tEPrice("Unit Price");
	tEPrice.setPointSize(11);
	tEPrice.setBold(true);
	cPrice.addElement(tEPrice, Qt::AlignRight);

	KDReports::Cell& cDiscount = tableElement.cell(0, 3);
	KDReports::TextElement tEDiscount("Discount");
	tEDiscount.setPointSize(11);
	tEDiscount.setBold(true);
	cDiscount.addElement(tEDiscount, Qt::AlignRight);

	KDReports::Cell& cQty = tableElement.cell(0, 4);
	KDReports::TextElement tEQty("Qty");
	tEQty.setPointSize(11);
	tEQty.setBold(true);
	cQty.addElement(tEQty, Qt::AlignRight);

	KDReports::Cell& cTotal = tableElement.cell(0, 5);
	KDReports::TextElement tETotal("Line Total");
	tETotal.setPointSize(11);
	tETotal.setBold(true);
	cTotal.addElement(tETotal, Qt::AlignRight);

//	report.addVerticalSpacing(6);
	KDReports::HtmlElement htmlElem;
	QString htm("<div><hr/></div>");
	htmlElem.setHtml(htm);
	report.addElement(htmlElem);

	int row = 1;
	int noOfPcs = 0, noOfItems = 0;
	while (querySale.next())
	{
		QString stockId = querySale.value("stock_id").toString();
		QString discount = QString::number(querySale.value("discount").toDouble(), 'f', 2);
		QString qty = querySale.value("quantity").toString();
		noOfPcs += qty.toInt();
		noOfItems++;
		QString subTotal = QString::number(querySale.value("total").toDouble(), 'f', 2);
		QString itemName = "";
		QString unitPrice = "";
		QString itemCode = "";

		//get the item name from the item table
		QString qItemStr("SELECT it.item_code, it.item_name , st.selling_price FROM stock st JOIN item it ON st.item_id = it.item_id AND st.stock_id = " + stockId);
		QSqlQuery queryItem(qItemStr);
		if (queryItem.next())
		{
			itemName = queryItem.value("item_name").toString();
			unitPrice = QString::number(queryItem.value("selling_price").toDouble(), 'f', 2);
			itemCode = queryItem.value("item_code").toString();
		}
		//columns (item_code, Description, UnitPrice, Discount, Qty, Sub Total)
		printRow(tableElement, row, 0, itemCode);
		printRow(tableElement, row, 1, itemName);
		printRow(tableElement, row, 2, unitPrice, Qt::AlignRight);
		printRow(tableElement, row, 3, discount, Qt::AlignRight);
		printRow(tableElement, row, 4, qty, Qt::AlignRight);
		printRow(tableElement, row, 5, subTotal, Qt::AlignRight);
		row++;
	}

	//report.addVerticalSpacing(5);
	{
		KDReports::Cell& emptyCell = tableElement.cell(row, 0);
		KDReports::HtmlElement htmlElem;
		QString html("<div><hr/></div>");
		htmlElem.setHtml(html);
		emptyCell.setColumnSpan(6);
		emptyCell.addElement(htmlElem);
		row++;
	}
	

	KDReports::Cell& totalTextC = tableElement.cell(row, 0);
	totalTextC.setColumnSpan(5);
	KDReports::TextElement totalTxt("Sub Total ");
	totalTxt.setPointSize(11);
	totalTxt.setBold(true);
	totalTextC.addElement(totalTxt, Qt::AlignRight);

	KDReports::Cell& totalCell = tableElement.cell(row, 5);
	KDReports::TextElement totalValue(QString::number(total, 'f', 2));
	totalValue.setPointSize(11);
	totalValue.setBold(true);
	totalCell.addElement(totalValue, Qt::AlignRight);

	row++;
	KDReports::Cell& payableTextC = tableElement.cell(row, 0);
	payableTextC.setColumnSpan(5);
	KDReports::TextElement payableTxt("Total ");
	payableTxt.setPointSize(11);
	payableTxt.setBold(true);
	payableTextC.addElement(payableTxt, Qt::AlignRight);

	KDReports::Cell& payableCell = tableElement.cell(row, 5);
	KDReports::TextElement payableValue(QString::number(totalPayingAmount, 'f', 2));
	payableValue.setPointSize(11);
	payableValue.setBold(true);
	payableCell.addElement(payableValue, Qt::AlignRight);
	//
	row++;
	KDReports::Cell& countText = tableElement.cell(row, 0);
	countText.setColumnSpan(5);
	KDReports::TextElement noOfItemsTxt("# of Items ");
	noOfItemsTxt.setPointSize(11);
	noOfItemsTxt.setBold(true);
	countText.addElement(noOfItemsTxt, Qt::AlignRight);

	KDReports::Cell& countItemCell = tableElement.cell(row, 5);
	KDReports::TextElement itemCountValue(QString::number(noOfItems));
	itemCountValue.setPointSize(11);
	itemCountValue.setBold(true);
	countItemCell.addElement(itemCountValue, Qt::AlignRight);

	//
	row++;
	KDReports::Cell& pcsText = tableElement.cell(row, 0);
	pcsText.setColumnSpan(5);
	KDReports::TextElement noOfPcsTxt("# of Pieces ");
	noOfPcsTxt.setPointSize(11);
	noOfPcsTxt.setBold(true);
	pcsText.addElement(noOfPcsTxt, Qt::AlignRight);

	KDReports::Cell& pcsItemCell = tableElement.cell(row, 5);
	KDReports::TextElement itemPcsValue(QString::number(noOfPcs));
	itemPcsValue.setPointSize(11);
	itemPcsValue.setBold(true);
	pcsItemCell.addElement(itemPcsValue, Qt::AlignRight);
	//
	report.addElement(tableElement);

	report.addVerticalSpacing(5);

	KDReports::TextElement customerInfo("Payment Info : " + payamentStr);
	customerInfo.setPointSize(11);
	report.addElement(customerInfo, Qt::AlignLeft);

	report.addVerticalSpacing(1);

	// customer info	
	if (m_customerId == "-1")
	{
		QString customer = "Bill To : N/A";

		KDReports::TextElement customerInfo(customer);
		customerInfo.setPointSize(11);
		report.addElement(customerInfo, Qt::AlignLeft);
	}
	else
	{
		QString customer = "Bill To : ";
		QSqlQuery q("SELECT * FROM customer WHERE customer_id = " + m_customerId);
		if (q.next())
		{
			customer.append(q.value("customer_id").toString());
			customer.append(" / ");
			customer.append(q.value("name").toString());
		}
		KDReports::TextElement customerInfo(customer);
		customerInfo.setPointSize(11);
		report.addElement(customerInfo, Qt::AlignLeft);
	}

// 	KDReports::Footer& foter = report.footer();
// 	KDReports::TextElement info("Powered by PROGEX Technologies.");
// 	KDReports::TextElement web("www.progextech.com  T.P.: 072-6430268/071-1308531");
// 	foter.addElement(info, Qt::AlignCenter);
// 	foter.addElement(web, Qt::AlignCenter);
	KDReports::TextElement poweredBy("Powered by PROGEX Technologies.");
	KDReports::TextElement web("www.progextech.com  T.P.: 072-6430268/071-1308531");
	report.addElement(poweredBy, Qt::AlignCenter);
	report.addElement(web, Qt::AlignCenter);

	QPrinter printer;
	printer.setPaperSize(QPrinter::A4);

	printer.setFullPage(false);
	printer.setOrientation(QPrinter::Portrait);

// 	 	  		QPrintPreviewDialog *dialog = new QPrintPreviewDialog(&printer, this);
// 	 	  		QObject::connect(dialog, SIGNAL(paintRequested(QPrinter*)), this, SLOT(slotPrint(QPrinter*)));
// 	 	  		dialog->setWindowTitle(tr("Print Document"));
// 	 	  		ES::MainWindowHolder::instance()->getMainWindow()->setCentralWidget(dialog);
// 	 	  		dialog->exec();

	report.print(&printer);
}

void ESSinglePayment::slotPrint(QPrinter* printer)
{
	//report.print(printer);
	this->close();
}

void ESSinglePayment::printRow(KDReports::TableElement& tableElement, int row, int col, QString elementStr, Qt::AlignmentFlag alignment)
{
	KDReports::Cell& cell = tableElement.cell(row, col);
	KDReports::TextElement te(elementStr);
	te.setPointSize(10);
	cell.addElement(te, alignment);
}

void ESSinglePayment::keyPressEvent(QKeyEvent * event)
{
	switch (event->key())
	{
	case Qt::Key_Return:
	case Qt::Key_Enter:
	{
						  slotFinalizeBill();
						  break;
	}
	default:
		QWidget::keyPressEvent(event);
	}
}

float ESSinglePayment::getInitialNetAmount() const
{
	return m_initialNetAmount;
}

void ESSinglePayment::setInitialNetAmount(float val)
{
	m_initialNetAmount = val;
}

void ESSinglePayment::slotDiscountPercentage()
{
	QString discountStr = ui.discountText->text();
	double discount = 0;
	if (!discountStr.isEmpty())
	{
		bool valid = false;
		discount = discountStr.toDouble(&valid);
		if (!valid)
		{
			QMessageBox mbox;
			mbox.setIcon(QMessageBox::Warning);
			mbox.setText(QString("Discount should be a number"));
			mbox.exec();
			return;
		}
	}
	double netAmount = ui.netAmountLbl->text().toDouble();
	if (ui.percentageCB->isChecked())
	{
		netAmount = netAmount - (netAmount * discount / 100);
	}
	else
	{
		netAmount = netAmount - discount;
	}
	ui.netAmountLbl->setText(QString::number(netAmount, 'f', 2));
}
