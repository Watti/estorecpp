#include "esaddbillitem.h"
#include <QKeyEvent>
#include <QShortcut>
#include <QMessageBox>
#include <QSqlQuery>

ESAddBillItem::ESAddBillItem(QTableWidget* cart, QWidget *parent)
{
	m_cart = cart;
	ui.setupUi(this);
	QStringList headerLabels;
	headerLabels.append("Item Code");
	headerLabels.append("Item Name");
	headerLabels.append("Price");
	headerLabels.append("Discount");

	ui.tableWidget->setHorizontalHeaderLabels(headerLabels);
	ui.tableWidget->horizontalHeader()->setStretchLastSection(true);
	ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);

	new QShortcut(QKeySequence(Qt::Key_Escape), this, SLOT(close()));
	ui.itemText->setFocus();
	ui.itemText->setFocusPolicy(Qt::StrongFocus);

	QObject::connect(ui.itemText, SIGNAL(textChanged(QString)), this, SLOT(slotSearch()));

	slotSearch();
}

ESAddBillItem::~ESAddBillItem()
{

}

void ESAddBillItem::slotSearch()
{
	QString searchText = ui.itemText->text();

	while (ui.tableWidget->rowCount() > 0)
	{
		ui.tableWidget->removeRow(0);
	}

	QSqlQuery queryStocks("SELECT item_id FROM stock");
	while (queryStocks.next())
	{
		QString queryString("SELECT * FROM item WHERE item_id = " + queryStocks.value(0).toString());

		if (!searchText.isEmpty())
		{
			queryString.append(" AND (item_code LIKE '%" + searchText + "%' OR item_name LIKE '%" + searchText + "%')");
		}
		
		QSqlQuery queryItems(queryString);
		int itemId = queryStocks.value(0).toInt();

		while (queryItems.next())
		{
			int row = ui.tableWidget->rowCount();
			ui.tableWidget->insertRow(row);

			ui.tableWidget->setItem(row, 0, new QTableWidgetItem(queryItems.value(2).toString()));
			ui.tableWidget->setItem(row, 1, new QTableWidgetItem(queryItems.value(3).toString()));
			ui.tableWidget->setItem(row, 2, new QTableWidgetItem(""));
			ui.tableWidget->setItem(row, 3, new QTableWidgetItem(""));
		}
	}
	ui.tableWidget->selectRow(0);
}

void ESAddBillItem::keyPressEvent(QKeyEvent * event)
{
	switch (event->key())
	{
	case Qt::Key_Return:
	{
		QList<QTableWidgetItem *> items = ui.tableWidget->selectedItems();
		
		if (!items.empty())
		{
			QTableWidgetItem* item = items.first();
			addToBill(item->text());
		}
	}
		break;
	case Qt::Key_Up:
	case Qt::Key_Down:
	case Qt::Key_Left:
	case Qt::Key_Right:
	{
		ui.tableWidget->setFocus();
		ui.tableWidget->selectRow(0);
	}
		break;
	default:
		ui.itemText->setFocus();
		break;
	}
}

void ESAddBillItem::addToBill(QString itemCode)
{
	QString qryStrItems("SELECT item_id, item_name FROM Item WHERE item_code = '" + itemCode+"'");
	QSqlQuery qryItems(qryStrItems);
	while (qryItems.next())
	{
		QString itemId = qryItems.value("item_id").toString();
		QString itemName = qryItems.value("item_name").toString();

		QString qryStrStock("SELECT * FROM stock_order WHERE item_id = " + itemId);
		QSqlQuery qryStock(qryStrStock);
		//QStringList rowItems;
		while (qryStock.next())
		{

			QString sellingPrice = qryStock.value("selling_price").toString();
			QString discount = qryStock.value("discount_type").toString();
			
		}
	}
}
