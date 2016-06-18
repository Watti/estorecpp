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
#include "espaymentwidget.h"
#include "entities/tabletextwidget.h"

namespace
{
	QString convertToPriceFormat(QString text, QString iCode)
	{
		QString retVal("0");
 		bool isValid = false;
 		double reducedPrice = text.toDouble(&isValid);
	
		if (isValid)
		{
			QString qryStr("SELECT item_id FROM item WHERE deleted = 0 AND item_code = '"+iCode+"'");
			QSqlQuery itemQuery(qryStr);
			if (itemQuery.next())
			{
				QString itemId = itemQuery.value("item_id").toString();
				QString stockQryStr("SELECT selling_price FROM stock WHERE item_id = '" + itemId+"'");
				QSqlQuery queryStock(stockQryStr);
				if (queryStock.next())
				{
					double sellingPrice = queryStock.value("selling_price").toDouble();
					if (sellingPrice > reducedPrice)
					{
						//TODO error message has to be shown
						reducedPrice = sellingPrice;
					}

				}
			}
		}
		return QString::number(reducedPrice, 'f', 2);

	}

	QString convertToQuantityFormat(QString text, QString itemCode)
	{
// 		double val = text.toDouble();
// 		return QString::number(val, 'f', 3);
		return text;
	}
}

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
	headerLabels.append("Sub Total");
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
	new QShortcut(QKeySequence(Qt::Key_F5), this, SLOT(slotCommit()));
	new QShortcut(QKeySequence(Qt::Key_F6), this, SLOT(slotSuspend()));
	new QShortcut(QKeySequence(Qt::Key_F7), this, SLOT(slotCancel()));
	QObject::connect(timer, SIGNAL(timeout()), this, SLOT(showTime()));
	QObject::connect(ui.tableWidget, SIGNAL(cellDoubleClicked(int, int)), this, SLOT(slotCellDoubleClicked(int, int)));
	QObject::connect(ui.commitButton, SIGNAL(clicked()), this, SLOT(slotCommit()));
	QObject::connect(ui.suspendButton, SIGNAL(clicked()), this, SLOT(slotSuspend()));
	QObject::connect(ui.cancelButton, SIGNAL(clicked()), this, SLOT(slotCancel()));
	QObject::connect(ui.newBillButton, SIGNAL(clicked()), this, SLOT(slotStartNewBill()));
	QObject::connect(ui.addItemButton, SIGNAL(clicked()), this, SLOT(slotShowAddItem()));
	QObject::connect(m_removeButtonSignalMapper, SIGNAL(mapped(QString)), this, SLOT(slotRemoveItem(QString)));

	if (!ES::DbConnection::instance()->open())
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Critical);
		mbox.setText(QString("Cannot connect to the database : ESAddBill::ESAddBill "));
		mbox.exec();
	}

	ui.billIdLabel->setText("###");
	ui.billedByLabel->setText(ES::Session::getInstance()->getUser()->getName());
	ui.branchLabel->setText(ES::Session::getInstance()->getBranchName());
	if (ES::Session::getInstance()->isBillStarted())
	{
		ui.billIdLabel->setText(ES::Session::getInstance()->getBillId());
	}

	// TODO: prompt code should be added
	checkAndContinuePendingBill();

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
	addBillItem->setFocus();
	addBillItem->focus();
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

