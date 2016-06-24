#include "essecondarydisplay.h"
#include <QSqlQuery>
#include "utility\session.h"

ESSecondaryDisplay::ESSecondaryDisplay(QWidget *parent /*= 0*/) : QWidget(parent)
{
	ui.setupUi(this);

	ui.baseWidget->hide();
	ui.thankYouLabel->show();

	QStringList headerLabels;
	headerLabels.append("Item Name");
	headerLabels.append("Unit Price");
	headerLabels.append("Quantity");
	headerLabels.append("Sub Total");

	ui.tableWidget->setHorizontalHeaderLabels(headerLabels);
	ui.tableWidget->horizontalHeader()->setStretchLastSection(true);
	ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	//ui.tableWidget->hideColumn(7);

	ui.grossAmountLbl->setText("0.00");
	ui.netAmountLbl->setText("0.00");
	ui.onOfItemsLbl->setText("0.00");
}

ESSecondaryDisplay::~ESSecondaryDisplay()
{

}

void ESSecondaryDisplay::slotSearch()
{

}

void ESSecondaryDisplay::startBill()
{
	ui.baseWidget->show();
	ui.thankYouLabel->hide();
}

void ESSecondaryDisplay::endBill()
{
	ui.baseWidget->hide();
	ui.thankYouLabel->show();
	ui.tableWidget->clearContents();
}

void ESSecondaryDisplay::update()
{
	while (ui.tableWidget->rowCount() > 0)
	{
		ui.tableWidget->removeRow(0);
	}

	QString billId = ES::Session::getInstance()->getBillId();

	QSqlQuery q("SELECT * FROM sale WHERE bill_id = " + billId + " AND deleted = 0");
	while (q.next())
	{
		int row = ui.tableWidget->rowCount();
		ui.tableWidget->insertRow(row);

		QSqlQuery q2("SELECT item_id FROM stock WHERE stock_id = " + q.value("stock_id").toString());
		if (q2.next())
		{
			QSqlQuery q3("SELECT item_name FROM item WHERE item_id = " + q2.value("item_id").toString());
			if (q3.next())
			{
				ui.tableWidget->setItem(row, 0, new QTableWidgetItem(q3.value("item_name").toString()));
			}
		}
		
		QTableWidgetItem* itemPrice = new QTableWidgetItem(QString::number(q.value("item_price").toDouble(), 'f', 2));
		itemPrice->setTextAlignment(Qt::AlignRight);
		ui.tableWidget->setItem(row, 1, itemPrice);

		QTableWidgetItem* itemQty = new QTableWidgetItem(QString::number(q.value("quantity").toDouble(), 'f', 2));
		itemQty->setTextAlignment(Qt::AlignRight);
		ui.tableWidget->setItem(row, 2, itemQty);

		QTableWidgetItem* itemTotal = new QTableWidgetItem(QString::number(q.value("total").toDouble(), 'f', 2));
		itemTotal->setTextAlignment(Qt::AlignRight);
		ui.tableWidget->setItem(row, 3, itemTotal);
	}
}

void ESSecondaryDisplay::setBillInfo(QString gross, QString net, QString items)
{
	ui.grossAmountLbl->setText(gross);
	ui.netAmountLbl->setText(net);
	ui.onOfItemsLbl->setText(items);
}

