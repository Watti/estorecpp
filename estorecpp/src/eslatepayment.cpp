#include "eslatepayment.h"
#include "QSqlQuery"
#include "entities\tabletextwidget.h"
#include "QMessageBox"
#include "escutomeroutstanding.h"
#include "KDReportsTableElement.h"
#include "KDReportsUnit.h"
#include "KDReportsCell.h"
#include "KDReportsTextElement.h"
#include "utility\session.h"
#include "utility\utility.h"
#include "KDReportsHtmlElement.h"
#include "QPrintPreviewDialog"
#include "QPrinter"
#include "utility\esmainwindowholder.h"
#include "QMainWindow"
#include "esmainwindow.h"

namespace
{
	QString convertToPriceFormat(QString text, int row, int col, QTableWidget* table)
	{
		double val = text.toDouble();
		return QString::number(val, 'f', 2);
	}
}

ESLatePayment::ESLatePayment(QWidget *parent /*= 0*/) : QWidget(parent)
{
	ui.setupUi(this);
	QObject::connect(ui.okBtn, SIGNAL(clicked()), this, SLOT(slotOk()));
	QObject::connect(ui.payingAmountCash, SIGNAL(textChanged(QString)), this, SLOT(slotCalculateCashBalance()));
	QObject::connect(ui.payingAmountCheque, SIGNAL(textChanged(QString)), this, SLOT(slotCalculateChequeBalance()));
}

ESLatePayment::~ESLatePayment()
{

}