void ESAddBill::keyPressEvent(QKeyEvent* evt)
{
	switch (evt->key())
	{
	case Qt::Key_Return:
	case Qt::Key_Enter:
	{
		int row = ui.tableWidget->currentRow();
		QString quantity = "";
		QTableWidgetItem* item = ui.tableWidget->item(row, 3);
		if (item)
		{
			quantity = item->text();
		}

		TableTextWidget* textWidget = new TableTextWidget(ui.tableWidget, row, 3, ui.tableWidget);
		QObject::connect(textWidget, SIGNAL(notifyEnterPressed(QString, int, int)), this, SLOT(slotQuantityCellUpdated(QString, int, int)));
		textWidget->setTextFormatterFunc(convertToQuantityFormat);
		textWidget->setText(quantity);
		textWidget->selectAll();
		ui.tableWidget->setCellWidget(row, 3, textWidget);
		textWidget->setFocus();
		break;
	}
	case Qt::Key_Up:
	case Qt::Key_Down:
	case Qt::Key_Left:
	case Qt::Key_Right:
	{
		ui.tableWidget->setFocus();
		break;
	}
	default:
		QWidget::keyPressEvent(evt);
	}
	QWidget::keyPressEvent(evt);
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

			QSqlQuery query("SELECT stock.selling_price, stock.discount, stock.qty FROM stock JOIN sale ON stock.stock_id = sale.stock_id WHERE sale.deleted = 0 AND sale.sale_id = " + saleId);
			if (query.first())
			{
				if (!le->text().isNull() && !le->text().isEmpty())
				{
					bool valid = false;
					double requestedQty = le->text().toDouble(&valid);
					if (!valid)
					{
						QMessageBox mbox;
						mbox.setIcon(QMessageBox::Warning);
						mbox.setText(QString("Please enter a valid number"));
						mbox.exec();
						return;
					}
					double currentQty = query.value("qty").toDouble();
					if (requestedQty > currentQty)
					{
						QMessageBox mbox;
						mbox.setIcon(QMessageBox::Critical);
						mbox.setText(QString("Low stock"));
						mbox.exec();
						return;
					}
				}

				double sellingPrice = query.value("selling_price").toDouble();
				double discount = query.value("discount").toDouble();;
				double subTotal = sellingPrice * quantity * ((100 - discount) / 100.f);

				QString st = QString::number(subTotal, 'f', 2);
				ui.tableWidget->item(row, 5)->setText(st);

				QSqlQuery q("UPDATE sale SET quantity = " + le->text() + ", total = " + st + " WHERE sale_id = " + saleId);
				calculateAndDisplayTotal();
			}
		}
	}
	le->setFocus();
}

void ESAddBill::calculateAndDisplayTotal()
{
	QString q = "SELECT * FROM sale where bill_id= " + ES::Session::getInstance()->getBillId() + " AND deleted = 0";
	QSqlQuery queryAllSales(q);
	double netAmount = 0, grossAmount = 0;
	int noOfItems = 0;
	while (queryAllSales.next())
	{
		double subTot = queryAllSales.value("total").toDouble();
		double discount = queryAllSales.value("discount").toDouble();
		grossAmount += ((subTot * 100) / (100 - discount));
		netAmount += subTot;
		noOfItems++;
	}
	ui.netAmountLabel->setText(QString::number(netAmount, 'f', 2));
	ui.grossAmountLabel->setText(QString::number(grossAmount, 'f', 2));
	ui.noOfItemLabel->setText(QString::number(noOfItems));
}

void ESAddBill::slotCommit()
{
	if (ES::Session::getInstance()->isBillStarted())
	{
		ESPayment* payment = new ESPayment(this, 0);

		payment->setWindowState(Qt::WindowActive);
		payment->setWindowModality(Qt::ApplicationModal);
		payment->setAttribute(Qt::WA_DeleteOnClose);
		//payment->setWindowFlags(Qt::CustomizeWindowHint | Qt::Window);
		payment->show();

		payment->setNetAmount(QString::number(ui.netAmountLabel->text().toDouble(), 'f', 2));
		payment->setNoOfItems(ui.noOfItemLabel->text());
		//payment->getUI().balanceLbl->setText("0.00");

		QSize sz = payment->size();
		QPoint screen = QApplication::desktop()->screen()->rect().center();
		payment->move(screen.x() - sz.width() / 2, screen.y() - sz.height() / 2);
	}

}

void ESAddBill::slotSuspend()
{
	if (ES::Session::getInstance()->isBillStarted())
	{
		QString billId = ES::Session::getInstance()->getBillId();
		QString netAmount = ui.netAmountLabel->text();
		QString paymentType = "1"; // ui.paymentMethodComboBox->currentData().toString();
		QString queryUpdateStr("UPDATE bill set amount = " + netAmount + ", payment_method = " + paymentType + ", status = 2 WHERE bill_id = " + billId);
		QSqlQuery query(queryUpdateStr);
		resetBill();
	}
}

