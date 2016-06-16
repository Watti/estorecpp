#include "escashbalanceconfigure.h"
#include "utility\esdbconnection.h"
#include "QMessageBox"
#include "utility\session.h"
#include "entities\user.h"
#include "QDateTime"
#include "QSqlQuery"

ESCashBalanceConfigure::ESCashBalanceConfigure(QWidget *parent) : QWidget(parent)
{
	ui.setupUi(this);
	//ui.labelWelcome->setText("Hello : " + ES::Session::getInstance()->getUser()->getName());
	//ui.label_4->setText(QDate::currentDate().toString("yyyy-MM-dd"));
	ui.typeBox->addItem("Expense", CashType::EXPENSE);
	ui.typeBox->addItem("Income", CashType::INCOME);
	if (!ES::DbConnection::instance()->open())
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Critical);
		mbox.setText(QString("Cannot connect to the database : ESCashBalanceConfigure"));
		mbox.exec();
	}
	//QObject::connect(ui.buttonDayStart, SIGNAL(clicked()), this, SLOT(startDay()));
	//QObject::connect(ui.btnCancel, SIGNAL(clicked()), this, SLOT(exit()));
	QObject::connect(ui.pettyCashOk, SIGNAL(clicked()), this, SLOT(slotPettyCash()));
	QString curDate = QDate::currentDate().toString("yyyy-MM-dd");
	QString selectQryStr("SELECT * FROM cash_config WHERE DATE(date) = '" + curDate + "'");
	QSqlQuery selectQuery(selectQryStr);
// 	if (selectQuery.size() > 0)
// 	{
// 		//ui.buttonDayStart->setDisabled(true);
// 	}
}

ESCashBalanceConfigure::~ESCashBalanceConfigure()
{}

Ui::CashBalanceConfigure& ESCashBalanceConfigure::getUI()
{
	return ui;
}

// void ESCashBalanceConfigure::startDay()
// {
// 	//QString startAmountStr = ui.txtStartAmount->text();
// 	if (startAmountStr != "")
// 	{
// 		bool isNumber = false;
// 		double startAmount = startAmountStr.toDouble(&isNumber);
// 
// 		if (isNumber)
// 		{
// 			QString curDate = QDate::currentDate().toString("yyyy-MM-dd");
// 			QString selectQryStr("SELECT * FROM cash_config WHERE DATE(date) = '" + curDate+"'");
// 			QSqlQuery selectQuery(selectQryStr);
// 			if (selectQuery.size() == 0)
// 			{
// 				int userId = ES::Session::getInstance()->getUser()->getId();
// 				QString insertQueryStr = "INSERT INTO cash_config (user_id, start_amount) VALUES (" + QString::number(userId) + ", " + startAmountStr + ")";
// 				QSqlQuery insertQuery(insertQueryStr);
// 				this->close();
// 			}
// 			else
// 			{
// 				QMessageBox mbox;
// 				mbox.setIcon(QMessageBox::Warning);
// 				mbox.setText(QString("Day has been already started"));
// 				mbox.exec();
// 			}
// 		}
// 		else
// 		{
// 			QMessageBox mbox;
// 			mbox.setIcon(QMessageBox::Warning);
// 			mbox.setText(QString("Please enter a number for Starting Amount"));
// 			mbox.exec();
// 		}
// 	}
// 	else
// 	{
// 		QMessageBox mbox;
// 		mbox.setIcon(QMessageBox::Warning);
// 		mbox.setText(QString("Starting Amount is empty !"));
// 		mbox.exec();
// 	}
// }

void ESCashBalanceConfigure::exit()
{
	this->close();
}

void ESCashBalanceConfigure::slotPettyCash()
{
	QString pCashAmountStr = ui.pettyCashAmount->text();
	QString pCashRemarks = ui.pettyCashRemarks->toPlainText();

	if (pCashAmountStr.isEmpty() || pCashRemarks.isEmpty())
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Warning);
		mbox.setText(QString("Amount/Remarks cannot be empty !"));
		mbox.exec();
		return;
	}
	bool valid = false;
	float amount = pCashAmountStr.toFloat(&valid);
	if (!valid)
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Warning);
		mbox.setText(QString("Invalid input for Amount !"));
		mbox.exec();
		return;
	}
	int userId = ES::Session::getInstance()->getUser()->getId();
	int cType = ui.typeBox->currentData().toInt();
	QSqlQuery query;
	query.prepare("INSERT INTO petty_cash (user_id, amount, remarks, type) VALUES (?, ?, ?, ?)");
	query.addBindValue(userId);
	query.addBindValue(amount);
	query.addBindValue(pCashRemarks);
	query.addBindValue(cType);
	if (!query.exec())
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Warning);
		mbox.setText(QString("Something goes wrong!"));
		mbox.exec();
		return;
	}
}
