#include "esordercheckin.h"
#include "utility\esdbconnection.h"
#include <QMessageBox>
#include "QDateTime"

ESOrderCheckIn::ESOrderCheckIn(QString orderId, QWidget *parent /*= 0*/)
:QWidget(parent), m_orderId(orderId)
{
	ui.setupUi(this);

	QStringList headerLabels;
	headerLabels.append("Item ID");
	headerLabels.append("Item Code");
	headerLabels.append("Item Name");
	headerLabels.append("Category");
	headerLabels.append("Purchasing Price");
	headerLabels.append("Qty");
	headerLabels.append("Unit");

	ui.itemTableWidget->setHorizontalHeaderLabels(headerLabels);
	ui.itemTableWidget->horizontalHeader()->setStretchLastSection(true);
	ui.itemTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.itemTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.itemTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.itemTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.itemTableWidget->hideColumn(0);

	QObject::connect(ui.itemTableWidget, SIGNAL(cellClicked(int, int)), this, SLOT(slotItemSelected(int, int)));

	if (!ES::DbConnection::instance()->open())
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Critical);
		mbox.setText(QString("Cannot connect to the database : AddItem"));
		mbox.exec();
	}
	else
	{
		QSqlQuery queryCategory("SELECT * FROM item_category WHERE deleted = 0");
		QString catCode = "select";
		int catId = -1;

		ui.categoryComboBox->addItem(catCode, catId);

		while (queryCategory.next())
		{
			catId = queryCategory.value("itemcategory_id").toInt();
			ui.categoryComboBox->addItem(queryCategory.value("itemcategory_name").toString(), catId);
		}

		// Fill order data
		QSqlQuery queryOrder("SELECT * FROM purchase_order WHERE deleted = 0 AND purchaseorder_id = " + m_orderId);
		if (queryOrder.next())
		{
			ui.lblOrderId->setText(m_orderId);
			ui.lblOrderDate->setText(queryOrder.value("order_date").toString());

			QSqlQuery querySupplier("SELECT * FROM supplier WHERE deleted = 0 AND supplier_id = " + queryOrder.value("supplier_id").toString());
			if (querySupplier.next())
			{
				ui.lblSupplierName->setText(querySupplier.value("supplier_name").toString());
			}

			ui.lblArrivedDate->setText(QDate::currentDate().toString("yyyy-MM-dd"));

			// Fill order items
			QSqlQuery queryOrderItems("SELECT * FROM purchase_order_item WHERE deleted = 0 AND purchaseorder_id = " + m_orderId);
			int row = 0;
			while (queryOrderItems.next())
			{
				row = ui.itemTableWidget->rowCount();
				ui.itemTableWidget->insertRow(row);

				QString itemId = queryOrderItems.value("item_id").toString();
				ui.itemTableWidget->setItem(row, 0, new QTableWidgetItem(itemId));
				ui.itemTableWidget->setItem(row, 4, new QTableWidgetItem(queryOrderItems.value("purchasing_price").toString()));
				ui.itemTableWidget->setItem(row, 5, new QTableWidgetItem(queryOrderItems.value("qty").toString()));
				
				QSqlQuery queryItem("SELECT * FROM item WHERE deleted = 0 AND item_id = " + itemId);
				if (queryItem.next())
				{
					ui.itemTableWidget->setItem(row, 1, new QTableWidgetItem(queryItem.value("item_code").toString()));
					ui.itemTableWidget->setItem(row, 2, new QTableWidgetItem(queryItem.value("item_name").toString()));

					QSqlQuery queryCat("SELECT * FROM item_category WHERE deleted = 0 AND itemcategory_id = " + queryItem.value("itemcategory_id").toString());
					if (queryCat.next())
					{
						ui.itemTableWidget->setItem(row, 3, new QTableWidgetItem(queryCat.value("itemcategory_name").toString()));
					}
					ui.itemTableWidget->setItem(row, 6, new QTableWidgetItem(queryItem.value("unit").toString()));
				}
			}
		}
	}
}

ESOrderCheckIn::~ESOrderCheckIn()
{

}

void ESOrderCheckIn::slotAddToStock()
{

}

void ESOrderCheckIn::slotRemoveFromStock()
{

}

void ESOrderCheckIn::slotHoldOrder()
{

}

void ESOrderCheckIn::slotFinalizeOrder()
{

}

void ESOrderCheckIn::slotSearch()
{

}

void ESOrderCheckIn::slotItemSelected(int row, int col)
{
	QTableWidgetItem* idCell = ui.itemTableWidget->item(row, 0);
	if (!idCell)
		return;

	QString itemId = idCell->text();
	QSqlQuery queryItem("SELECT * FROM item WHERE deleted = 0 AND item_id = " + itemId);
	if (queryItem.next())
	{
		ui.itemCode->setText(queryItem.value("item_code").toString());
	}
}
