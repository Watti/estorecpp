#include "esmultiplepayment.h"
#include "QMessageBox"
#include "QShortcut"
#include <string>

ESMultiplePayment::ESMultiplePayment(QWidget *parent /*= 0*/) : QWidget(parent)
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
	ui.tableWidget->hideColumn(7);

	QObject::connect(ui.cashBtn, SIGNAL(clicked()), this, SLOT(slotPaymentMethodSelected()));
	QObject::connect(ui.creditBtn, SIGNAL(clicked()), this, SLOT(slotPaymentMethodSelected()));
	QObject::connect(ui.chequeBtn, SIGNAL(clicked()), this, SLOT(slotPaymentMethodSelected()));
	QObject::connect(ui.creditCardBtn, SIGNAL(clicked()), this, SLOT(slotPaymentMethodSelected()));
	QObject::connect(ui.loyalityCardBtn, SIGNAL(clicked()), this, SLOT(slotPaymentMethodSelected()));
	QObject::connect(m_removeButtonSignalMapper, SIGNAL(mapped(int)), this, SLOT(slotRemove(int)));

	QObject::connect(ui.cashText, SIGNAL(textChanged(QString)), this, SLOT(slotCalculateBalance()));
	QObject::connect(ui.interestTxt, SIGNAL(textChanged(QString)), this, SLOT(slotCalculateInterest()));
	QObject::connect(ui.addBtn, SIGNAL(clicked()), this, SLOT(slotAdd()));
	QObject::connect(ui.okBtn, SIGNAL(clicked()), this, SLOT(slotOk()));

	new QShortcut(QKeySequence(Qt::Key_Escape), this, SLOT(close()));
	setMinimumWidth(900);
	setMinimumHeight(550);
	//resize(900, 1);
	adjustSize();
	ui.dateEdit->setDate(QDate::currentDate());
}

ESMultiplePayment::~ESMultiplePayment()
{

}

void ESMultiplePayment::setCustomerId(QString customerId)
{

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
}


void ESMultiplePayment::slotPaymentMethodSelected()
{
	if (ui.creditBtn == sender())
	{
		ui.lbl1->hide();
		ui.interestLbl->show();
		ui.dateLbl->show(); // due date

		ui.txt1->hide();
		ui.interestTxt->show();
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

		ui.lbl1->setText("Cheque No. :  ");
		ui.lbl2->setText("Bank :  ");

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
		ui.dateEdit->hide();

		ui.lbl1->setText("Card No. :  ");
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

		ui.lbl1->setText("Card No. :  ");
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

		ui.paymentType->setText("Cash :  ");

		ui.creditBtn->setChecked(false);
		ui.chequeBtn->setChecked(false);
		ui.creditCardBtn->setChecked(false);
		ui.loyalityCardBtn->setChecked(false);
		m_paymentType = "CASH";
	}

	//resize(900, 1);
	adjustSize();
}

