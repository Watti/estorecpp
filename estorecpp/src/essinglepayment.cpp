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
#include <memory>
#include "utility\utility.h"

ESSinglePayment::ESSinglePayment(ESAddBill* addBill, QWidget *parent /*= 0*/, bool isReturnBill) : 
QWidget(parent), m_addBill(addBill), m_isReturnBill(isReturnBill)
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
			mbox.setText(QString("Failed to insert cash amount to the database"));
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
			mbox.setText(QString("Failed to add CREDIT info"));
			mbox.exec();
		}
		else
		{
			finishBill(netAmount, billId);

			double outstandingAmount = ES::Utility::getTotalCreditOutstanding(m_customerId);
			outstandingAmount += netAmount;

			ES::Utility::updateOutstandingAmount(m_customerId, netAmount);
		}
	}
	else
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Critical);
		mbox.setText(QString("Failed to add PAYMENT info"));
		mbox.exec();
	}
}

void ESSinglePayment::handleChequePayment(int billId, double netAmount)
{
	QString initialNetAmount = QString::number(m_initialNetAmount);
	float interest = ui.lineEdit->text().toFloat();
	QString totalChequeOutstanding = QString::number(m_initialNetAmount* (100+interest)/100);
	QString chequeNo = ui.txt1->text();
	QString bank = ui.txt2->text();
	QString dueDate = ui.dateEdit->text();
	QSqlQuery query;
	query.prepare("INSERT INTO payment (bill_id, total_amount, payment_type) VALUES (?, ?, 'CHEQUE')");
	query.addBindValue(billId);
	query.addBindValue(initialNetAmount);

	if (query.exec())
	{
		int lastInsertedId = query.lastInsertId().toInt();
		QSqlQuery q;
		q.prepare("INSERT INTO cheque (payment_id, amount, interest, cheque_number, bank, due_date) VALUES (?, ?, ?, ?, ?, ?)");
		q.addBindValue(lastInsertedId);
		q.addBindValue(initialNetAmount);
		q.addBindValue(ui.lineEdit->text());
		q.addBindValue(chequeNo);
		q.addBindValue(bank);
		q.addBindValue(dueDate);
		if (!q.exec())
		{
			QMessageBox mbox;
			mbox.setIcon(QMessageBox::Critical);
			mbox.setText(QString("Failed to add CHEQUE info"));
			mbox.exec();
		}
		else
		{
			float chequeAmount = initialNetAmount.toFloat() * ((100 + interest) / 100);
			QString qStr("INSERT INTO cheque_information (customer_id, cheque_number, bank, due_date, amount) VALUES (" + 
				m_customerId + ",'" + chequeNo + "','" + bank + "','" + dueDate +"', '"+ QString::number(chequeAmount)+"')");
			QSqlQuery outstandingQry;
			if (!outstandingQry.exec(qStr))
			{
				QMessageBox mbox;
				mbox.setIcon(QMessageBox::Critical);
				mbox.setText(QString("Failed to add cheque information"));
				mbox.exec();
			}
			finishBill(netAmount, billId);
		}
	}
	else
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Critical);
		mbox.setText(QString("Failed to add PAYMENT info"));
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
	qq.addBindValue(QString::number(netAmount));
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
		saleQuantityQuery.prepare("SELECT * FROM sale WHERE deleted=0 AND bill_id = ?");
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

		if (m_addBill)
		{
			m_addBill->resetBill();
		}

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
	double netAmout = m_initialNetAmount;

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

	struct PaymentSummaryElement
	{
		QString type, amount, no, date, interest, netAmount;
	};
	QVector<std::shared_ptr<PaymentSummaryElement>> payamentSummaryTableInfo;

	QSqlQuery queryPaymentType("SELECT * FROM payment WHERE bill_id = " + QString::number(billId));
	QString paymentTypes = "";
	float totalPayingAmount = 0;
	while (queryPaymentType.next())
	{
		QString paymentType = queryPaymentType.value("payment_type").toString();
		QString paymentId = queryPaymentType.value("payment_id").toString();
		if (paymentType == "CARD")
		{
			QSqlQuery queryCard("SELECT * FROM card WHERE payment_id = " + paymentId);
			if (queryCard.next())
			{
				float interest = queryCard.value("interest").toFloat();
				float amount = queryCard.value("amount").toFloat();
				float netAmount = amount;
				amount = amount + (amount * interest) / 100;
				totalPayingAmount += amount;
				std::shared_ptr<PaymentSummaryElement> pse = std::make_shared<PaymentSummaryElement>();
				pse->type = paymentType;
				pse->no = queryCard.value("card_no").toString();
				pse->date = "-";
				pse->amount = QString::number(amount, 'f', 2);
				pse->interest = QString::number(interest, 'f', 2) + "%";
				pse->netAmount = QString::number(netAmount, 'f', 2);
				payamentSummaryTableInfo.push_back(pse);
			}
		}
		else if (paymentType == "CHEQUE")
		{
			QSqlQuery query("SELECT * FROM cheque WHERE payment_id = " + paymentId);
			if (query.next())
			{
				float interest = query.value("interest").toFloat();
				float amount = query.value("amount").toFloat();
				float netAmount = amount;
				amount = amount + (amount * interest) / 100;
				totalPayingAmount += amount;

				std::shared_ptr<PaymentSummaryElement> pse = std::make_shared<PaymentSummaryElement>();
				pse->type = paymentType;
				pse->no = query.value("cheque_number").toString();
				pse->amount = QString::number(amount, 'f', 2);
				pse->date = query.value("due_date").toString();
				pse->interest = QString::number(interest, 'f', 2) + "%";
				pse->netAmount = QString::number(netAmount, 'f', 2);
				payamentSummaryTableInfo.push_back(pse);
			}
		}
		else if (paymentType == "CREDIT")
		{
			QSqlQuery query("SELECT * FROM credit WHERE payment_id = " + paymentId);
			if (query.next())
			{
				float interest = query.value("interest").toFloat();
				float amount = query.value("amount").toFloat();
				float netAmount = amount;
				amount = amount + (amount * interest) / 100;
				totalPayingAmount += amount;

				std::shared_ptr<PaymentSummaryElement> pse = std::make_shared<PaymentSummaryElement>();
				pse->type = paymentType;
				pse->date = query.value("due_date").toString();
				pse->no = "-";
				pse->amount = QString::number(amount, 'f', 2);
				payamentSummaryTableInfo.push_back(pse);
				pse->interest = QString::number(interest, 'f', 2) + "%";
				pse->netAmount = QString::number(netAmount, 'f', 2);
			}
		}
		else if (paymentType == "CASH")
		{
			QSqlQuery query("SELECT * FROM cash WHERE payment_id = " + paymentId);
			if (query.next())
			{
				float amount = query.value("amount").toFloat();
				totalPayingAmount += amount;

				std::shared_ptr<PaymentSummaryElement> pse = std::make_shared<PaymentSummaryElement>();
				pse->type = paymentType;
				pse->date = "-";
				pse->no = "-";
				pse->amount = QString::number(amount, 'f', 2);
				pse->interest = "0%";
				pse->netAmount = QString::number(amount, 'f', 2);
				payamentSummaryTableInfo.push_back(pse);
			}
		}
		else if (paymentType == "LOYALTY")
		{
		}
	}

	KDReports::Report report;

	bool secondDisplayOn = ES::Session::getInstance()->isSecondDisplayOn();

	QString dateStr = "Date : ";
	dateStr.append(QDateTime::currentDateTime().toString("yyyy-MM-dd"));
	QString timeStr = "Time : ";
	timeStr.append(QDateTime::currentDateTime().toString("hh : mm"));
	QString billIdStr("Bill No : " + QString::number(billId));
	if (secondDisplayOn)
	{
		billIdStr = "Bill No : " + QString::number(billId % 10000);
	}

	KDReports::TableElement infoTableElement;
	infoTableElement.setHeaderRowCount(3);
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
		KDReports::TextElement t("Customer : " + ui.nameText->text());
		t.setPointSize(10);
		userNameCell.addElement(t, Qt::AlignLeft);
	}{
		KDReports::Cell& userNameCell = infoTableElement.cell(2, 0);
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

	KDReports::HtmlElement htmlElem;
	QString htm("<div><hr/></div>");
	htmlElem.setHtml(htm);
	report.addElement(htmlElem);


	QString querySaleStr("SELECT * FROM sale WHERE bill_id = " + QString::number(billId) + " AND deleted = 0");
	QSqlQuery querySale(querySaleStr);

	KDReports::TableElement tableElement;
	tableElement.setHeaderColumnCount(6);
	if (secondDisplayOn)
	{
		tableElement.setHeaderColumnCount(5);
	}
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

	if (secondDisplayOn)
	{
		KDReports::Cell& cQty = tableElement.cell(0, 3);
		KDReports::TextElement tEQty("Qty");
		tEQty.setPointSize(11);
		tEQty.setBold(true);
		cQty.addElement(tEQty, Qt::AlignRight);

		KDReports::Cell& cTotal = tableElement.cell(0, 4);
		KDReports::TextElement tETotal("Line Total");
		tETotal.setPointSize(11);
		tETotal.setBold(true);
		cTotal.addElement(tETotal, Qt::AlignRight);
	}
	else
	{
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
	}


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
			unitPrice = QString::number(querySale.value("item_price").toDouble(), 'f', 2);
			itemCode = queryItem.value("item_code").toString();
		}
		//columns (item_code, Description, UnitPrice, Discount, Qty, Sub Total)
		printRow(tableElement, row, 0, itemCode);
		printRow(tableElement, row, 1, itemName);
		printRow(tableElement, row, 2, unitPrice, Qt::AlignRight);
		if (secondDisplayOn)
		{
			printRow(tableElement, row, 3, qty, Qt::AlignRight);
			printRow(tableElement, row, 4, subTotal, Qt::AlignRight);
		}
		else
		{
			printRow(tableElement, row, 3, discount, Qt::AlignRight);
			printRow(tableElement, row, 4, qty, Qt::AlignRight);
			printRow(tableElement, row, 5, subTotal, Qt::AlignRight);
		}
		row++;
	}

	{
		KDReports::Cell& emptyCell = tableElement.cell(row, 0);
		KDReports::HtmlElement htmlElem;
		QString html("<div><hr/></div>");
		htmlElem.setHtml(html);
		emptyCell.setColumnSpan(6);
		emptyCell.addElement(htmlElem);
		row++;
	}

	// customer info	
	if (m_customerId == "-1")
	{
		QString customer = "Customer Id";

		KDReports::Cell& billToCell = tableElement.cell(row, 0);
		KDReports::TextElement billTo(customer);
		billTo.setPointSize(11);
		billToCell.addElement(billTo, Qt::AlignLeft);
	}
	else
	{
		QString customer = "Customer Id";
		QSqlQuery q("SELECT * FROM customer WHERE customer_id = " + m_customerId);

		KDReports::Cell& billToCell = tableElement.cell(row, 0);
		KDReports::TextElement billTo(customer);
		billTo.setPointSize(10);
		billToCell.addElement(billTo, Qt::AlignLeft);
		if (q.next())
		{
			KDReports::Cell& cell = tableElement.cell(row, 1);
			KDReports::TextElement te(q.value("customer_id").toString());
			te.setPointSize(10);
			te.setBold(false);
			cell.addElement(te, Qt::AlignLeft);
		}
	}

	if (secondDisplayOn)
	{
		KDReports::Cell& payableTextC = tableElement.cell(row, 3);
		KDReports::TextElement payableTxt("Total ");
		payableTxt.setPointSize(11);
		payableTxt.setBold(true);
		payableTextC.addElement(payableTxt, Qt::AlignRight);

		KDReports::Cell& payableCell = tableElement.cell(row, 4);
		KDReports::TextElement payableValue(QString::number(totalPayingAmount, 'f', 2));
		payableValue.setPointSize(11);
		payableValue.setBold(true);
		payableCell.addElement(payableValue, Qt::AlignRight);
	}
	else
	{

		KDReports::Cell& payableTextC = tableElement.cell(row, 4);
		KDReports::TextElement payableTxt("Total ");
		payableTxt.setPointSize(11);
		payableTxt.setBold(true);
		payableTextC.addElement(payableTxt, Qt::AlignRight);

		KDReports::Cell& payableCell = tableElement.cell(row, 5);
		KDReports::TextElement payableValue(QString::number(totalPayingAmount, 'f', 2));
		payableValue.setPointSize(11);
		payableValue.setBold(true);
		payableCell.addElement(payableValue, Qt::AlignRight);
	}
	row++;

	double prevOutstanding = ui.outstandingText->text().toDouble();
	{
		KDReports::Cell& cell = tableElement.cell(row, 0);
		KDReports::TextElement te("Prev. Due");
		te.setPointSize(10);
		te.setBold(false);
		cell.addElement(te, Qt::AlignLeft);
	}
	{
		KDReports::Cell& cell = tableElement.cell(row, 1);
		KDReports::TextElement te(QString::number(prevOutstanding, 'f', 2));
		te.setPointSize(10);
		te.setBold(false);
		cell.addElement(te, Qt::AlignLeft);
	}

	if (secondDisplayOn)
	{
		KDReports::Cell& countText = tableElement.cell(row, 3);
		KDReports::TextElement noOfItemsTxt("# of Items ");
		noOfItemsTxt.setPointSize(11);
		noOfItemsTxt.setBold(true);
		countText.addElement(noOfItemsTxt, Qt::AlignRight);

		KDReports::Cell& countItemCell = tableElement.cell(row, 4);
		KDReports::TextElement itemCountValue(QString::number(noOfItems));
		itemCountValue.setPointSize(11);
		itemCountValue.setBold(true);
		countItemCell.addElement(itemCountValue, Qt::AlignRight);
	}
	else
	{
		KDReports::Cell& countText = tableElement.cell(row, 4);
		KDReports::TextElement noOfItemsTxt("# of Items ");
		noOfItemsTxt.setPointSize(11);
		noOfItemsTxt.setBold(true);
		countText.addElement(noOfItemsTxt, Qt::AlignRight);

		KDReports::Cell& countItemCell = tableElement.cell(row, 5);
		KDReports::TextElement itemCountValue(QString::number(noOfItems));
		itemCountValue.setPointSize(11);
		itemCountValue.setBold(true);
		countItemCell.addElement(itemCountValue, Qt::AlignRight);
	}

	row++;

	{
		KDReports::Cell& cell = tableElement.cell(row, 0);
		KDReports::TextElement te("Total Due");
		te.setPointSize(10);
		te.setBold(false);
		cell.addElement(te, Qt::AlignLeft);
	}
	{
		float billOutstanding = ES::Utility::getOutstandingForBill(billId);
		double totalOutstanding = prevOutstanding + billOutstanding;
		KDReports::Cell& cell = tableElement.cell(row, 1);
		KDReports::TextElement te(QString::number(totalOutstanding, 'f', 2));
		te.setPointSize(10);
		te.setBold(false);
		cell.addElement(te, Qt::AlignLeft);
	}

	if (secondDisplayOn)
	{
		KDReports::Cell& pcsText = tableElement.cell(row, 3);
		KDReports::TextElement noOfPcsTxt("# of Pieces ");
		noOfPcsTxt.setPointSize(11);
		noOfPcsTxt.setBold(true);
		pcsText.addElement(noOfPcsTxt, Qt::AlignRight);

		KDReports::Cell& pcsItemCell = tableElement.cell(row, 4);
		KDReports::TextElement itemPcsValue(QString::number(noOfPcs));
		itemPcsValue.setPointSize(11);
		itemPcsValue.setBold(true);
		pcsItemCell.addElement(itemPcsValue, Qt::AlignRight);
	}
	else
	{
		KDReports::Cell& pcsText = tableElement.cell(row, 4);
		KDReports::TextElement noOfPcsTxt("# of Pieces ");
		noOfPcsTxt.setPointSize(11);
		noOfPcsTxt.setBold(true);
		pcsText.addElement(noOfPcsTxt, Qt::AlignRight);

		KDReports::Cell& pcsItemCell = tableElement.cell(row, 5);
		KDReports::TextElement itemPcsValue(QString::number(noOfPcs));
		itemPcsValue.setPointSize(11);
		itemPcsValue.setBold(true);
		pcsItemCell.addElement(itemPcsValue, Qt::AlignRight);

	}

	report.addElement(tableElement);
	report.addVerticalSpacing(1);

	if (!secondDisplayOn)
	{
		KDReports::TableElement paymentSummaryElement;
		paymentSummaryElement.setHeaderRowCount(payamentSummaryTableInfo.size());
		paymentSummaryElement.setHeaderColumnCount(6);
		paymentSummaryElement.setBorder(1);
		paymentSummaryElement.setWidth(100, KDReports::Percent);
		int pointSizeForPayement = 9;
		{
			KDReports::Cell& cell = paymentSummaryElement.cell(0, 0);
			KDReports::TextElement textElm("Type");
			textElm.setPointSize(pointSizeForPayement);
			textElm.setBold(true);
			cell.addElement(textElm, Qt::AlignCenter);
		}
		{
			KDReports::Cell& cell = paymentSummaryElement.cell(0, 1);
			KDReports::TextElement textElm("Net Amount");
			textElm.setPointSize(pointSizeForPayement);
			textElm.setBold(true);
			cell.addElement(textElm, Qt::AlignCenter);
		}
		{
			KDReports::Cell& cell = paymentSummaryElement.cell(0, 2);
			KDReports::TextElement textElm("Interest");
			textElm.setPointSize(pointSizeForPayement);
			textElm.setBold(true);
			cell.addElement(textElm, Qt::AlignCenter);
		}
		{
			KDReports::Cell& cell = paymentSummaryElement.cell(0, 3);
			KDReports::TextElement textElm("Line Total");
			textElm.setPointSize(pointSizeForPayement);
			textElm.setBold(true);
			cell.addElement(textElm, Qt::AlignCenter);
		}
		{
			KDReports::Cell& cell = paymentSummaryElement.cell(0, 4);
			KDReports::TextElement textElm("Cheque/Card No");
			textElm.setPointSize(pointSizeForPayement);
			textElm.setBold(true);
			cell.addElement(textElm, Qt::AlignCenter);
		}
		{
			KDReports::Cell& cell = paymentSummaryElement.cell(0, 5);
			KDReports::TextElement textElm("Payment Date");
			textElm.setPointSize(pointSizeForPayement);
			textElm.setBold(true);
			cell.addElement(textElm, Qt::AlignCenter);
		}
		int count = 1;
		for (std::shared_ptr<PaymentSummaryElement> pse : payamentSummaryTableInfo)
		{
			{
				KDReports::Cell& cell = paymentSummaryElement.cell(count, 0);
				KDReports::TextElement textElm(pse->type);
				textElm.setPointSize(pointSizeForPayement);
				textElm.setBold(false);
				cell.addElement(textElm, Qt::AlignLeft);
			}
			{
			KDReports::Cell& cell = paymentSummaryElement.cell(count, 1);
			KDReports::TextElement textElm(pse->netAmount);
			textElm.setPointSize(pointSizeForPayement);
			textElm.setBold(false);
			cell.addElement(textElm, Qt::AlignRight);
		}
			{
				KDReports::Cell& cell = paymentSummaryElement.cell(count, 2);
				KDReports::TextElement textElm(pse->interest);
				textElm.setPointSize(pointSizeForPayement);
				textElm.setBold(false);
				cell.addElement(textElm, Qt::AlignRight);
			}
			{
				KDReports::Cell& cell = paymentSummaryElement.cell(count, 3);
				KDReports::TextElement textElm(pse->amount);
				textElm.setPointSize(pointSizeForPayement);
				textElm.setBold(false);
				cell.addElement(textElm, Qt::AlignRight);
			}
			{
				KDReports::Cell& cell = paymentSummaryElement.cell(count, 4);
				KDReports::TextElement textElm(pse->no);
				textElm.setPointSize(pointSizeForPayement);
				textElm.setBold(false);
				cell.addElement(textElm, Qt::AlignLeft);
			}
			{
				KDReports::Cell& cell = paymentSummaryElement.cell(count, 5);
				KDReports::TextElement textElm(pse->date);
				textElm.setPointSize(pointSizeForPayement);
				textElm.setBold(false);
				cell.addElement(textElm, Qt::AlignLeft);
			}
			count++;
		}
		report.addElement(paymentSummaryElement);
	}

	report.addVerticalSpacing(1);

	KDReports::TextElement customerInfo2("Thank You!");
	customerInfo2.setPointSize(11);
	report.addElement(customerInfo2, Qt::AlignCenter);

	QPrinter printer;
	printer.setPaperSize(QPrinter::Custom);

	printer.setFullPage(false);
	printer.setOrientation(QPrinter::Portrait);

	//preview start
