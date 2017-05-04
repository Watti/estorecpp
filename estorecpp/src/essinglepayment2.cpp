#include "essinglepayment2.h"
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

ESSinglePayment2::ESSinglePayment2(ESReturnItems* returnItemsWidget, QWidget *parent /*= 0*/) :
QWidget(parent), m_returnItemsWidget(returnItemsWidget)
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
	m_billId = returnItemsWidget->getUI().billIdLbl->text().toLong();
	m_returnTotal = returnItemsWidget->getUI().returnTotal->text().toFloat();
	m_returnStartAmount = returnItemsWidget->getUI().returnSubTotal->text().toFloat();
	m_initialNetAmount = returnItemsWidget->getUI().newTotal->text().toFloat() - m_returnTotal;
	m_newBillTotal = returnItemsWidget->getUI().newTotal->text().toFloat();
	m_returnBillId = returnItemsWidget->getUI().billIdSearchText->text();
	ui.netAmountLbl->setText(QString::number(m_initialNetAmount, 'f', 2));
	ui.totalBillLbl->setText(QString::number(m_initialNetAmount, 'f', 2));
	ui.noOfItemsLbl->setText(QString::number(returnItemsWidget->getUI().billTableWidget->rowCount()));

}

ESSinglePayment2::~ESSinglePayment2()
{

}

void ESSinglePayment2::slotSearch()
{

}


void ESSinglePayment2::slotCalculateBalance()
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

void ESSinglePayment2::slotFinalizeBill()
{
	if (!validate())
		return;

	//QString billIdStr = ES::Session::getInstance()->getBillId();
	QString netAmountStr = ui.netAmountLbl->text();
	//QString paymentType = "CASH";// ui.paymentMethodCombo->currentText();
	bool isValid = false;

	double netAmount = netAmountStr.toDouble(&isValid);
	if (!isValid /*|| netAmount < 0*/)
	{
		return;
	}
	int billId = m_billId;
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
		double outstandingAmount = ES::Utility::getTotalCreditOutstanding(m_customerId);
		double outstandingLimit = ES::Utility::getOutstandingLimit(m_customerId);
		outstandingAmount += netAmount;
		bool exeedingOutstanding = (outstandingAmount > outstandingLimit);
		if (outstandingLimit != -1 && outstandingLimit > 0)
		{
			if (exeedingOutstanding)
			{
				QMessageBox mbox;
				mbox.setIcon(QMessageBox::Warning);
				mbox.setText(QString("Outstanding Limit has been exceeded ! ! ! Cannot Proceed without settling the outstanding amount"));
				mbox.exec();
				this->close();
				return;
			}
			else
			{
				handleCreditPayment(billId, netAmount);
			}
		}
		else
		{
			handleCreditPayment(billId, netAmount);
		}
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
	m_returnItemsWidget->finishBill();
}

bool ESSinglePayment2::validate()
{
	return true;
}

void ESSinglePayment2::handleCashPayment(int billId, double netAmount)
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
			mbox.setText(QString("Failed to insert into cash"));
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
		mbox.setText(QString("Failed to insert into payment"));
		mbox.exec();
	}
}

void ESSinglePayment2::handleCreditPayment(int billId, double netAmount)
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

void ESSinglePayment2::handleChequePayment(int billId, double netAmount)
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

void ESSinglePayment2::handleCreditCardPayment(int billId, double netAmount)
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

void ESSinglePayment2::handleLoyaltyPayment(int billId, double netAmount)
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

void ESSinglePayment2::slotPaymentMethodSelected()
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

void ESSinglePayment2::setCustomerId(QString customerId)
{
	m_customerId = customerId;
}

