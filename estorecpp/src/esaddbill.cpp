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
#include "utility/utility.h"

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

	QTimer *timer = new QTimer(this);
	timer->start(1000);

	m_removeButtonSignalMapper = new QSignalMapper(this);

	new QShortcut(QKeySequence(Qt::Key_F4), this, SLOT(slotShowAddItem()));
	new QShortcut(QKeySequence(Qt::Key_F3), this, SLOT(slotStartNewBill()));
	QObject::connect(timer, SIGNAL(timeout()), this, SLOT(showTime()));
	QObject::connect(ui.tableWidget, SIGNAL(cellClicked(int, int)), this, SLOT(slotEdit(int, int)));
	QObject::connect(ui.commitButton, SIGNAL(clicked()), this, SLOT(slotCommit()));
	QObject::connect(ui.suspendButton, SIGNAL(clicked()), this, SLOT(slotSuspend()));
	QObject::connect(ui.cancelButton, SIGNAL(clicked()), this, SLOT(slotCancel()));
	QObject::connect(m_removeButtonSignalMapper, SIGNAL(mapped(QString)), this, SLOT(slotRemoveItem(QString)));

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
		QString insertBillQuery("INSERT INTO bill (user_id) VALUES(");
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
			QPalette palette;
			palette.setColor(QPalette::Base, QColor(88, 88, 250));
			palette.setColor(QPalette::Text, Qt::white);
			le->setPalette(palette);
			le->setReadOnly(false);
		}
		else
		{
			le->setPalette(parentWidget()->palette());
			le->setReadOnly(true);
			double quantity = le->text().toDouble();

			QSqlQuery query("SELECT stock.selling_price FROM stock JOIN sale ON stock.stock_id = sale.stock_id WHERE sale.deleted = 0");
			if (query.first())
			{
				double sellingPrice = query.value("selling_price").toDouble();
				double discount = 0.0;
				double subTotal = sellingPrice * quantity * ((100 - discount) / 100.f);

				QString st = QString::number(subTotal, 'f', 2);
				ui.tableWidget->item(row, 5)->setText(st);

				QSqlQuery q("UPDATE sale SET quantity = " + le->text() + ", total = " + st + " WHERE sale_id = " + saleId);
			}

		}
		calculateAndDisplayTotal();
	}
	le->setFocus();
}

