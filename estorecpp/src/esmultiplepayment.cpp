#include "esmultiplepayment.h"
#include <QMessageBox>
#include <QShortcut>
#include <QSqlQuery>
#include <string>
#include "utility\session.h"
#include "QPrinter"
#include "KDReportsVariableType.h"
#include "KDReportsCell.h"
#include "KDReportsTextElement.h"
#include "KDReportsTableElement.h"
#include "QDateTime"
#include "KDReportsReport.h"
#include "QPrintPreviewDialog"
#include "utility\esmainwindowholder.h"
#include "esmainwindow.h"
#include "KDReportsHeader.h"
#include "KDReportsHtmlElement.h"
#include "qnamespace.h"
#include <memory>
#include <wingdi.h>
#include "easylogging++.h"
#include "utility\utility.h"

ESMultiplePayment::ESMultiplePayment(ESAddBill* addBill, QWidget *parent /*= 0*/) : QWidget(parent), m_addBill(addBill)
{
	m_customerId = "-1";
	ui.setupUi(this);

	m_removeButtonSignalMapper = new QSignalMapper(this);

	ui.cashBtn->setChecked(true);
	ui.lbl1->hide();
	ui.lbl2->hide();
	ui.dateLbl->hide();
	ui.interestLbl->hide();

	ui.txt1->hide();
	ui.txt2->hide();
	ui.dateEdit->hide();
	ui.interestTxt->hide();
	ui.interestPercentageCB->hide();

	ui.paymentType->setText("Cash :  ");
	m_paymentType = "CASH";

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

	QStringList headerLabels;
	headerLabels.append("Payment Type");
	headerLabels.append("Amount");
	headerLabels.append("Interest");
	headerLabels.append("Payment");
	headerLabels.append("Due Date");
	headerLabels.append("Number");
	headerLabels.append("Bank");
	headerLabels.append("Actions");
	headerLabels.append("Id");

	ui.tableWidget->setHorizontalHeaderLabels(headerLabels);
	ui.tableWidget->horizontalHeader()->setStretchLastSection(true);
	ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	ui.tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.tableWidget->hideColumn(8);

	QObject::connect(ui.cashBtn, SIGNAL(clicked()), this, SLOT(slotPaymentMethodSelected()));
	QObject::connect(ui.creditBtn, SIGNAL(clicked()), this, SLOT(slotPaymentMethodSelected()));
	QObject::connect(ui.chequeBtn, SIGNAL(clicked()), this, SLOT(slotPaymentMethodSelected()));
	QObject::connect(ui.creditCardBtn, SIGNAL(clicked()), this, SLOT(slotPaymentMethodSelected()));
	QObject::connect(ui.loyalityCardBtn, SIGNAL(clicked()), this, SLOT(slotPaymentMethodSelected()));
	QObject::connect(m_removeButtonSignalMapper, SIGNAL(mapped(int)), this, SLOT(slotRemove(int)));

	QObject::connect(ui.cashText, SIGNAL(textChanged(QString)), this, SLOT(slotCalculateBalance()));
	QObject::connect(ui.addBtn, SIGNAL(clicked()), this, SLOT(slotAdd()));
	QObject::connect(ui.okBtn, SIGNAL(clicked()), this, SLOT(slotFinalizeBill()));
	//QObject::connect(ui.interestTxt, SIGNAL(textChanged(QString)), this, SLOT(slotInterestChanged()));

	new QShortcut(QKeySequence(Qt::Key_Escape), this, SLOT(close()));
	setMinimumWidth(900);
	setMinimumHeight(550);
	//resize(900, 1);
	adjustSize();
	ui.dateEdit->setDate(QDate::currentDate());
	ui.okBtn->setDisabled(true);
}

ESMultiplePayment::~ESMultiplePayment()
{

}

void ESMultiplePayment::setCustomerId(QString customerId)
{
	m_customerId = customerId;
}

void ESMultiplePayment::slotSearch()
{

}

bool ESMultiplePayment::validate()
{
	return true;
}


void ESMultiplePayment::slotCalculateBalance()
{
	bool isValid = false;
	double cash = 0.0;
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

	double amount = ui.netAmountLbl->text().toDouble();
	ui.balanceLbl->setText(QString::number(cash - amount, 'f', 2));

	if (ui.cashText->text().isEmpty())
	{
		ui.balanceLbl->setText("0.00");
	}
}


