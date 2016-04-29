#include "esaddmanualstockitemswidget.h"
#include "utility/esmainwindowholder.h"
#include "utility/esdbconnection.h"
#include "utility/utility.h"
#include "utility/session.h"
#include <QMessageBox>

ESAddManualStockItems::ESAddManualStockItems(QWidget *parent /*= 0*/)
{
	ui.setupUi(this);
	ui.itemIdText->setVisible(false);

	QStringList headerLabels;
	headerLabels.append("Item ID");
	headerLabels.append("Item Code");
	headerLabels.append("Item Name");
	headerLabels.append("Category");
	headerLabels.append("Unit");
	headerLabels.append("Description");

	ui.tableWidget->setHorizontalHeaderLabels(headerLabels);
	ui.tableWidget->horizontalHeader()->setStretchLastSection(true);
	ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.tableWidget->hideColumn(0);

	QObject::connect(ui.tableWidget, SIGNAL(cellClicked(int, int)), this, SLOT(slotItemSelected(int, int)));
	QObject::connect(ui.addToStockBtn, SIGNAL(clicked()), this, SLOT(slotAddToStock()));

	if (!ES::DbConnection::instance()->open())
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Critical);
		mbox.setText(QString("Cannot connect to the database : ESManageStockItems::displayStockItems "));
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

		slotSearch();
	}
}

ESAddManualStockItems::~ESAddManualStockItems()
{

}

void ESAddManualStockItems::slotSearch()
{
	while (ui.tableWidget->rowCount() > 0)
	{
		ui.tableWidget->removeRow(0);
	}

	QString text = ui.searchTextBox->text();
	QString selectedCategory = ui.categoryComboBox->currentText();

	QString q;
	q.append("SELECT item.item_id, item.item_code, item.item_name, item_category.itemcategory_name, item.unit, item.description FROM item JOIN item_category ON item.itemcategory_id = item_category.itemcategory_id WHERE item.deleted = 0 ");

	if (!text.isEmpty())
	{
		q.append("AND (item.item_code LIKE '%");
		q.append(text);
		q.append("%' OR item.item_name LIKE '%");
		q.append(text);
		q.append("%' OR item.description LIKE '%");
		q.append(text);
		q.append("%') ");
	}
	if (!selectedCategory.isEmpty() && selectedCategory != DEFAULT_DB_COMBO_VALUE)
	{
		q.append("AND item_category.itemcategory_name = '");
		q.append(selectedCategory);
		q.append("' ");
	}

	QSqlQuery query;
	if (!query.exec(q))
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Critical);
		mbox.setText(QString("Something went wrong: Cannot acquire stock data"));
		mbox.exec();
	}
	else
	{
		ui.tableWidget->setSortingEnabled(false);
		while (query.next())
		{
			int row = ui.tableWidget->rowCount();
			ui.tableWidget->insertRow(row);

			QString itemId = query.value("item_id").toString();

			ui.tableWidget->setItem(row, 0, new QTableWidgetItem(itemId));
			ui.tableWidget->setItem(row, 1, new QTableWidgetItem(query.value("item_code").toString()));
			ui.tableWidget->setItem(row, 2, new QTableWidgetItem(query.value("item_name").toString()));
			ui.tableWidget->setItem(row, 3, new QTableWidgetItem(query.value("itemcategory_name").toString()));
			ui.tableWidget->setItem(row, 4, new QTableWidgetItem(query.value("unit").toString()));
			ui.tableWidget->setItem(row, 5, new QTableWidgetItem(query.value("description").toString()));
		}
		ui.tableWidget->setSortingEnabled(true);
	}

}

void ESAddManualStockItems::slotItemSelected(int row, int col)
{
	QTableWidgetItem* idCell = ui.tableWidget->item(row, 0);
	if (!idCell)
		return;

	QString itemId = idCell->text();
	QSqlQuery queryItem("SELECT * FROM item WHERE deleted = 0 AND item_id = " + itemId);
	if (queryItem.next())
	{
		ui.itemCode->setText(queryItem.value("item_code").toString());
	}

	QSqlQuery queryStock("SELECT * FROM stock WHERE deleted = 0 AND item_id = " + itemId);
	if (queryStock.next())
	{
		ui.minQty->setText(queryStock.value("min_qty").toString());
	}
	ui.itemIdText->setText(itemId);
}

void ESAddManualStockItems::slotAddToStock()
{
	if (ui.itemIdText->text().isEmpty())
	{
		return;
	}
	QString itemId = ui.itemIdText->text();
	QString sellingPrice = ui.sellingPrice->text();
	QString discount = ui.discount->text();

	double currentQty = ui.qty->text().toDouble();
	int userId = ES::Session::getInstance()->getUser()->getId();
	QString userIdStr;
	userIdStr.setNum(userId);

	QSqlQuery itemStock("SELECT * FROM stock WHERE deleted = 0 AND item_id = " + itemId);
	if (itemStock.next())
	{
		QString stockId = itemStock.value("stock_id").toString();
		sellingPrice = itemStock.value("selling_price").toString();
		currentQty += itemStock.value("qty").toDouble();

		QString qtyStr;
		qtyStr.setNum(currentQty);
		QSqlQuery q("UPDATE stock SET qty = " + qtyStr + " WHERE stock_id = " + stockId);
	}
	else
	{
		QString qtyStr;
		qtyStr.setNum(currentQty);
		QString q("INSERT INTO stock (item_id, qty, selling_price, discount, user_id) VALUES (" +
			itemId + "," + qtyStr + "," + sellingPrice + "," + discount + "," + userIdStr + ")");
		QSqlQuery query;
		if (query.exec(q))
		{
			QMessageBox mbox;
			mbox.setIcon(QMessageBox::Information);
			mbox.setText(QString("Success"));
			mbox.exec();
		}
		else
		{
			QMessageBox mbox;
			mbox.setIcon(QMessageBox::Critical);
			mbox.setText(QString("Something goes wrong: stock update failed"));
			mbox.exec();
		}
	}

}