void ESAddBill::calculateAndDisplayTotal()
{
	QString q = "SELECT * FROM sale where bill_id= " + ES::Session::getInstance()->getBillId() + " AND deleted = 0";
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
		QString queryUpdateStr("UPDATE bill set amount = " + netAmount + ", payment_method = " + paymentType + " , status = 1 WHERE bill_id = " + billId);
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
		QString queryUpdateStr("UPDATE bill set amount = " + netAmount + ", payment_method = " + paymentType + ", status = 2 WHERE bill_id = " + billId);
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
		QString queryUpdateStr("UPDATE bill set amount = " + netAmount + ", status = 3 WHERE bill_id = " + billId);
		QSqlQuery query(queryUpdateStr);
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

void ESAddBill::slotRemoveItem(QString saleId)
{
	if (ES::Utility::verifyUsingMessageBox(this, "EStore", "Do you really want to remove this?"))
	{
		QString billId = ES::Session::getInstance()->getBillId();
		QString queary("UPDATE sale set deleted = 1 WHERE sale_id = " + saleId + " AND bill_id = " + billId);
		QSqlQuery q(queary);

		// Clear table
		while (ui.tableWidget->rowCount() > 0)
		{
			ui.tableWidget->removeRow(0);
		}
		QString qStr = "SELECT * FROM sale WHERE bill_id = " + billId + " AND deleted = 0";
		QSqlQuery queryBillTable(qStr);

		int row = ui.tableWidget->rowCount();
		while (queryBillTable.next())
		{
			row = ui.tableWidget->rowCount();
			ui.tableWidget->insertRow(row);

			QString saleId = queryBillTable.value("sale_id").toString();
			QString stockId = queryBillTable.value("stock_id").toString();

			ui.tableWidget->setItem(row, 5, new QTableWidgetItem(QString::number(queryBillTable.value("total").toFloat(), 'f', 2)));
			QSqlQuery queryItem("SELECT i.* FROM item i , stock s WHERE i.item_id = s.item_id AND s.stock_id = " + stockId);
			if (queryItem.first())
			{
				QString itemCode = queryItem.value("item_code").toString();
				QString itemName = queryItem.value("item_name").toString();

				ui.tableWidget->setItem(row, 0, new QTableWidgetItem(itemCode));
				ui.tableWidget->setItem(row, 1, new QTableWidgetItem(itemName));

				QSqlQuery saleQuery("SELECT st.item_id FROM stock st, sale s WHERE s.stock_id = st.stock_id AND s.sale_id = " + saleId);
				if (saleQuery.first())
				{
					QString itemId = saleQuery.value("item_id").toString();
					QString qStr("SELECT * from stock_order WHERE item_id = " + itemId);
					QSqlQuery sOrderQuery(qStr);
					if (sOrderQuery.first())
					{
						//float uPrice = sOrderQuery.value("selling_price").toFloat();
						ui.tableWidget->setItem(row, 2, new QTableWidgetItem(sOrderQuery.value("selling_price").toString()));
						ui.tableWidget->setItem(row, 4, new QTableWidgetItem(sOrderQuery.value("discount_type").toString()));
						//float discount = sOrderQuery.value("discount_type").toFloat();

					}
				}
				QWidget* base = new QWidget(ui.tableWidget);
				QPushButton* removeBtn = new QPushButton("Remove", base);
				removeBtn->setMaximumWidth(100);

				QObject::connect(removeBtn, SIGNAL(clicked()), m_removeButtonSignalMapper, SLOT(map()));
				m_removeButtonSignalMapper->setMapping(removeBtn, saleId);

				QHBoxLayout *layout = new QHBoxLayout;
				layout->setContentsMargins(0, 0, 0, 0);
				layout->addWidget(removeBtn);
				layout->insertStretch(2);
				base->setLayout(layout);
				ui.tableWidget->setCellWidget(row, 6, base);
				base->show();
				ui.tableWidget->setItem(row, 3, new QTableWidgetItem(queryBillTable.value("quantity").toString()));
				ui.tableWidget->setItem(row, 7, new QTableWidgetItem(saleId));
			}
		}
		calculateAndDisplayTotal();
	}
}

void ESAddBill::proceedPendingBill(QString billId)
{
	if (!ES::Session::getInstance()->isBillStarted())
	{
		ES::Session::getInstance()->startBill();
		QString userID;
		userID.setNum(ES::Session::getInstance()->getUser()->getId());

		ES::Session::getInstance()->setBillId(billId);
		ui.billIdLabel->setText(billId);
		ui.billedByLabel->setText(ES::Session::getInstance()->getUser()->getName());
		//ui.branchLabel->setText("NUGEGODA");
		QString paymentMethodQueryStr("SELECT type, type_id FROM payment WHERE type_id = (SELECT payment_method FROM bill WHERE bill_id = " + billId + ")");
		QSqlQuery paymentQuery(paymentMethodQueryStr);
		while (paymentQuery.next())
		{
			ui.paymentMethodComboBox->setCurrentText(paymentQuery.value("type").toString());
		}
		int row = ui.tableWidget->rowCount();

		QString q = "SELECT * FROM sale where bill_id= " + billId + " AND deleted = 0";
		QSqlQuery queryAllSales(q);
		while (queryAllSales.next())
		{
			QString stockId = queryAllSales.value("stock_id").toString();
			QString qty = queryAllSales.value("quantity").toString();
			QString discount = queryAllSales.value("discount").toString();
			QString amount = queryAllSales.value("total").toString();
			QString saleId = queryAllSales.value("sale_id").toString();
			QString price = "0", itemCode = "", itemName = "";
			q = "SELECT * FROM item i, stock s WHERE i.item_id = s.item_id AND s.stock_id = " + stockId;
			QSqlQuery quaryStockItem(q);
			while (quaryStockItem.next())
			{
				itemCode = quaryStockItem.value("item_code").toString();
				itemName = quaryStockItem.value("item_name").toString();

				q = "SELECT * FROM stock_order WHERE item_id = " + quaryStockItem.value("item_id").toString();
				QSqlQuery stockOrderQuery(q);
				while (stockOrderQuery.next())
				{
					price = stockOrderQuery.value("selling_price").toString();
				}
			}
			row = ui.tableWidget->rowCount();
			ui.tableWidget->insertRow(row);
			ui.tableWidget->setItem(row, 0, new QTableWidgetItem(itemCode));
			ui.tableWidget->setItem(row, 1, new QTableWidgetItem(itemName));
			ui.tableWidget->setItem(row, 2, new QTableWidgetItem(price));
			ui.tableWidget->setItem(row, 3, new QTableWidgetItem(qty));
			ui.tableWidget->setItem(row, 4, new QTableWidgetItem(discount));
			ui.tableWidget->setItem(row, 5, new QTableWidgetItem(amount));

			QWidget* base = new QWidget(ui.tableWidget);
			QPushButton* removeBtn = new QPushButton("Remove", base);
			removeBtn->setMaximumWidth(100);

			QObject::connect(removeBtn, SIGNAL(clicked()), m_removeButtonSignalMapper, SLOT(map()));
			m_removeButtonSignalMapper->setMapping(removeBtn, saleId);

			QHBoxLayout *layout = new QHBoxLayout;
			layout->setContentsMargins(0, 0, 0, 0);
			layout->addWidget(removeBtn);
			layout->insertStretch(2);
			base->setLayout(layout);
			ui.tableWidget->setCellWidget(row, 6, base);
			base->show();
		}
		calculateAndDisplayTotal();
	}
}