// 	QPrintPreviewDialog *dialog = new QPrintPreviewDialog(&printer, this);
// 	QObject::connect(dialog, SIGNAL(paintRequested(QPrinter*)), this, SLOT(slotPrint(QPrinter*)));
// 	dialog->setWindowTitle(tr("Print Document"));
// 	ES::MainWindowHolder::instance()->getMainWindow()->setCentralWidget(dialog);
// 	dialog->exec();
	//preview end

	KDReports::Header& header2 = report.header(KDReports::FirstPage);

	QString titleStr = ES::Session::getInstance()->getBillTitle();
	KDReports::TextElement titleElement(titleStr);
	titleElement.setPointSize(14);
	titleElement.setBold(true);
	header2.addElement(titleElement, Qt::AlignCenter);

	QString addressStr = ES::Session::getInstance()->getBillAddress();
	KDReports::TextElement addressElement(addressStr);
	addressElement.setPointSize(10);
	addressElement.setBold(false);
	header2.addElement(addressElement, Qt::AlignCenter);

	QString phoneStr = ES::Session::getInstance()->getBillPhone();
	KDReports::TextElement telElement(phoneStr);
	telElement.setPointSize(10);
	telElement.setBold(false);
	header2.addElement(telElement, Qt::AlignCenter);

	QString emailStr = ES::Session::getInstance()->getBillEmail();
	if (emailStr != "")
	{
		KDReports::TextElement emailElement(emailStr);
		emailElement.setPointSize(10);
		emailElement.setBold(false);
		header2.addElement(emailElement, Qt::AlignCenter);
	}

	KDReports::Header& header1 = report.header(KDReports::AllPages);

	KDReports::TextElement billIdHead("Bill No : " + QString::number(billId));
	billIdHead.setPointSize(11);
	billIdHead.setBold(true);
	header1.addElement(billIdHead);

	report.setMargins(10, 15, 10, 15);

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
	te.setPointSize(ES::Session::getInstance()->getBillItemFontSize());
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