void ESAddBill::slotCancel()
{
	QString pendingBill = "SELECT * FROM bill WHERE deleted = 0 AND status = 2";
	QSqlQuery pendingBillQry(pendingBill);
	if (pendingBillQry.next()/*ES::Session::getInstance()->isBillStarted()*/)
	{
		if (pendingBillQry.value("status").toInt() == 2)
		{
			//There is a pending bill
			QString billId = pendingBillQry.value("bill_id").toString();
			QString netAmount = ui.netAmountLabel->text();
			QString queryUpdateStr("UPDATE bill set amount = " + netAmount + ", status = 3 WHERE bill_id = " + billId);
			QSqlQuery query(queryUpdateStr);
			resetBill();
		}
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
	ui.billIdLabel->setText("###");
	//ui.paymentMethodComboBox->setCurrentIndex(0);
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

		populateTable(queryBillTable);
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

		QString q = "SELECT * FROM sale where bill_id= " + billId + " AND deleted = 0";
		QSqlQuery queryAllSales(q);

		populateTable(queryAllSales);
		calculateAndDisplayTotal();
	}
}

void ESAddBill::slotCellDoubleClicked(int row, int col)
{
	if (col == 2)
	{
		QTableWidgetItem* item = ui.tableWidget->item(row, col);
		QString price = (item) ? item->text() : "";
		TableTextWidget* textWidget = new TableTextWidget(ui.tableWidget, row, col, ui.tableWidget);
		textWidget->setTextFormatterFunc(convertToPriceFormat);
		textWidget->setText(price);
		textWidget->selectAll();
		QObject::connect(textWidget, SIGNAL(notifyEnterPressed(QString, int, int)), this, SLOT(slotSellingPriceUpdated(QString, int, int)));
		ui.tableWidget->setCellWidget(row, col, textWidget);
	}
}

void ESAddBill::checkAndContinuePendingBill()
{
	QString pendingBill = "SELECT * FROM bill WHERE deleted = 0 AND status = 2";
	QSqlQuery pendingBillQry(pendingBill);
	if (pendingBillQry.next()/*ES::Session::getInstance()->isBillStarted()*/)
	{
		if (pendingBillQry.value("status").toInt() == 2 &&
			pendingBillQry.value("user_id").toInt() == ES::Session::getInstance()->getUser()->getId())
		{
			//There is a pending bill
			QString billId = pendingBillQry.value("bill_id").toString();
			ui.billIdLabel->setText(billId);

			ES::Session::getInstance()->startBill();
			ES::Session::getInstance()->setBillId(billId);
			//proceedPendingBill(ES::Session::getInstance()->getBillId());
			int row = ui.tableWidget->rowCount();

			QString q = "SELECT * FROM sale where bill_id= " + billId + " AND deleted = 0";
			QSqlQuery queryAllSales(q);

			populateTable(queryAllSales);
			calculateAndDisplayTotal();
		}
	}
}

void ESAddBill::slotQuantityCellUpdated(QString txt, int row, int col)
{
	QTableWidgetItem* item = ui.tableWidget->item(row, 7);
	if (item)
	{
		QString saleId = item->text();
		double quantity = txt.toDouble();

		QSqlQuery query("SELECT stock.discount, stock.qty, sale.item_price FROM stock JOIN sale ON stock.stock_id = sale.stock_id WHERE sale.deleted = 0 AND sale.sale_id = " + saleId);
		if (query.first())
		{
			if (!txt.isNull() && !txt.isEmpty())
			{
				bool valid = false;
				double requestedQty = txt.toDouble(&valid);
				if (!valid)
				{
					QMessageBox mbox;
					mbox.setIcon(QMessageBox::Warning);
					mbox.setText(QString("Please enter a valid number"));
					mbox.exec();
					return;
				}
				double currentQty = query.value("qty").toDouble();
				if (requestedQty > currentQty)
				{
					QMessageBox mbox;
					mbox.setIcon(QMessageBox::Critical);
					mbox.setText(QString("Low stock"));
					mbox.exec();
					return;
				}
			}

			double sellingPrice = query.value("item_price").toDouble();
			double discount = query.value("discount").toDouble();;
			double subTotal = sellingPrice * quantity * ((100 - discount) / 100.f);

			QString st = QString::number(subTotal, 'f', 2);
			ui.tableWidget->item(row, 5)->setText(st);

			QSqlQuery q("UPDATE sale SET quantity = " + txt + ", total = " + st + " WHERE sale_id = " + saleId);
			calculateAndDisplayTotal();
		}
	}

}