void ESMultiplePayment::slotPaymentMethodSelected()
{
	if (ui.creditBtn == sender())
	{
		ui.lbl1->hide();
		ui.lbl2->hide(); // bank
		ui.interestLbl->show();
		ui.dateLbl->show(); // due date

		ui.txt1->hide();
		ui.txt2->hide();
		ui.interestTxt->show();
		ui.interestPercentageCB->show();
		ui.dateEdit->show();

		ui.paymentType->setText("Amount :  ");

		ui.cashBtn->setChecked(false);
		ui.chequeBtn->setChecked(false);
		ui.creditCardBtn->setChecked(false);
		ui.loyalityCardBtn->setChecked(false);
		m_paymentType = "CREDIT";
	}
	else if (ui.chequeBtn == sender())
	{
		ui.lbl1->show(); // cheque no
		ui.lbl2->show(); // bank
		ui.dateLbl->show(); // due date
		ui.interestLbl->show();

		ui.txt1->show();
		ui.txt2->show();
		ui.dateEdit->show();
		ui.interestTxt->show();
		ui.interestPercentageCB->show();

		ui.lbl1->setText("Cheque No. :  ");
		ui.lbl2->setText("Bank :  ");
		ui.txt1->setText("");
		ui.txt2->setText("");

		ui.cashBtn->setChecked(false);
		ui.creditBtn->setChecked(false);
		ui.creditCardBtn->setChecked(false);
		ui.loyalityCardBtn->setChecked(false);

		ui.paymentType->setText("Amount :  ");
		m_paymentType = "CHEQUE";
	}
	else if (ui.creditCardBtn == sender())
	{
		ui.lbl1->show(); // card no
		ui.lbl2->hide();
		ui.interestLbl->show();
		ui.dateLbl->hide();

		ui.txt1->show();
		ui.txt2->hide();
		ui.interestTxt->show();
		ui.interestPercentageCB->show();
		ui.dateEdit->hide();

		ui.lbl1->setText("Card No. :  ");
		ui.txt1->setText("");
		//ui.lbl2->setText("Interest % :  ");
		ui.paymentType->setText("Amount :  ");

		ui.cashBtn->setChecked(false);
		ui.chequeBtn->setChecked(false);
		ui.creditBtn->setChecked(false);
		ui.loyalityCardBtn->setChecked(false);

		m_paymentType = "CARD";
	}
	else if (ui.loyalityCardBtn == sender())
	{
		ui.lbl1->show(); // card no
		ui.lbl2->hide();
		ui.dateLbl->hide();
		ui.interestLbl->show();

		ui.txt1->show();
		ui.txt2->hide();
		ui.dateEdit->hide();
		ui.interestTxt->show();
		ui.interestPercentageCB->show();

		ui.lbl1->setText("Card No. :  ");
		ui.txt1->setText("");
		ui.paymentType->setText("Amount :  ");


		ui.cashBtn->setChecked(false);
		ui.chequeBtn->setChecked(false);
		ui.creditCardBtn->setChecked(false);
		ui.creditBtn->setChecked(false);

		m_paymentType = "LOYALTY";
	}
	else if (ui.cashBtn == sender())
	{
		ui.lbl1->hide();
		ui.lbl2->hide();
		ui.dateLbl->hide();
		ui.interestLbl->hide();

		ui.txt1->hide();
		ui.txt2->hide();
		ui.dateEdit->hide();
		ui.interestTxt->hide();
		ui.interestPercentageCB->hide();

		ui.paymentType->setText("Cash :  ");

		ui.creditBtn->setChecked(false);
		ui.chequeBtn->setChecked(false);
		ui.creditCardBtn->setChecked(false);
		ui.loyalityCardBtn->setChecked(false);
		m_paymentType = "CASH";
	}
	ui.interestTxt->setText("0");
	//resize(900, 1);
	adjustSize();
}

