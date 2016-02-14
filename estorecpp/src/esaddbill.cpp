#include "esaddbill.h"
#include "esaddbillitem.h"
#include "utility/session.h"
#include <QShortcut>
#include <QDesktopWidget>
#include <QApplication>
#include <QSqlQuery>
#include "QDateTime"
#include "QTimer"
#include "utility/esdbconnection.h"
#include "QMessageBox"

ESAddBill::ESAddBill(QWidget *parent)
:QWidget(parent)
{
	ui.setupUi(this);

	QStringList headerLabels;
	headerLabels.append("Code");
	headerLabels.append("Item");
	headerLabels.append("Price");
	headerLabels.append("Qty");
	headerLabels.append("Discount");
	headerLabels.append("Amount");
	headerLabels.append("Actions");
	headerLabels.append("Sale_ID");

	ui.tableWidget->setHorizontalHeaderLabels(headerLabels);
	ui.tableWidget->horizontalHeader()->setStretchLastSection(true);
	ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.tableWidget->hideColumn(7);

	if (!ES::DbConnection::instance()->open())
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Critical);
		mbox.setText(QString("Cannot connect to the database : ESAddBill::ESAddBill "));
		mbox.exec();
	}
	else
	{
		QSqlQuery queryPayment("SELECT * FROM payment");
		QStringList catogory;
		while (queryPayment.next())
		{
			ui.paymentMethodComboBox->addItem(queryPayment.value("type").toString(), queryPayment.value("type_id").toInt());
		}
	}

	ui.billedByLabel->setText(ES::Session::getInstance()->getUser()->getName());
	ui.branchLabel->setText("NUGEGODA");
	ui.billIdLabel->setText("###");

	QTimer *timer = new QTimer(this);
	timer->start(1000);

	new QShortcut(QKeySequence(Qt::Key_F4), this, SLOT(slotShowAddItem()));
	new QShortcut(QKeySequence(Qt::Key_F3), this, SLOT(slotStartNewBill()));
	connect(timer, SIGNAL(timeout()), this, SLOT(showTime()));
	connect(ui.tableWidget, SIGNAL(cellClicked(int, int)), this, SLOT(slotEdit(int, int)));
	QObject::connect(ui.commitButton, SIGNAL(clicked()), this, SLOT(slotCommit()));
	QObject::connect(ui.suspendButton, SIGNAL(clicked()), this, SLOT(slotSuspend()));
	QObject::connect(ui.cancelButton, SIGNAL(clicked()), this, SLOT(slotCancel()));
}

ESAddBill::~ESAddBill()
{

}

void ESAddBill::slotShowAddItem()
{
	if (!ES::Session::getInstance()->isBillStarted())
		return;

	QRect rec = QApplication::desktop()->screenGeometry();
	int width = rec.width();
	int height = rec.height();

	width -= 200;
	height -= 200;

	ESAddBillItem* addBillItem = new ESAddBillItem(this, this);
	addBillItem->resize(QSize(width, height));
	addBillItem->setWindowState(Qt::WindowActive);
	addBillItem->setWindowModality(Qt::ApplicationModal);
	addBillItem->setAttribute(Qt::WA_DeleteOnClose);
	addBillItem->setWindowFlags(Qt::CustomizeWindowHint | Qt::Window);
	addBillItem->show();
}

void ESAddBill::slotStartNewBill()
{

	if (!ES::Session::getInstance()->isBillStarted())
	{
		ES::Session::getInstance()->startBill();
		QString insertBillQuery("INSERT INTO Bill (user_id) VALUES(");
		QString userID;
		userID.setNum(ES::Session::getInstance()->getUser()->getId());
		insertBillQuery.append(userID);
		insertBillQuery.append(")");

		QSqlQuery insertNewBill(insertBillQuery);
		int id = (insertNewBill.lastInsertId()).value<int>();

		QString billID;
		billID.setNum(id);
		ES::Session::getInstance()->setBillId(billID);
		ui.billIdLabel->setText(billID);
		ui.billedByLabel->setText(ES::Session::getInstance()->getUser()->getName());
		ui.branchLabel->setText("NUGEGODA");
	}
}