void ESAddBill::populateTable(QSqlQuery &queryBillTable)
{
	int row = ui.tableWidget->rowCount();
	while (queryBillTable.next())
	{
		row = ui.tableWidget->rowCount();
		ui.tableWidget->insertRow(row);

		QString saleId = queryBillTable.value("sale_id").toString();
		QString stockId = queryBillTable.value("stock_id").toString();
		double discount = queryBillTable.value("discount").toDouble();
		double itemPrice = queryBillTable.value("item_price").toDouble();
		double quantity = queryBillTable.value("quantity").toDouble();

		QSqlQuery queryItem("SELECT item.*, stock.selling_price, stock.discount FROM item JOIN stock ON item.item_id = stock.item_id WHERE stock.stock_id = " + stockId);
		if (queryItem.first())
		{
			ui.tableWidget->setItem(row, 0, new QTableWidgetItem(queryItem.value("item_code").toString()));
			ui.tableWidget->setItem(row, 1, new QTableWidgetItem(queryItem.value("item_name").toString()));

			QTableWidgetItem* sellingPriceItem = new QTableWidgetItem();
			sellingPriceItem->setTextAlignment(Qt::AlignRight);
			sellingPriceItem->setText(QString::number(itemPrice, 'f', 2));
			ui.tableWidget->setItem(row, 2, sellingPriceItem);

			QTableWidgetItem* quentityItem = new QTableWidgetItem();
			quentityItem->setTextAlignment(Qt::AlignRight);
			quentityItem->setText(QString::number(quantity));
			ui.tableWidget->setItem(row, 3, quentityItem);

			QTableWidgetItem* discountItem = new QTableWidgetItem();
			discountItem->setTextAlignment(Qt::AlignRight);
			discountItem->setText(QString::number(discount, 'f', 2));
			ui.tableWidget->setItem(row, 4, discountItem);

			QTableWidgetItem* totalItem = new QTableWidgetItem();
			totalItem->setTextAlignment(Qt::AlignRight);
			totalItem->setText(QString::number(queryBillTable.value("total").toDouble(), 'f', 2));
			ui.tableWidget->setItem(row, 5, totalItem);

			QWidget* base = new QWidget(ui.tableWidget);
			QPushButton* removeBtn = new QPushButton(base);
			removeBtn->setIcon(QIcon("icons/delete.png"));
			removeBtn->setIconSize(QSize(24, 24));
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
			ui.tableWidget->setItem(row, 7, new QTableWidgetItem(saleId));
		}
	}
}

void ESAddBill::slotSellingPriceUpdated(QString txt, int row, int col)
{
	QTableWidgetItem* item = ui.tableWidget->item(row, 7);
	if (item)
	{
		QString saleId = item->text();
		double itemPrice = txt.toDouble();
		QSqlQuery saleQtyQuery("SELECT quantity FROM sale WHERE sale_id = " + saleId + " AND deleted = 0");
		if (saleQtyQuery.next())
		{
			double qty = saleQtyQuery.value("quantity").toDouble();
			double total = qty * itemPrice;
			QSqlQuery updateQuery;
			updateQuery.prepare("UPDATE sale SET item_price = ?, total = ? WHERE sale_id = ?");
			updateQuery.addBindValue(itemPrice);
			updateQuery.addBindValue(total);
			updateQuery.addBindValue(saleId);
			updateQuery.exec();

			QTableWidgetItem* totalItem = new QTableWidgetItem();
			totalItem->setTextAlignment(Qt::AlignRight);
			totalItem->setText(QString::number(total, 'f', 2));
			ui.tableWidget->setItem(row, 5, totalItem);

			calculateAndDisplayTotal();
		}
	}
}
