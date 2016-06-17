#include "esaddmanualstockitemswidget.h"
#include "utility/esmainwindowholder.h"
#include "utility/esdbconnection.h"
#include "utility/utility.h"
#include "utility/session.h"
#include <QMessageBox>
#include "QString"
#include "easylogging++.h"

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
	QObject::connect(ui.searchTextBox, SIGNAL(textChanged(QString)), this, SLOT(slotSearch()));
	QObject::connect(ui.categoryComboBox, SIGNAL(activated(QString)), this, SLOT(slotSearch()));
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
		q.append("AND (item_category.itemcategory_code LIKE '%");
		q.append(text);
// 		q.append("%' OR item.item_name LIKE '%");
// 		q.append(text);
// 		q.append("%' OR item.description LIKE '%");
// 		q.append(text);
		q.append("%') ");
	}
	if (!selectedCategory.isEmpty() && selectedCategory != DEFAULT_DB_COMBO_VALUE)
	{
		q.append("AND item_category.itemcategory_name = '");
		q.append(selectedCategory);
		q.append("' ");
	}
	if (text.isEmpty())
	{
		q.append(" LIMIT 25");
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
	ui.minQty->setText("0");
	ui.discount->setText("0");
	ui.sellingPrice->setText("0");
	ui.qty->setText("0");

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
		ui.discount->setText(queryStock.value("discount").toString());
		ui.sellingPrice->setText(queryStock.value("selling_price").toString());
		ui.qty->setText(queryStock.value("qty").toString());
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
	QString newSellingPrice = ui.sellingPrice->text();
	if (newSellingPrice == nullptr || newSellingPrice.isEmpty())
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Warning);
		mbox.setText(QString("Selling price cannot be empty"));
		mbox.exec();
		return;
	}
	bool isValid = false;
	newSellingPrice.toDouble(&isValid);
	if (!isValid)
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Warning);
		mbox.setText(QString("Invalid input - Selling Price"));
		mbox.exec();
	}
	QString discount = ui.discount->text();
	if (discount == nullptr || discount.isEmpty())
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Warning);
		mbox.setText(QString("Selling price cannot be empty"));
		mbox.exec();
		return;
	}

	isValid = false;
	discount.toDouble(&isValid);
	if (!isValid)
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Warning);
		mbox.setText(QString("Invalid input - Discount"));
		mbox.exec();
	}
	if (ui.qty->text() == nullptr || ui.qty->text().isEmpty())
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Warning);
		mbox.setText(QString("Quantity cannot be empty"));
		mbox.exec();
		return;
	}
	isValid = false;
	double currentQty = ui.qty->text().toDouble(&isValid);
	if (!isValid)
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Warning);
		mbox.setText(QString("Invalid input - Quantity"));
		mbox.exec();
	}
	if (ui.minQty->text() == nullptr || ui.minQty->text().isEmpty())
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Warning);
		mbox.setText(QString("Min quantity cannot be empty"));
		mbox.exec();
		return;
	}
	isValid = false;
	ui.minQty->text().toDouble(&isValid);
	if (!isValid)
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Warning);
		mbox.setText(QString("Invalid input - Min Quantity"));
		mbox.exec();
	}

	int userId = ES::Session::getInstance()->getUser()->getId();
	QString userIdStr;
	userIdStr.setNum(userId);
	bool success = true;
	QSqlQuery itemStock("SELECT * FROM stock WHERE item_id = " + itemId);
	if (itemStock.next())
	{
		//item is already available in the stock
		QString stockId = itemStock.value("stock_id").toString();
		QString currentSellingPrice = itemStock.value("selling_price").toString();

		//currentQty += itemStock.value("qty").toDouble();

		QString qtyStr;
		qtyStr.setNum(currentQty);
		QString q("UPDATE stock SET deleted = 0, qty = " + qtyStr + " , selling_price = "+ newSellingPrice + " , discount = "
			+ discount + " , min_qty =  " + ui.minQty->text() + " WHERE stock_id = " + stockId);
		QSqlQuery query;
		if (!query.exec(q))
		{
			success = false;
			QMessageBox mbox;
			mbox.setIcon(QMessageBox::Critical);
			mbox.setText(QString("Something goes wrong: stock update failed"));
			mbox.exec();

			QString logError("[ESAddManualStockItems::slotAddToStock] Stock update has been failed query = ");
			logError.append(q);
			LOG(ERROR) << logError.toLatin1().data();
		}
	}
	else
	{
		QString qtyStr;
		qtyStr.setNum(currentQty);
		QString q("INSERT INTO stock (item_id, qty, min_qty, selling_price, discount, user_id) VALUES (" +
			itemId + "," + qtyStr + "," + ui.minQty->text() +","+ newSellingPrice + "," + discount + "," + userIdStr + ")");
		QSqlQuery query;
		if (!query.exec(q))
		{
			success = false;
			QMessageBox mbox;
			mbox.setIcon(QMessageBox::Critical);
			mbox.setText(QString("Something goes wrong: stock update failed"));
			mbox.exec();
			QString logError("[ESAddManualStockItems::slotAddToStock] Stock insert has been failed query =");
			logError.append(q);
			
			LOG(ERROR) << logError.toLatin1().data();
		}
	}
	if (success)
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Information);
		mbox.setText(QString("Item has been added to the stock"));
		mbox.exec();
		ui.minQty->setText("");
		ui.discount->setText("");
		ui.sellingPrice->setText("");
		ui.qty->setText("");
		ui.itemCode->setText("");
	}

}