void ESMultiplePayment::slotAdd()
{
	// 
	// 	headerLabels.append("Payment Type");
	// 	headerLabels.append("Amount");
	// 	headerLabels.append("Interest");
	// 	headerLabels.append("Due Date");
	// 	headerLabels.append("Number");
	// 	headerLabels.append("Bank");
	// 	headerLabels.append("Actions");

	QPushButton* removeBtn = new QPushButton(ui.tableWidget);
	removeBtn->setIcon(QIcon("icons/delete.png"));
	removeBtn->setIconSize(QSize(24, 24));
	removeBtn->setMaximumWidth(36);

	bool isValid = false;
	double amountPaid = 0.0;
	if (!ui.cashText->text().isEmpty())
	{
		amountPaid = ui.cashText->text().toDouble(&isValid);
		if (!isValid)
		{
			QMessageBox mbox;
			mbox.setIcon(QMessageBox::Warning);
			mbox.setText(QString("Invalid input - Amount"));
			mbox.exec();
			ui.cashText->clear();
			return;
		}
		if (amountPaid <= 0)
		{
			QMessageBox mbox;
			mbox.setIcon(QMessageBox::Warning);
			mbox.setText(QString("Invalid input - Amount"));
			mbox.exec();
			return;
		}
	}
	else
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Warning);
		mbox.setText(QString("Invalid input - Amount"));
		mbox.exec();
		return;
	}

	double netAmount = ui.netAmountLbl->text().toDouble();
	double remaining = netAmount - amountPaid;
	double payingAmount = ui.totalBillLbl->text().toDouble();
	if (remaining >= 0)
	{
		ui.netAmountLbl->setText(QString::number(remaining, 'f', 2));
		ui.cashText->clear();
		ui.cashText->setFocus();
	}
	if (remaining <= 0)
	{
		ui.netAmountLbl->setText("0.00");
		ui.addBtn->setDisabled(true);
		amountPaid = netAmount;
		ui.okBtn->setEnabled(true);
	}

	int row = ui.tableWidget->rowCount();
	ui.tableWidget->insertRow(row);
	if (m_paymentType == "CASH")
	{
		ui.tableWidget->setItem(row, 0, new QTableWidgetItem(m_paymentType));
		ui.tableWidget->setItem(row, 1, new QTableWidgetItem(QString::number(amountPaid, 'f', 2)));
		ui.tableWidget->setItem(row, 2, new QTableWidgetItem("-/-"));
		ui.tableWidget->setItem(row, 3, new QTableWidgetItem(QString::number(amountPaid, 'f', 2)));
		ui.tableWidget->setItem(row, 4, new QTableWidgetItem("-/-"));
		ui.tableWidget->setItem(row, 5, new QTableWidgetItem("-/-"));
		ui.tableWidget->setItem(row, 6, new QTableWidgetItem("-/-"));
	}
	else if (m_paymentType == "CREDIT")
	{
		isValid = false;
		double interest = ui.interestTxt->text().toDouble(&isValid);
		if (isValid)
		{
			double interestAmount = amountPaid * (interest / 100);
			double amountWithInterest = +(interestAmount + amountPaid);
			payingAmount += interestAmount;
			ui.totalBillLbl->setText(QString::number(payingAmount, 'f', 2));
			ui.tableWidget->setItem(row, 0, new QTableWidgetItem(m_paymentType));
			ui.tableWidget->setItem(row, 1, new QTableWidgetItem(QString::number(amountPaid, 'f', 2)));
			ui.tableWidget->setItem(row, 2, new QTableWidgetItem(ui.interestTxt->text()));
			ui.tableWidget->setItem(row, 3, new QTableWidgetItem(QString::number(amountWithInterest, 'f', 2)));
			ui.tableWidget->setItem(row, 4, new QTableWidgetItem(ui.dateEdit->text()));
			ui.tableWidget->setItem(row, 5, new QTableWidgetItem("-/-"));
			ui.tableWidget->setItem(row, 6, new QTableWidgetItem("-/-"));
		}
	}
	else if (m_paymentType == "CHEQUE")
	{
		isValid = false;
		double interest = ui.interestTxt->text().toDouble(&isValid);
		if (isValid)
		{
			double interestAmount = amountPaid * (interest / 100);
			double amountWithInterest = +(interestAmount + amountPaid);
			payingAmount += interestAmount;
			ui.totalBillLbl->setText(QString::number(payingAmount, 'f', 2));
			ui.tableWidget->setItem(row, 0, new QTableWidgetItem(m_paymentType));
			ui.tableWidget->setItem(row, 1, new QTableWidgetItem(QString::number(amountPaid, 'f', 2)));
			ui.tableWidget->setItem(row, 2, new QTableWidgetItem(ui.interestTxt->text()));
			ui.tableWidget->setItem(row, 3, new QTableWidgetItem(QString::number(amountWithInterest, 'f', 2)));
			ui.tableWidget->setItem(row, 4, new QTableWidgetItem(ui.dateEdit->text()));
			ui.tableWidget->setItem(row, 5, new QTableWidgetItem(ui.txt1->text()));
			ui.tableWidget->setItem(row, 6, new QTableWidgetItem(ui.txt2->text()));
		}
	}
	else if (m_paymentType == "CARD")
	{
		isValid = false;
		double interest = ui.interestTxt->text().toDouble(&isValid);
		if (isValid)
		{
			double interestAmount = amountPaid * (interest / 100);
			double amountWithInterest = +(interestAmount + amountPaid);
			payingAmount += interestAmount;
			ui.totalBillLbl->setText(QString::number(payingAmount, 'f', 2));
			ui.tableWidget->setItem(row, 0, new QTableWidgetItem(m_paymentType));
			ui.tableWidget->setItem(row, 1, new QTableWidgetItem(QString::number(amountPaid, 'f', 2)));
			ui.tableWidget->setItem(row, 2, new QTableWidgetItem(ui.interestTxt->text()));
			ui.tableWidget->setItem(row, 3, new QTableWidgetItem(QString::number(amountWithInterest, 'f', 2)));
			ui.tableWidget->setItem(row, 4, new QTableWidgetItem("-/-"));
			ui.tableWidget->setItem(row, 5, new QTableWidgetItem(ui.txt1->text()));
			ui.tableWidget->setItem(row, 6, new QTableWidgetItem("-/-"));
		}
	}
	else if (m_paymentType == "LOYALTY")
	{
		ui.tableWidget->setItem(row, 0, new QTableWidgetItem(m_paymentType));
		ui.tableWidget->setItem(row, 1, new QTableWidgetItem(QString::number(amountPaid, 'f', 2)));
		ui.tableWidget->setItem(row, 2, new QTableWidgetItem(ui.interestTxt->text()));
		ui.tableWidget->setItem(row, 3, new QTableWidgetItem("-/-"));
		ui.tableWidget->setItem(row, 4, new QTableWidgetItem(ui.txt1->text()));
		ui.tableWidget->setItem(row, 5, new QTableWidgetItem("-/-"));
		ui.tableWidget->setItem(row, 6, new QTableWidgetItem("-/-"));
	}
	ui.cashText->setText(ui.netAmountLbl->text());
	ui.interestTxt->setText("0");
	m_removeButtonSignalMapper->setMapping(removeBtn, row);
	QObject::connect(removeBtn, SIGNAL(clicked()), m_removeButtonSignalMapper, SLOT(map()));
	ui.tableWidget->setCellWidget(row, 7, removeBtn);
	ui.tableWidget->setItem(row, 7, new QTableWidgetItem(QString::number(row)));
}