void ESSinglePayment2::finishBill(double netAmount, int billId)
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
		// Update database
		m_returnItemsWidget->updateDatabase();

		// Update stock quantity
		QSqlQuery saleQuantityQuery;
		saleQuantityQuery.prepare("SELECT * FROM sale WHERE deleted = 0 AND bill_id = ?");
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
					//QSqlQuery stockUpdateQuery;
					//stockUpdateQuery.prepare("UPDATE stock SET qty = ? WHERE stock_id = ?");
					//stockUpdateQuery.addBindValue(remainingQty);
					//stockUpdateQuery.addBindValue(stockId);
					//stockUpdateQuery.exec();

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

#if NETWORK_SYSTEM
			QSqlQuery queryDeleteBillSession("DELETE FROM bill_session WHERE bill_id = " + QString::number(billId));
#endif
		}

		if (ui.doPrintCB->isChecked())
		{
			printBill(billId, netAmount);
		}
	}
	this->close();
}

void ESSinglePayment2::slotInterestChanged()
{
	double interest = ui.lineEdit->text().toDouble();
	double netAmout = m_initialNetAmount;

	double totalBill = netAmout + netAmout * (interest / 100.0);

	ui.netAmountLbl->setText(QString::number(totalBill, 'f', 2));
}

void ESSinglePayment2::printBill(int billId, float total)
{
	slotPrintReturnBill();
}

void ESSinglePayment2::slotPrint(QPrinter* printer)
{
	report.print(printer);
	this->close();
}

void ESSinglePayment2::printRow(KDReports::TableElement& tableElement, int row, int col, QString elementStr, Qt::AlignmentFlag alignment)
{
	KDReports::Cell& cell = tableElement.cell(row, col);
	KDReports::TextElement te(elementStr);
	te.setPointSize(ES::Session::getInstance()->getBillItemFontSize());
	cell.addElement(te, alignment);
}

void ESSinglePayment2::keyPressEvent(QKeyEvent * event)
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

float ESSinglePayment2::getInitialNetAmount() const
{
	return m_initialNetAmount;
}

void ESSinglePayment2::setInitialNetAmount(float val)
{
	m_initialNetAmount = val;
}

void ESSinglePayment2::slotDiscountPercentage()
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

float ESSinglePayment2::getReturnInterest() const
{
	return m_returnInterest;
}

void ESSinglePayment2::setReturnInterest(float val)
{
	m_returnInterest = val;
}

float ESSinglePayment2::getReturnTotal() const
{
	return m_returnTotal;
}

void ESSinglePayment2::setReturnTotal(float val)
{
	m_returnTotal = val;
}