// float ESSinglePayment::getTotalOutstanding(QString customerId)
// {
// 	float totalAmount = 0.f;
// 	QString query;
// 	query.append("SELECT * FROM customer_outstanding WHERE customer_id = ");
// 	query.append(customerId);
// 	query.append(" AND settled = 0");
// 
// 	QSqlQuery q(query);
// 	while (q.next())
// 	{
// 		QString paymentId = q.value("payment_id").toString();
// 		QSqlQuery qry("SELECT * FROM payment WHERE payment_id = " + paymentId);
// 		QString pm = q.value("payment_method").toString();
// 		float interest = 0;
// 		if (pm == "CREDIT")
// 		{
// 			QSqlQuery qq("SELECT * FROM credit WHERE credit_id = " + q.value("table_id").toString());
// 			if (qq.next())
// 			{
// 				interest = qq.value("interest").toFloat();
// 				float amount = qq.value("amount").toFloat();
// 				totalAmount += (amount * (100 + interest) / 100);
// 			}
// 		}
// 		else if (pm == "CHEQUE")
// 		{
// 			QSqlQuery qq("SELECT * FROM cheque WHERE cheque_id = " + q.value("table_id").toString());
// 			if (qq.next())
// 			{
// 				interest = qq.value("interest").toFloat();
// 				float amount = qq.value("amount").toFloat();
// 				totalAmount += (amount * (100 + interest) / 100);
// 			}
// 		}
// 	}
// 	return totalAmount;
// }
