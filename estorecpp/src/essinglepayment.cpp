#include "essinglepayment.h"
#include <QMessageBox>
#include "utility\session.h"
#include "QSqlQuery"
#include "QPrintPreviewDialog"
#include "utility\esmainwindowholder.h"
#include "KDReportsTextElement.h"
#include "KDReportsTableElement.h"
#include "KDReportsCell.h"
#include "QShortcut"

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

	new QShortcut(QKeySequence(Qt::Key_Escape), this, SLOT(close()));
	//resize(400, 1);
	adjustSize();
	ui.dateEdit->setDate(QDate::currentDate());
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
	query.prepare("INSERT INTO payment (bill_id, total_amount, payment_type) VALUES (?, ?, 'CASH')");
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
	query.prepare("INSERT INTO payment (bill_id, total_amount, payment_type) VALUES (?, ?, 'CREDIT')");
	query.addBindValue(billId);
	query.addBindValue(netAmount);
	if (query.exec())
	{
		int lastInsertedId = query.lastInsertId().toInt();
		QSqlQuery q;
		q.prepare("INSERT INTO credit (payment_id, amount, interest, due_date) VALUES (?, ?, ?, ?)");
		q.addBindValue(lastInsertedId);
		q.addBindValue(netAmount);
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
	query.addBindValue(netAmount);
	if (query.exec())
	{
		int lastInsertedId = query.lastInsertId().toInt();
		QSqlQuery q;
		q.prepare("INSERT INTO cheque (payment_id, amount, interest, cheque_number, bank, due_date) VALUES (?, ?, ?, ?, ?, ?)");
		q.addBindValue(lastInsertedId);
		q.addBindValue(netAmount);
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
	query.addBindValue(netAmount);
	if (query.exec())
	{
		int lastInsertedId = query.lastInsertId().toInt();
		QSqlQuery q;
		q.prepare("INSERT INTO card (payment_id, amount, interest, card_no) VALUES (?, ?, ?, ?)");
		q.addBindValue(lastInsertedId);
		q.addBindValue(netAmount);
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
	query.addBindValue(netAmount);
	if (query.exec())
	{
		int lastInsertedId = query.lastInsertId().toInt();
		QSqlQuery q;
		q.prepare("INSERT INTO loyalty (payment_id, amount, interest, card_no) VALUES (?, ?, ?, ?)");
		q.addBindValue(lastInsertedId);
		q.addBindValue(netAmount);
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
		ui.lbl1->hide();
		//ui.lbl2->show(); // interest
		ui.dateLbl->show(); // due date
		ui.label_6->hide();
		ui.balanceLbl->hide();

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
	}
	else if (ui.chequeBtn == sender())
	{
		ui.lbl1->show(); // cheque no
		ui.lbl2->show(); // bank
		ui.dateLbl->show(); // due date
		ui.label_6->hide();
		ui.balanceLbl->hide();

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
	}
	else if (ui.creditCardBtn == sender())
	{
		ui.lbl1->show(); // card no
		ui.lbl2->hide();
		ui.dateLbl->hide();
		ui.label_6->hide();
		ui.balanceLbl->hide();

		ui.txt1->show();
		ui.txt2->hide();
		ui.dateEdit->hide();

		ui.lbl1->setText("Card No. :  ");
		ui.paymentType->setText("Amount :  ");

		ui.cashBtn->setChecked(false);
		ui.chequeBtn->setChecked(false);
		ui.creditBtn->setChecked(false);
		ui.loyalityCardBtn->setChecked(false);

		ui.label_4->show();
		ui.lineEdit->show();
	}
	else if (ui.loyalityCardBtn == sender())
	{
		ui.lbl1->show(); // card no
		ui.lbl2->hide();
		ui.dateLbl->hide();
		ui.label_6->hide();
		ui.balanceLbl->hide();

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
	}
	else if (ui.cashBtn == sender())
	{
		ui.lbl1->hide();
		ui.lbl2->hide();
		ui.dateLbl->hide();
		ui.label_6->show();
		ui.balanceLbl->show();

		ui.txt1->hide();
		ui.txt2->hide();
		ui.dateEdit->hide();

		ui.paymentType->setText("Cash :  ");

		ui.creditBtn->setChecked(false);
		ui.chequeBtn->setChecked(false);
		ui.creditCardBtn->setChecked(false);
		ui.loyalityCardBtn->setChecked(false);

		ui.label_4->hide();
		ui.lineEdit->hide();
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

void ESSinglePayment::slotInterestChanged()
{
	double interest = ui.lineEdit->text().toDouble();
	double netAmout = ui.netAmountLbl->text().toDouble();

	double totalBill = netAmout + netAmout * (interest / 100.0);

	ui.netAmountLbl->setText(QString::number(totalBill, 'f', 2));
}

void ESSinglePayment::printBill(int billId, float total)
{
	KDReports::Report report;

	QString dateStr ="Date : ";
	dateStr.append(QDateTime::currentDateTime().toString("yyyy-MM-dd"));
	QString timeStr = "Time : ";
	timeStr.append(QDateTime::currentDateTime().toString("hh : mm"));


	KDReports::TextElement titleElement("HIRUNA MARKETING (PVT) LTD");
	titleElement.setPointSize(15);
	titleElement.setBold(true);
	report.addElement(titleElement, Qt::AlignHCenter);

	report.addVerticalSpacing(10);

	KDReports::TextElement date(dateStr);
	report.addElement(date, Qt::AlignLeft);
	KDReports::TextElement time(timeStr);
	report.addElement(time, Qt::AlignLeft);


	QString querySaleStr("SELECT * FROM sale WHERE bill_id = "+QString::number(billId)+" AND deleted = 0");
	QSqlQuery querySale(querySaleStr);

	//columns (item_code, Description, UnitPrice, Discount, Qty, Sub Total)

	KDReports::TableElement tableElement;
	tableElement.setHeaderRowCount(5);
	tableElement.setHeaderColumnCount(6);
	tableElement.setBorder(0);
	tableElement.setWidth(100, KDReports::Percent);

	KDReports::Cell& cICode = tableElement.cell(0, 0);
	KDReports::TextElement tICode("Item Code");
	tICode.setPointSize(12);
	tICode.setBold(true);
	cICode.addElement(tICode);

	KDReports::Cell& cIName = tableElement.cell(0, 1);
	KDReports::TextElement tEItem("Item");
	tEItem.setBold(true);
	tEItem.setPointSize(12);
	cIName.addElement(tEItem);

	KDReports::Cell& cPrice = tableElement.cell(0, 2);
	KDReports::TextElement tEPrice("Unit Price");
	tEPrice.setPointSize(12);
	tEPrice.setBold(true);
	cPrice.addElement(tEPrice);

	KDReports::Cell& cDiscount = tableElement.cell(0, 3);
	KDReports::TextElement tEDiscount("Discount");
	tEDiscount.setPointSize(12);
	tEDiscount.setBold(true);
	cDiscount.addElement(tEDiscount);

	KDReports::Cell& cQty = tableElement.cell(0, 4);
	KDReports::TextElement tEQty("Qty");
	tEQty.setPointSize(12);
	tEQty.setBold(true);
	cQty.addElement(tEQty);

	KDReports::Cell& cTotal = tableElement.cell(0, 5);
	KDReports::TextElement tETotal("Sub Total");
	tETotal.setPointSize(12);
	tETotal.setBold(true);
	cTotal.addElement(tETotal);

	report.addVerticalSpacing(6);

	int row = 1;
	while (querySale.next())
	{
		QString stockId = querySale.value("stock_id").toString();
		QString discount = QString::number(querySale.value("discount").toDouble(), 'f', 2);
		QString qty = querySale.value("quantity").toString();
		QString subTotal = QString::number(querySale.value("total").toDouble(), 'f', 2);
		QString itemName = "";
		QString unitPrice = "";
		QString itemCode = "";
	

		//get the item name from the item table
		QString qItemStr("SELECT it.item_code, it.item_name , st.selling_price FROM stock st JOIN item it ON st.item_id = it.item_id AND st.stock_id = "+stockId);
		QSqlQuery queryItem(qItemStr);
		if (queryItem.next())
		{
			itemName = queryItem.value("item_name").toString();
			unitPrice = queryItem.value("selling_price").toString();
			itemCode = queryItem.value("item_code").toString();
		}
		//columns (item_code, Description, UnitPrice, Discount, Qty, Sub Total)
		printRow(tableElement, row, 0, itemCode);
		printRow(tableElement, row, 1, itemName);
		printRow(tableElement, row, 2, unitPrice);
		printRow(tableElement, row, 3, discount);
		printRow(tableElement, row, 4, qty);
		printRow(tableElement, row, 5, subTotal);
		row++;
	}

	report.addVerticalSpacing(5);

	KDReports::Cell& totalTextC = tableElement.cell(row, 5);
	KDReports::TextElement totalTxt("Total : ");
	totalTxt.setPointSize(12);
	totalTxt.setBold(true);
	totalTextC.addElement(totalTxt);

	KDReports::Cell& totalCell = tableElement.cell(row, 5);
	KDReports::TextElement totalValue(QString::number(total, 'f', 2));
	totalValue.setPointSize(12);
	totalValue.setBold(true);
	totalCell.addElement(totalValue);

	report.addElement(tableElement);

	QPrinter printer;
	printer.setPaperSize(QPrinter::A4);

	printer.setFullPage(false);
	printer.setOrientation(QPrinter::Portrait);

// 		QPrintPreviewDialog *dialog = new QPrintPreviewDialog(&printer, this);
// 		QObject::connect(dialog, SIGNAL(paintRequested(QPrinter*)), this, SLOT(slotPrint(QPrinter*)));
// 		dialog->setWindowTitle(tr("Print Document"));
// 		ES::MainWindowHolder::instance()->getMainWindow()->setCentralWidget(dialog);
// 		dialog->exec();

	report.print(&printer);
}

void ESSinglePayment::slotPrint(QPrinter* printer)
{
	//report.print(printer);
	
}

void ESSinglePayment::printRow(KDReports::TableElement& tableElement, int row, int col, QString elementStr)
{
	KDReports::Cell& cell = tableElement.cell(row, col);
	KDReports::TextElement te(elementStr);
	te.setPointSize(12);
	cell.addElement(te);
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