void ESSinglePayment2::slotPrintReturnBill()
{
	QString billedUser = "Billed Cashier : ";
	QString qStrUserQry("SELECT display_name FROM user JOIN bill ON user.user_id = bill.user_id WHERE bill.bill_id = " + 
		QString::number(m_returnItemsWidget->getOldBillId()));
	QSqlQuery q(qStrUserQry);
	if (q.next())
	{
		billedUser.append(q.value("display_name").toString());
	}
	QString titleStr = "Return Bill";
	KDReports::TextElement titleElement(titleStr);
	titleElement.setPointSize(14);
	titleElement.setBold(true);
	report.addElement(titleElement, Qt::AlignHCenter);
	
	QString dateStr = "Date : ";
	dateStr.append(QDateTime::currentDateTime().toString("yyyy-MM-dd"));
	QString timeStr = "Time : ";
	timeStr.append(QDateTime::currentDateTime().toString("hh : mm"));

	KDReports::TableElement infoTableElement;
	infoTableElement.setHeaderRowCount(3);
	infoTableElement.setHeaderColumnCount(2);
	infoTableElement.setBorder(0);
	infoTableElement.setWidth(100, KDReports::Percent);
	{
		KDReports::Cell& oldBillNoCell = infoTableElement.cell(0, 0);
		KDReports::TextElement t("Orig.Bill No : " + QString::number(m_returnItemsWidget->getOldBillId()));
		t.setPointSize(10);
		oldBillNoCell.addElement(t, Qt::AlignLeft);
	}{
		KDReports::Cell& userNameCell = infoTableElement.cell(1, 0);
		KDReports::TextElement t("Cashier : " + ES::Session::getInstance()->getUser()->getName());
		t.setPointSize(10);
		userNameCell.addElement(t, Qt::AlignLeft);
	}{
		KDReports::Cell& userNameCell = infoTableElement.cell(2, 0);
		KDReports::TextElement t(billedUser);
		t.setPointSize(10);
		userNameCell.addElement(t, Qt::AlignLeft);
	}{
		KDReports::Cell& returnBillNoCell = infoTableElement.cell(0, 1);
		KDReports::TextElement t("Bill No : " + QString::number(m_returnItemsWidget->getBillId()));
		t.setPointSize(10);
		returnBillNoCell.addElement(t, Qt::AlignRight);
	}{
		KDReports::Cell& dateCell = infoTableElement.cell(1, 1);
		KDReports::TextElement t(dateStr);
		t.setPointSize(10);
		dateCell.addElement(t, Qt::AlignRight);
	}{
		KDReports::Cell& timeCell = infoTableElement.cell(2, 1);
		KDReports::TextElement t(timeStr);
		t.setPointSize(10);
		timeCell.addElement(t, Qt::AlignRight);
	}

	report.addElement(infoTableElement);
	report.addVerticalSpacing(1);

	KDReports::TextElement retItemsElem("Return Items");
	retItemsElem.setPointSize(11);
	retItemsElem.setBold(false);
	report.addElement(retItemsElem, Qt::AlignLeft);

	KDReports::HtmlElement htmlElem1;
	QString htm1("<div><hr/></div>");
	htmlElem1.setHtml(htm1);
	report.addElement(htmlElem1);
	//report.addVerticalSpacing(1);

	//////////////////////////////////////////////////////////////////////////
	KDReports::TableElement dataTableElement;
	dataTableElement.setHeaderRowCount(2);
	dataTableElement.setHeaderColumnCount(5);
	dataTableElement.setBorder(0);
	dataTableElement.setWidth(100, KDReports::Percent);
	double unitPrice = 0, qty = 0, retTotal = 0;
	int row = 0;
	for (int i = 0; i < m_returnItemsWidget->getUI().tableWidget->rowCount(); ++i)
	{
		row = i;
		{
			KDReports::Cell& cell = dataTableElement.cell(i, 0);
			QString code = m_returnItemsWidget->getUI().tableWidget->item(i, 0)->text();
			KDReports::TextElement t(code);
			t.setPointSize(10);
			cell.addElement(t, Qt::AlignLeft);
		}{
			KDReports::Cell& cell = dataTableElement.cell(i, 1);
			QString name = m_returnItemsWidget->getUI().tableWidget->item(i, 1)->text();
			KDReports::TextElement t(name);
			t.setPointSize(10);
			cell.addElement(t, Qt::AlignLeft);
		}{
			QString qtyStr = m_returnItemsWidget->getUI().tableWidget->item(i, 2)->text();
			qty = qtyStr.toDouble();
			KDReports::Cell& cell = dataTableElement.cell(i, 2);
			KDReports::TextElement t(qtyStr);
			t.setPointSize(10);
			cell.addElement(t, Qt::AlignRight);
		}{
			KDReports::Cell& cell = dataTableElement.cell(i, 3);
			QString billedPrice = m_returnItemsWidget->getUI().tableWidget->item(i, 3)->text();
			KDReports::TextElement t(billedPrice);
			t.setPointSize(10);
			cell.addElement(t, Qt::AlignRight);
		}{
			QString paidPrice = m_returnItemsWidget->getUI().tableWidget->item(i, 4)->text();
			unitPrice = paidPrice.toDouble();
			double total = qty * unitPrice;
			KDReports::Cell& cell = dataTableElement.cell(i, 4);
			KDReports::TextElement t(QString::number(total, 'f', 2));
			t.setPointSize(10);
			cell.addElement(t, Qt::AlignRight);
		}

		retTotal += unitPrice * qty;
	}

	QString returnInterestStr = m_returnItemsWidget->getUI().returnInterest->text();
	double returnInterest = returnInterestStr.toDouble();
	if (returnInterest > 0)
	{
		retTotal = retTotal + (retTotal * returnInterest * 0.01);
	}

	row++; // sub total
	{
		KDReports::Cell& total = dataTableElement.cell(row, 3);
		//total.setColumnSpan(5);
		KDReports::TextElement totalTxt("Sub Total(RTN) :");
		totalTxt.setPointSize(10);
		totalTxt.setBold(true);
		total.addElement(totalTxt, Qt::AlignRight);
	}
	{
		KDReports::Cell& total = dataTableElement.cell(row, 4);
		//total.setColumnSpan(5);
		QString totalStr = QString::number(retTotal, 'f', 2);
		KDReports::TextElement totalValue(totalStr);
		totalValue.setPointSize(10);
		totalValue.setBold(true);
		total.addElement(totalValue, Qt::AlignRight);
	}

	row++; // interest
	{
		KDReports::Cell& total = dataTableElement.cell(row, 3);
		//total.setColumnSpan(5);
		KDReports::TextElement totalTxt("Interest :");
		totalTxt.setPointSize(10);
		totalTxt.setBold(true);
		total.addElement(totalTxt, Qt::AlignRight);
	}
	{
		KDReports::Cell& total = dataTableElement.cell(row, 4);
		//total.setColumnSpan(5);
		KDReports::TextElement totalValue(QString::number(returnInterest, 'f', 2));
		totalValue.setPointSize(10);
		totalValue.setBold(true);
		total.addElement(totalValue, Qt::AlignRight);
	}

	report.addElement(dataTableElement);
	report.addVerticalSpacing(2);

	KDReports::TextElement newItemsElem("New Items");
	newItemsElem.setPointSize(11);
	newItemsElem.setBold(false);
	report.addElement(newItemsElem, Qt::AlignLeft);
	report.addElement(htmlElem1);

	//////////////////////////////////////////////////////////////////////////
	KDReports::TableElement newdataTableElement;
	newdataTableElement.setHeaderRowCount(2);
	newdataTableElement.setHeaderColumnCount(5);
	newdataTableElement.setBorder(0);
	newdataTableElement.setWidth(100, KDReports::Percent);
	
	double retTotal2 = 0;
	int row2 = 0;
	for (int i = 0; i < m_returnItemsWidget->getUI().billTableWidget->rowCount(); ++i)
	{
		double unitPrice2 = 0, qty2 = 0;
		row2 = i;
		{
			KDReports::Cell& cell = newdataTableElement.cell(i, 0);
			QString code = m_returnItemsWidget->getUI().billTableWidget->item(i, 0)->text();
			KDReports::TextElement t(code);
			t.setPointSize(10);
			cell.addElement(t, Qt::AlignLeft);
		}{
			KDReports::Cell& cell = newdataTableElement.cell(i, 1);
			QString name = m_returnItemsWidget->getUI().billTableWidget->item(i, 1)->text();
			KDReports::TextElement t(name);
			t.setPointSize(10);
			cell.addElement(t, Qt::AlignLeft);
		}{
			QString qtyStr = m_returnItemsWidget->getUI().billTableWidget->item(i, 3)->text();
			qty2 = qtyStr.toDouble();
			KDReports::Cell& cell = newdataTableElement.cell(i, 2);
			KDReports::TextElement t(qtyStr);
			t.setPointSize(10);
			cell.addElement(t, Qt::AlignRight);
		}{
			KDReports::Cell& cell = newdataTableElement.cell(i, 3);
			QString billedPrice = m_returnItemsWidget->getUI().billTableWidget->item(i, 2)->text();
			KDReports::TextElement t(billedPrice);
			t.setPointSize(10);
			cell.addElement(t, Qt::AlignRight);
		}{
			QString paidPrice = m_returnItemsWidget->getUI().billTableWidget->item(i, 2)->text();
			unitPrice2 = paidPrice.toDouble();
			double total = qty2 * unitPrice2;
			KDReports::Cell& cell = newdataTableElement.cell(i, 4);
			KDReports::TextElement t(QString::number(total, 'f', 2));
			t.setPointSize(10);
			cell.addElement(t, Qt::AlignRight);
		}

		retTotal2 += unitPrice2 * qty2;
	}
	row2++;
	{//net total
		KDReports::Cell& total = newdataTableElement.cell(row2, 3);
		//total.setColumnSpan(5);
		KDReports::TextElement totalTxt("Sub Total (NEW) :");
		totalTxt.setPointSize(10);
		totalTxt.setBold(true);
		total.addElement(totalTxt, Qt::AlignRight);
	}
	{
		KDReports::Cell& total = newdataTableElement.cell(row2, 4);
		//total.setColumnSpan(5);
		QString totalStr = QString::number(retTotal2, 'f', 2);
		KDReports::TextElement totalValue(totalStr);
		totalValue.setPointSize(10);
		totalValue.setBold(true);
		total.addElement(totalValue, Qt::AlignRight);
	}
	QString newBillInterestStr = m_returnItemsWidget->getUI().newInterest->text();
	double newBillInterest = newBillInterestStr.toDouble();
	if (newBillInterest > 0)
	{
		retTotal2 = retTotal2 * (100 + newBillInterest) / 100.0;
	}
	row2++;
	{//interest
		KDReports::Cell& total = newdataTableElement.cell(row2, 3);
		//total.setColumnSpan(5);
		KDReports::TextElement totalTxt("Interest(NEW) :");
		totalTxt.setPointSize(10);
		totalTxt.setBold(true);
		total.addElement(totalTxt, Qt::AlignRight);
	}
	{
		KDReports::Cell& total = newdataTableElement.cell(row2, 4);
		//total.setColumnSpan(5);
		QString totalStr = QString::number(newBillInterest, 'f', 2);
		KDReports::TextElement totalValue(totalStr);
		totalValue.setPointSize(10);
		totalValue.setBold(true);
		total.addElement(totalValue, Qt::AlignRight);
	}
	row2++; // sub total
	{
		KDReports::Cell& total = newdataTableElement.cell(row2, 3);
		//total.setColumnSpan(5);
		KDReports::TextElement totalTxt("Total(NEW) :");
		totalTxt.setPointSize(10);
		totalTxt.setBold(true);
		total.addElement(totalTxt, Qt::AlignRight);
	}
	{
		KDReports::Cell& total = newdataTableElement.cell(row2, 4);
		//total.setColumnSpan(5);
		QString totalStr = QString::number(retTotal2, 'f', 2);
		KDReports::TextElement totalValue(totalStr);
		totalValue.setPointSize(10);
		totalValue.setBold(true);
		total.addElement(totalValue, Qt::AlignRight);
	}
	
	//////////////////////////////////////////////////////////////////////////

	row2+=2; // sub total
	{
		KDReports::Cell& total = newdataTableElement.cell(row2, 0);
		//total.setColumnSpan(5);
		KDReports::TextElement totalTxt("Orig.Bill Total :");
		totalTxt.setPointSize(10);
		totalTxt.setBold(true);
		total.addElement(totalTxt, Qt::AlignLeft);
	}
	{
		double oldTotal;
		QSqlQuery billAmount("SELECT amount FROM bill WHERE bill_id =" + QString::number(m_returnItemsWidget->getOldBillId()));
		if (billAmount.next())
		{
			oldTotal = billAmount.value("amount").toDouble();
		}
		KDReports::Cell& total = newdataTableElement.cell(row2, 1);
		//total.setColumnSpan(5);
		KDReports::TextElement totalTxt(QString::number(oldTotal, 'f', 2));
		totalTxt.setPointSize(10);
		totalTxt.setBold(true);
		total.addElement(totalTxt, Qt::AlignLeft);
	}
	{
		KDReports::Cell& total = newdataTableElement.cell(row2, 3);
		//total.setColumnSpan(5);
		KDReports::TextElement totalTxt("Sub Total :");
		totalTxt.setPointSize(10);
		totalTxt.setBold(true);
		total.addElement(totalTxt, Qt::AlignRight);
	}
	{
		//double interest = m_returnItemsWidget->getUI().returnInterest->text().toDouble();
		double subTotal =( retTotal * -1) + retTotal2;

		KDReports::Cell& total = newdataTableElement.cell(row2, 4);
		//total.setColumnSpan(5);
		QString totalStr = QString::number(subTotal, 'f', 2);
		KDReports::TextElement totalValue(totalStr);
		totalValue.setPointSize(10);
		totalValue.setBold(true);
		total.addElement(totalValue, Qt::AlignRight);
	}
	row2++;
	{
		KDReports::Cell& total = newdataTableElement.cell(row2, 0);
		//total.setColumnSpan(5);
		KDReports::TextElement totalTxt("Outstanding :");
		totalTxt.setPointSize(10);
		totalTxt.setBold(true);
		total.addElement(totalTxt, Qt::AlignLeft);
	}
	{
		KDReports::Cell& total = newdataTableElement.cell(row2, 1);
		//total.setColumnSpan(5);
		KDReports::TextElement totalTxt(ui.outstandingText->text());
		totalTxt.setPointSize(10);
		totalTxt.setBold(true);
		total.addElement(totalTxt, Qt::AlignLeft);
	}
	if (m_paymentMethod == "CREDIT")
	{
		{
			KDReports::Cell& total = newdataTableElement.cell(row2, 3);
			//total.setColumnSpan(5);
			KDReports::TextElement totalTxt("Not Paid :");
			totalTxt.setPointSize(10);
			totalTxt.setBold(true);
			total.addElement(totalTxt, Qt::AlignRight);
		}
		{
			KDReports::Cell& total = newdataTableElement.cell(row2, 4);
			//total.setColumnSpan(5);
			double interest = m_returnItemsWidget->getUI().returnInterest->text().toDouble();
			double subTotal =  (retTotal *-1)  + retTotal2;
			double notPaid = ui.outstandingText->text().toDouble() + subTotal;
			KDReports::TextElement totalTxt(QString::number(notPaid, 'f', 2));
			totalTxt.setPointSize(10);
			totalTxt.setBold(true);
			total.addElement(totalTxt, Qt::AlignRight);
		}
	}

	report.addElement(newdataTableElement);
	report.addVerticalSpacing(2);
	if (!ES::Session::getInstance()->isSecondDisplayOn())
	{
		KDReports::Header& header2 = report.header(KDReports::FirstPage);

		QString titleStr2 = ES::Session::getInstance()->getBillTitle();
		KDReports::TextElement titleElement2(titleStr2);
		titleElement2.setPointSize(14);
		titleElement2.setBold(true);
		header2.addElement(titleElement2, Qt::AlignCenter);

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
	}
	
	KDReports::Header& header1 = report.header(KDReports::AllPages);

	KDReports::TextElement billIdHead("Bill No : " + QString::number(m_returnItemsWidget->getBillId()));
	billIdHead.setPointSize(11);
	billIdHead.setBold(true);
	header1.addElement(billIdHead);

	//////////////////////////////////////////////////////////////////////////

	QPrinter printer;
	printer.setPaperSize(QPrinter::A4);

	printer.setFullPage(false);
	printer.setOrientation(QPrinter::Portrait);

	QPrintPreviewDialog *dialog = new QPrintPreviewDialog(&printer, this);
	QObject::connect(dialog, SIGNAL(paintRequested(QPrinter*)), this, SLOT(slotPrint(QPrinter*)));
	dialog->setWindowTitle(tr("Print Document"));
	ES::MainWindowHolder::instance()->getMainWindow()->setCentralWidget(dialog);
	dialog->exec();

	//report.print(&printer);
}