void ESMultiplePayment::slotRemove(int row)
{
	int rowCount = ui.tableWidget->rowCount();
	for (int i = 0; i < rowCount; ++i)
	{
		QTableWidgetItem* item = ui.tableWidget->item(i, 7);
		std::string s = item->text().toStdString();
		if (item && item->text().toInt() == row)
		{
			QTableWidgetItem* amountItem = ui.tableWidget->item(i, 1);
			double amountPaid = amountItem->text().toDouble();
			double netAmount = ui.netAmountLbl->text().toDouble();
			//double interest = ui.tableWidget->item(i, 2)->text().toDouble();
			//amountPaid = (amountPaid - (amountPaid*interest) / 100);
			ui.netAmountLbl->setText(QString::number((amountPaid + netAmount), 'f', 2));
			ui.cashText->setText(QString::number((amountPaid + netAmount), 'f', 2));
			double interest = ui.tableWidget->item(i, 2)->text().toDouble();
			double totalAmount = ui.totalBillLbl->text().toDouble() - (amountPaid*interest / 100);
			ui.totalBillLbl->setText(QString::number(totalAmount, 'f', 2));
			ui.addBtn->setEnabled(true);
			ui.okBtn->setDisabled(true);

			ui.tableWidget->removeRow(i);
			ui.cashText->setFocus();
			break;
		}
	}

}