void ESLatePayment::slotOk()
{
	float outstandingTotal = 0;
	bool valid = false;
	bool isCheque = false;
	QString chNo = "", dueDate = "", bank = "", remarks = "";
	float payingAmount = 0;
	int index = ui.tabWidget->currentIndex();
	if (index == 0)
	{
		//cash
		payingAmount = ui.payingAmountCash->text().toFloat(&valid);
		if (!valid)
		{
			QMessageBox mbox;
			mbox.setIcon(QMessageBox::Critical);
			mbox.setText(QString("Please enter a number"));
			mbox.exec();
			return;
		}

		remarks = ui.remarksCash->toPlainText();
	}
	else if (index == 1)
	{
		//cheque
		valid = false;
		payingAmount = ui.payingAmountCheque->text().toFloat(&valid);
		chNo = ui.chequeNo->text();
		dueDate = ui.dueDate->date().toString("yyyy-MM-dd");
		bank = ui.bank->text();
		remarks = ui.remarksCheque->toPlainText();
		if (!valid)
		{
			QMessageBox mbox;
			mbox.setIcon(QMessageBox::Critical);
			mbox.setText(QString("Please enter a number"));
			mbox.exec();
			return;
		}
		isCheque = true;
	}
	QString qryStr("SELECT * FROM customer_outstanding WHERE customer_id = " + m_customerId);
	QSqlQuery queryOutstanding(qryStr);
	if (queryOutstanding.next())
	{
		float currentOutstanding = queryOutstanding.value("current_outstanding").toFloat();
		outstandingTotal = currentOutstanding;
		if (payingAmount > currentOutstanding)
		{
			payingAmount = currentOutstanding;
		}
		float newOutstanding = currentOutstanding - payingAmount;
		QSqlQuery qOutstandingUpdate;
		QString updateQryStr("UPDATE customer_outstanding SET current_outstanding = " + QString::number(newOutstanding) + " WHERE customer_id = " + m_customerId);
		if (qOutstandingUpdate.exec(updateQryStr))
		{
			QString payByCash = "1";
			if (isCheque)
			{
				QString qStr("INSERT INTO cheque_information (customer_id, cheque_number, bank, due_date, amount) VALUES (" +
					m_customerId + ",'" + chNo + "','" + bank + "','" + dueDate + "', '" + QString::number(payingAmount) + "')");
				QSqlQuery qryCheckInsert;
				if (!qryCheckInsert.exec(qStr))
				{
					QMessageBox mbox;
					mbox.setIcon(QMessageBox::Critical);
					mbox.setText(QString("Insertion to cheque_information has been failed ! ! !"));
					mbox.exec();
				}
				payByCash = "0";
			}
			QString OSettleQryStr("INSERT INTO customer_outstanding_settlement (customer_id, amount, settled_by_cash) VALUES (" + m_customerId + "," + QString::number(payingAmount)+","+payByCash+")");
			QSqlQuery queryOutstandingSettlement(OSettleQryStr);
			//print the bill

			QString dateStr = "Date : ";
			dateStr.append(QDateTime::currentDateTime().toString("yyyy-MM-dd"));
			QString timeStr = "Time : ";
			timeStr.append(QDateTime::currentDateTime().toString("hh : mm"));

			KDReports::TableElement infoTableElement;
			infoTableElement.setHeaderRowCount(2);
			infoTableElement.setHeaderColumnCount(2);
			infoTableElement.setBorder(0);
			infoTableElement.setWidth(100, KDReports::Percent);

			{
				KDReports::Cell& userNameCell = infoTableElement.cell(0, 0);
				KDReports::TextElement t("Customer : " + ui.customerName->text());
				t.setPointSize(10);
				userNameCell.addElement(t, Qt::AlignLeft);
			}
			{
				QString userName = ES::Session::getInstance()->getUser()->getName();

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

			KDReports::HtmlElement htmlElem;
			QString htm("<div><hr/></div>");
			htmlElem.setHtml(htm);
			report.addElement(htmlElem);

			{
				KDReports::TextElement titleElement("Credit Payment Receipt");
				titleElement.setPointSize(12);
				titleElement.setBold(true);
				report.addElement(titleElement, Qt::AlignCenter);
			}

			report.addElement(htmlElem);
			report.addVerticalSpacing(2);
			int pointSizeForPayement = 10;
			{
				KDReports::TextElement textElm("Amount Paid : " + QString::number(payingAmount, 'f', 2));
				textElm.setPointSize(pointSizeForPayement);
				textElm.setBold(true);
				report.addElement(textElm, Qt::AlignLeft);
			}
			report.addVerticalSpacing(2);
			{
				KDReports::TextElement textElm("Current Due : " + QString::number((outstandingTotal - payingAmount), 'f', 2));
				textElm.setPointSize(9);
				textElm.setBold(false);
				report.addElement(textElm, Qt::AlignLeft);
			}

			report.addVerticalSpacing(1);

			KDReports::TextElement customerInfo2("Thank You!");
			customerInfo2.setPointSize(8);
			report.addElement(customerInfo2, Qt::AlignCenter);

			KDReports::Header& header = report.header(KDReports::FirstPage);


			QString titleStr = ES::Session::getInstance()->getBillTitle();
			{
				KDReports::TextElement element(titleStr);
				element.setPointSize(13);
				element.setBold(false);
				header.addElement(element, Qt::AlignCenter);
			}
			QString addressStr = ES::Session::getInstance()->getBillAddress();
			{
				KDReports::TextElement element(addressStr);
				element.setPointSize(10);
				element.setBold(false);
				header.addElement(element, Qt::AlignCenter);
			}
			QString phoneStr = ES::Session::getInstance()->getBillPhone();
			{
				KDReports::TextElement element(phoneStr);
				element.setPointSize(10);
				element.setBold(false);
				header.addElement(element, Qt::AlignCenter);
			}

			QString emailStr = ES::Session::getInstance()->getBillEmail();
			if (emailStr != "")
			{
				KDReports::TextElement emailElement(emailStr);
				emailElement.setPointSize(10);
				emailElement.setBold(false);
				header.addElement(emailElement, Qt::AlignCenter);
			}

			QPrinter printer;
			printer.setPaperSize(QPrinter::Custom);
			printer.setFullPage(false);
			printer.setOrientation(QPrinter::Portrait);

			//preview start
			QPrintPreviewDialog *dialog = new QPrintPreviewDialog(&printer, this);
			QObject::connect(dialog, SIGNAL(paintRequested(QPrinter*)), this, SLOT(slotPrint(QPrinter*)));
			dialog->setWindowTitle(tr("Print Document"));
			ES::MainWindowHolder::instance()->getMainWindow()->setCentralWidget(dialog);
			dialog->exec();
			//preview end
		}
		else
		{
			QMessageBox mbox;
			mbox.setIcon(QMessageBox::Critical);
			mbox.setText(QString("Error !!! Unable to update the outstanding"));
			mbox.exec();
			return;
		}
		
		this->close();
	}
}

void ESLatePayment::setCustomerId(QString customerId)
{
	m_customerId = customerId;
}

void ESLatePayment::slotCalculateCashBalance()
{
	bool valid = false;
	if (ui.payingAmountCash->text().isEmpty())
	{
		ui.remainingAmountCash->setText(ui.currentOutstandingCash->text());
	}
	float payingAmount = ui.payingAmountCash->text().toFloat(&valid);
	if (valid)
	{
		float remainingAmount = ui.currentOutstandingCash->text().toFloat() - payingAmount;
		ui.remainingAmountCash->setText(QString::number(remainingAmount, 'f', 2));
	}
}

void ESLatePayment::slotCalculateChequeBalance()
{
	bool valid = false;
	if (ui.payingAmountCheque->text().isEmpty())
	{
		ui.remainingAmountCheque->setText(ui.currentOutstandingCheque->text());
	}
	float payingAmount = ui.payingAmountCheque->text().toFloat(&valid);
	if (valid)
	{
		float remainingAmount = ui.currentOutstandingCheque->text().toFloat() - payingAmount;
		ui.remainingAmountCheque->setText(QString::number(remainingAmount, 'f', 2));
	}
}

void ESLatePayment::slotPrint(QPrinter* printer)
{
	report.print(printer);
}