void ESAddBill::showTime()
{
	ui.dateLabel->setText(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
}

Ui::AddBillWidget& ESAddBill::getUI()
{
	return ui;
}

void ESAddBill::slotSearch()
{

}

void ESAddBill::slotReturnPressed(QString saleId, int row)
{
	QLineEdit* le = qobject_cast<QLineEdit*>(sender());
	if (le)
	{
		if (le->isReadOnly())
		{
			le->setReadOnly(false);
		}
		else
		{
			le->setReadOnly(true);
			float quantity = le->text().toFloat();

			QSqlQuery saleQuery("SELECT st.item_id FROM stock st, sale s WHERE s.stock_id = st.stock_id AND s.sale_id = " + saleId);
			if (saleQuery.first())
			{
				QString itemId = saleQuery.value("item_id").toString();
				QString qStr("SELECT * from stock_order WHERE item_id = "+itemId);
				QSqlQuery sOrderQuery(qStr);
				if (sOrderQuery.first())
				{
					float uPrice = sOrderQuery.value("selling_price").toFloat();
					float dicount = sOrderQuery.value("discount_type").toFloat();

					float subTotal = uPrice*quantity * ((100 - dicount) / 100.f);
					QString st = QString::number(subTotal, 'f', 2);
					ui.tableWidget->item(row, 5)->setText(st);

					QString q = "UPDATE sale SET quantity = " + le->text() + ", total = " + st + " WHERE sale_id = " + saleId;
					QSqlQuery query(q);					
				}
			}

		}
		calculateAndDisplayTotal();
	}
	le->setFocus();
}

void ESAddBill::calculateAndDisplayTotal()
{
	QString q = "SELECT * FROM sale where bill_id= " + ES::Session::getInstance()->getBillId();
	QSqlQuery queryAllSales(q);
	float netAmount = 0;
	int noOfItems = 0;
	while (queryAllSales.next())
	{
		netAmount += queryAllSales.value("total").toFloat();
		noOfItems++;
	}
	ui.netAmountLabel->setText(QString::number(netAmount, 'f', 2));
	ui.noOfItemLabel->setText(QString::number(noOfItems));
}

void ESAddBill::slotCommit()
{
	if (ES::Session::getInstance()->isBillStarted())
	{
		QString billId = ES::Session::getInstance()->getBillId();
		QString netAmount = ui.netAmountLabel->text();
		QString paymentType = ui.paymentMethodComboBox->currentData().toString();
		QString queryUpdateStr("UPDATE bill set amount = " + netAmount + ", payment_method = "+paymentType+" , status = 1 WHERE bill_id = " + billId);
		QSqlQuery query(queryUpdateStr);
		resetBill();
	}

}

void ESAddBill::slotSuspend()
{
	if (ES::Session::getInstance()->isBillStarted())
	{
		QString billId = ES::Session::getInstance()->getBillId();
		QString netAmount = ui.netAmountLabel->text();
		QString paymentType = ui.paymentMethodComboBox->currentData().toString();
		QString queryUpdateStr("UPDATE bill set amount = " + netAmount + ", payment_method = " + paymentType + ", status = 3 WHERE bill_id = " + billId);
		QSqlQuery query(queryUpdateStr);
		resetBill();
	}
}

void ESAddBill::slotCancel()
{
	if (ES::Session::getInstance()->isBillStarted())
	{
		QString billId = ES::Session::getInstance()->getBillId();
		QString netAmount = ui.netAmountLabel->text();
		QString queryUpdateStr("UPDATE bill set amount = " + netAmount + ", deleted = 1 WHERE bill_id = " + billId); QSqlQuery query(queryUpdateStr);
		resetBill();
	}
}

void ESAddBill::resetBill()
{
	ES::Session::getInstance()->setBillId("");
	ES::Session::getInstance()->endBill();
	while (ui.tableWidget->rowCount() > 0)
	{
		ui.tableWidget->removeRow(0);
	}
	ui.noOfItemLabel->setText("0");
	ui.netAmountLabel->setText("0.00");
	ui.billIdLabel->setText("xxx");
	ui.paymentMethodComboBox->setCurrentIndex(0);
}