void ESMultiplePayment::slotFinalizeBill()
{
	// 	headerLabels.append("Payment Type");
	// 	headerLabels.append("Amount");
	// 	headerLabels.append("Interest");
	// 	headerLabels.append("Due Date");
	// 	headerLabels.append("Number");
	// 	headerLabels.append("Bank");
	// 	headerLabels.append("Actions");
	QString billIdStr = ES::Session::getInstance()->getBillId();
	int billId = billIdStr.toInt();
	double totalNetAmount = 0, payingAmount = 0;
	{
		int rowCount = ui.tableWidget->rowCount();
		for (int i = 0; i < rowCount; ++i)
		{
			totalNetAmount += ui.tableWidget->item(i, 1)->text().toDouble();
			payingAmount += ui.tableWidget->item(i, 3)->text().toDouble();
		}
	}

	int rowCount = ui.tableWidget->rowCount();
	for (int i = 0; i < rowCount; ++i)
	{
		double amount = ui.tableWidget->item(i, 1)->text().toDouble();
		double interest = ui.tableWidget->item(i, 2)->text().toDouble();
		QTableWidgetItem* paymentTypeItem = ui.tableWidget->item(i, 0);
		if (paymentTypeItem)
		{
			if (paymentTypeItem->text() == "CASH")
			{
				QSqlQuery query;
				query.prepare("INSERT INTO payment (bill_id, payment_type, total_amount) VALUES (?, 'CASH', ?)");
				query.addBindValue(billId);
				query.addBindValue(amount);
				if (query.exec())
				{
					int lastInsertedId = query.lastInsertId().toInt();
					QSqlQuery q;
					q.prepare("INSERT INTO cash (payment_id, amount) VALUES (?, ?)");
					q.addBindValue(lastInsertedId);
					q.addBindValue(amount);
					if (!q.exec())
					{
						QMessageBox mbox;
						mbox.setIcon(QMessageBox::Critical);
						mbox.setText(QString("Failed"));
						mbox.exec();
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
			else if (paymentTypeItem->text() == "CREDIT")
			{
				QSqlQuery query;
				query.prepare("INSERT INTO payment (bill_id, payment_type, total_amount) VALUES (?, 'CREDIT', ?)");
				query.addBindValue(billIdStr);
				query.addBindValue(amount);
				if (query.exec())
				{
					int lastInsertedId = query.lastInsertId().toInt();
					QSqlQuery q;
					q.prepare("INSERT INTO credit (payment_id, amount, interest, due_date) VALUES (?, ?, ?, ?)");
					q.addBindValue(lastInsertedId);
					q.addBindValue(amount);
					q.addBindValue(ui.tableWidget->item(i, 2)->text());//percentage is directly stored
					q.addBindValue(ui.tableWidget->item(i, 4)->text());
					if (!q.exec())
					{
						QMessageBox mbox;
						mbox.setIcon(QMessageBox::Critical);
						mbox.setText(QString("Failed"));
						mbox.exec();
					}
					else
					{
						double outstandingAmount = ES::Utility::getTotalCreditOutstanding(m_customerId);//cheque is not considered here
						double outstandingForTheBill = amount* (100 + interest) / 100;
						outstandingAmount += outstandingForTheBill;

						ES::Utility::updateOutstandingAmount(m_customerId, outstandingForTheBill);
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
			else if (paymentTypeItem->text() == "CHEQUE")
			{
				QSqlQuery query;
				query.prepare("INSERT INTO payment (bill_id, payment_type, total_amount) VALUES (?, 'CHEQUE', ?)");
				query.addBindValue(billIdStr);
				query.addBindValue(amount);
				if (query.exec())
				{
					int lastInsertedId = query.lastInsertId().toInt();
					QSqlQuery q;
					q.prepare("INSERT INTO cheque (payment_id, amount, interest, cheque_number, bank, due_date) VALUES (?, ?, ?, ?, ?, ?)");
					q.addBindValue(lastInsertedId);
					q.addBindValue(amount);
					q.addBindValue(ui.tableWidget->item(i, 2)->text());
					q.addBindValue(ui.tableWidget->item(i, 5)->text());
					q.addBindValue(ui.tableWidget->item(i, 6)->text());
					q.addBindValue(ui.tableWidget->item(i, 4)->text());
					if (!q.exec())
					{
						QMessageBox mbox;
						mbox.setIcon(QMessageBox::Critical);
						mbox.setText(QString("Failed"));
						mbox.exec();
					}
					else
					{
						QString qStr("INSERT INTO cheque_information (customer_id, cheque_number, bank, due_date) VALUES (" +
							m_customerId + ",'" + ui.tableWidget->item(i, 5)->text() + "','" + ui.tableWidget->item(i, 6)->text() + "','" + ui.tableWidget->item(i, 4)->text() + "')");
						QSqlQuery outstandingQry;
						if (!outstandingQry.exec(qStr))
						{
							QMessageBox mbox;
							mbox.setIcon(QMessageBox::Critical);
							mbox.setText(QString("Failed to add cheque information"));
							mbox.exec();
						}
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
			else if (paymentTypeItem->text() == "CARD")
			{
				QSqlQuery query;
				query.prepare("INSERT INTO payment (bill_id, payment_type, total_amount) VALUES (?, 'CARD', ?)");
				query.addBindValue(billIdStr);
				query.addBindValue(amount);
				if (query.exec())
				{
					int lastInsertedId = query.lastInsertId().toInt();
					QSqlQuery q;
					q.prepare("INSERT INTO card (payment_id, amount, card_no, interest) VALUES (?, ?, ?, ?)");
					q.addBindValue(lastInsertedId);
					q.addBindValue(amount);
					q.addBindValue(ui.tableWidget->item(i, 5)->text());
					q.addBindValue(ui.tableWidget->item(i, 2)->text());
					if (!q.exec())
					{
						QMessageBox mbox;
						mbox.setIcon(QMessageBox::Critical);
						mbox.setText(QString("Failed"));
						mbox.exec();
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
			else if (paymentTypeItem->text() == "LOYALTY")
			{
				QSqlQuery query;
				query.prepare("INSERT INTO payment (bill_id, payment_type, total_amount) VALUES (?, 'LOYALTY', ?)");
				query.addBindValue(billIdStr);
				query.addBindValue(amount);
				if (query.exec())
				{
					int lastInsertedId = query.lastInsertId().toInt();
					QSqlQuery q;
					q.prepare("INSERT INTO loyalty (payment_id, amount, card_no, interest) VALUES (?, ?, ?, ?)");
					q.addBindValue(lastInsertedId);
					q.addBindValue(amount);
					q.addBindValue(ui.tableWidget->item(i, 5)->text());
					q.addBindValue(ui.tableWidget->item(i, 2)->text());
					if (!q.exec())
					{
						QMessageBox mbox;
						mbox.setIcon(QMessageBox::Critical);
						mbox.setText(QString("Failed"));
						mbox.exec();
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
		}
	}

	finishBill(payingAmount, billId);
}

void ESMultiplePayment::finishBill(double netAmount, int billId)
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
			printBill(billId, netAmount);
		}
	}
	this->close();
}

void ESMultiplePayment::printBill(int billId, float total)
{
	QString billStr("SELECT * FROM bill WHERE bill_id = " + QString::number(billId));
	QString userName = "";
	QSqlQuery queryBill(billStr);
	if (queryBill.next())
	{
		QString userId = queryBill.value("user_id").toString();
		QString qStrUser("SELECT display_name FROM user WHERE user_id = " + userId);
		m_customerId = queryBill.value("customer_id").toString();
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
	QString payamentStr = "";
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
	customerInfo2.setPointSize(8);
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

void ESMultiplePayment::printRow(KDReports::TableElement& tableElement, int row, int col, QString elementStr, Qt::AlignmentFlag alignment)
{
	KDReports::Cell& cell = tableElement.cell(row, col);
	KDReports::TextElement te(elementStr);
	te.setPointSize(ES::Session::getInstance()->getBillItemFontSize());
	cell.addElement(te, alignment);
}

void ESMultiplePayment::slotPrint(QPrinter* printer)
{
	//report.print(printer);
	this->close();
}

float ESMultiplePayment::getInitialNetAmount() const
{
	return m_initialNetAmount;
}

void ESMultiplePayment::setInitialNetAmount(float val)
{
	m_initialNetAmount = val;
}