void ESMultiplePayment::slotAdd()
{
	int row = ui.tableWidget->rowCount();
	ui.tableWidget->insertRow(row);
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
			mbox.setText(QString("Invalid input - Cash"));
			mbox.exec();
			ui.cashText->clear();
			return;
		}
	}

	double netAmount = ui.netAmountLbl->text().toDouble();
	double remaining = netAmount - amountPaid;
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
	}

	if (m_paymentType == "CASH")
	{
		ui.tableWidget->setItem(row, 0, new QTableWidgetItem(m_paymentType));
		ui.tableWidget->setItem(row, 1, new QTableWidgetItem(QString::number(amountPaid, 'f', 2)));
		ui.tableWidget->setItem(row, 2, new QTableWidgetItem("-/-"));
		ui.tableWidget->setItem(row, 3, new QTableWidgetItem("-/-"));
		ui.tableWidget->setItem(row, 4, new QTableWidgetItem("-/-"));
		ui.tableWidget->setItem(row, 5, new QTableWidgetItem("-/-"));
	}
	else if (m_paymentType == "CREDIT")
	{
		ui.tableWidget->setItem(row, 0, new QTableWidgetItem(m_paymentType));
		ui.tableWidget->setItem(row, 1, new QTableWidgetItem(QString::number(amountPaid, 'f', 2)));
		ui.tableWidget->setItem(row, 2, new QTableWidgetItem(ui.interestTxt->text()));
		ui.tableWidget->setItem(row, 3, new QTableWidgetItem(ui.dateEdit->text()));
		ui.tableWidget->setItem(row, 4, new QTableWidgetItem("-/-"));
		ui.tableWidget->setItem(row, 5, new QTableWidgetItem("-/-"));
	}
	else if (m_paymentType == "CHEQUE")
	{
		ui.tableWidget->setItem(row, 0, new QTableWidgetItem(m_paymentType));
		ui.tableWidget->setItem(row, 1, new QTableWidgetItem(QString::number(amountPaid, 'f', 2)));
		ui.tableWidget->setItem(row, 2, new QTableWidgetItem(ui.interestTxt->text()));
		ui.tableWidget->setItem(row, 3, new QTableWidgetItem(ui.dateEdit->text()));
		ui.tableWidget->setItem(row, 4, new QTableWidgetItem(ui.txt1->text()));
		ui.tableWidget->setItem(row, 5, new QTableWidgetItem(ui.txt2->text()));
	}
	else if (m_paymentType == "CARD")
	{
		ui.tableWidget->setItem(row, 0, new QTableWidgetItem(m_paymentType));
		ui.tableWidget->setItem(row, 1, new QTableWidgetItem(QString::number(amountPaid, 'f', 2)));
		ui.tableWidget->setItem(row, 2, new QTableWidgetItem(ui.interestTxt->text()));
		ui.tableWidget->setItem(row, 3, new QTableWidgetItem("-/-"));
		ui.tableWidget->setItem(row, 4, new QTableWidgetItem(ui.txt1->text()));
		ui.tableWidget->setItem(row, 5, new QTableWidgetItem("-/-"));
	}
	else if (m_paymentType == "LOYALTY")
	{
		ui.tableWidget->setItem(row, 0, new QTableWidgetItem(m_paymentType));
		ui.tableWidget->setItem(row, 1, new QTableWidgetItem(QString::number(amountPaid, 'f', 2)));
		ui.tableWidget->setItem(row, 2, new QTableWidgetItem(ui.interestTxt->text()));
		ui.tableWidget->setItem(row, 3, new QTableWidgetItem("-/-"));
		ui.tableWidget->setItem(row, 4, new QTableWidgetItem(ui.txt1->text()));
		ui.tableWidget->setItem(row, 5, new QTableWidgetItem("-/-"));
	}
	
	m_removeButtonSignalMapper->setMapping(removeBtn, row);
	QObject::connect(removeBtn, SIGNAL(clicked()), m_removeButtonSignalMapper, SLOT(map()));
	ui.tableWidget->setCellWidget(row, 6, removeBtn);
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

			ui.netAmountLbl->setText(QString::number((amountPaid + netAmount), 'f', 2));
			ui.addBtn->setEnabled(true);

			ui.tableWidget->removeRow(i);
			ui.cashText->setFocus();
			break;
		}
	}
	
}

void ESMultiplePayment::slotOk()
{
	int rowCount = ui.tableWidget->rowCount();
	for (int i = 0; i < rowCount; ++i)
	{
		QTableWidgetItem* item = ui.tableWidget->item(i, 7);
	}
}

void ESMultiplePayment::slotCalculateInterest()
{
	bool isValid = false;
	double interest = 0.0;
	QString interstStr = ui.interestTxt->text();
	if (!interstStr.isEmpty())
	{
		interest = ui.interestTxt->text().toDouble(&isValid);
		if (!isValid)
		{
			QMessageBox mbox;
			mbox.setIcon(QMessageBox::Warning);
			mbox.setText(QString("Invalid input - Interest"));
			mbox.exec();
			ui.cashText->clear();
			return;
		}
	}

	double netAmount = ui.netAmountLbl->text().toDouble();
	double newAmount = newAmount + netAmount * (interest / 100);
	ui.netAmountLbl->setText(QString::number(newAmount, 'f', 2));

}

