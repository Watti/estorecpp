#include "esmultiplepayment.h"
#include "QMessageBox"
#include "QShortcut"

ESMultiplePayment::ESMultiplePayment(QWidget *parent /*= 0*/) : QWidget(parent)
{
	m_customerId = "-1";
	ui.setupUi(this);

	m_removeButtonSignalMapper = new QSignalMapper(this);

	ui.cashBtn->setChecked(true);
	ui.lbl1->hide();
	ui.lbl2->hide();
	ui.dateLbl->hide();

	ui.txt1->hide();
	ui.txt2->hide();
	ui.dateEdit->hide();

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

	ui.tableWidget->setHorizontalHeaderLabels(headerLabels);
	ui.tableWidget->horizontalHeader()->setStretchLastSection(true);
	ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	ui.tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);

	QObject::connect(ui.cashBtn, SIGNAL(clicked()), this, SLOT(slotPaymentMethodSelected()));
	QObject::connect(ui.creditBtn, SIGNAL(clicked()), this, SLOT(slotPaymentMethodSelected()));
	QObject::connect(ui.chequeBtn, SIGNAL(clicked()), this, SLOT(slotPaymentMethodSelected()));
	QObject::connect(ui.creditCardBtn, SIGNAL(clicked()), this, SLOT(slotPaymentMethodSelected()));
	QObject::connect(ui.loyalityCardBtn, SIGNAL(clicked()), this, SLOT(slotPaymentMethodSelected()));

	QObject::connect(ui.cashText, SIGNAL(textChanged(QString)), this, SLOT(slotCalculateBalance()));
	QObject::connect(ui.addBtn, SIGNAL(clicked()), this, SLOT(slotAdd()));

	new QShortcut(QKeySequence(Qt::Key_Escape), this, SLOT(close()));
	setMinimumWidth(900);
	setMinimumHeight(550);
	//resize(900, 1);
	adjustSize();
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


void ESMultiplePayment::slotPaymentMethodSelected()
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
		m_paymentType = "CREDIT";
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
		m_paymentType = "CHEQUE";
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

		m_paymentType = "CARD";
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

		m_paymentType = "LOYALTY";
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

	if (m_paymentType == "CASH")
	{
		ui.tableWidget->setItem(row, 0, new QTableWidgetItem(m_paymentType));
		ui.tableWidget->setItem(row, 1, new QTableWidgetItem(ui.cashText->text()));
	}
	else if (m_paymentType == "CREDIT")
	{
		ui.tableWidget->setItem(row, 0, new QTableWidgetItem(m_paymentType));
		ui.tableWidget->setItem(row, 1, new QTableWidgetItem(ui.cashText->text()));
	}
	else if (m_paymentType == "CHEQUE")
	{
		ui.tableWidget->setItem(row, 0, new QTableWidgetItem(m_paymentType));
		ui.tableWidget->setItem(row, 1, new QTableWidgetItem(ui.cashText->text()));
	}
	else if (m_paymentType == "CARD")
	{
		ui.tableWidget->setItem(row, 0, new QTableWidgetItem(m_paymentType));
		ui.tableWidget->setItem(row, 1, new QTableWidgetItem(ui.cashText->text()));
	}
	else if (m_paymentType == "LOYALTY")
	{
		ui.tableWidget->setItem(row, 0, new QTableWidgetItem(m_paymentType));
		ui.tableWidget->setItem(row, 1, new QTableWidgetItem(ui.cashText->text()));
	}
	
	m_removeButtonSignalMapper->setMapping(removeBtn, row);
	QObject::connect(removeBtn, SIGNAL(clicked()), m_removeButtonSignalMapper, SLOT(map()));
	ui.tableWidget->setCellWidget(row, 6, removeBtn);
}


