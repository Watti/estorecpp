#include "esaddbillitem.h"
#include <QKeyEvent>
#include <QShortcut>
#include <QMessageBox>
#include <QSqlQuery>
#include "utility\session.h"
#include "entities\SaleLineEdit.h"
#include "QWidget"

ESAddBillItem::ESAddBillItem(ESAddBill* cart, QWidget *parent)
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
	m_removeButtonSignalMapper = new QSignalMapper(this);

	QObject::connect(m_removeButtonSignalMapper, SIGNAL(mapped(QString)), this, SLOT(slotRemove(QString)));
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
	QString billId = ES::Session::getInstance()->getBillId();
	QString lastInsertedID = "";

	if (qryItems.first())
	{
		QString itemId = qryItems.value("item_id").toString();
		QString itemName = qryItems.value("item_name").toString();

		QString qryStrStockOrder("SELECT * FROM stock_order WHERE item_id = " + itemId);
		QSqlQuery qryStockOrder(qryStrStockOrder);
		if (qryStockOrder.first())
		{
			QString sellingPrice = qryStockOrder.value("selling_price").toString();
			QString discount = qryStockOrder.value("discount_type").toString();
			QString qryStrStock("SELECT * FROM stock WHERE item_id = " + itemId);
			QSqlQuery qryStock(qryStrStock);
			if (qryStock.first())
			{
				QString stockId = qryStock.value("stock_id").toString();				
				QString q = "INSERT INTO sale  (stock_id,  bill_id, discount, deleted) VALUES(" + stockId + ", " + billId + "," + discount + ", 0) ";
				QSqlQuery query;
				if (query.exec(q))
				{
					lastInsertedID = query.lastInsertId().value<QString>();
				}
			}
		}
	}

	// Clear table
	while (m_cart->getUI().tableWidget->rowCount() > 0)
	{
		m_cart->getUI().tableWidget->removeRow(0);
	}
	
	// Populate table
	QString qStr = "SELECT * FROM sale WHERE bill_id = " + billId;
	QSqlQuery queryBillTable(qStr);	

	int row = m_cart->getUI().tableWidget->rowCount();
	while (queryBillTable.next())
	{
		row = m_cart->getUI().tableWidget->rowCount();
		m_cart->getUI().tableWidget->insertRow(row);

		// 						headerLabels.append("Code");
		// 						headerLabels.append("Item");
		// 						headerLabels.append("Price");
		// 						headerLabels.append("Qty");
		// 						headerLabels.append("Discount");
		// 						headerLabels.append("Amount");
		// 						headerLabels.append("Actions");

		QString saleId = queryBillTable.value("sale_id").toString();
		QString stockId = queryBillTable.value("stock_id").toString();
		QSqlQuery queryItem("SELECT i.* FROM item i , stock s WHERE i.item_id = s.item_id AND s.stock_id = " + stockId);
		if (queryItem.first())
		{
			QString itemCode = queryItem.value("item_code").toString();
			QString itemName = queryItem.value("item_name").toString();

			m_cart->getUI().tableWidget->setItem(row, 0, new QTableWidgetItem(itemCode));
			m_cart->getUI().tableWidget->setItem(row, 1, new QTableWidgetItem(itemName));

			QSqlQuery saleQuery("SELECT st.item_id FROM stock st, sale s WHERE s.stock_id = st.stock_id AND s.sale_id = " + saleId);
			if (saleQuery.first())
			{
				QString itemId = saleQuery.value("item_id").toString();
				QString qStr("SELECT * from stock_order WHERE item_id = " + itemId);
				QSqlQuery sOrderQuery(qStr);
				if (sOrderQuery.first())
				{
					//float uPrice = sOrderQuery.value("selling_price").toFloat();
					m_cart->getUI().tableWidget->setItem(row, 2, new QTableWidgetItem(sOrderQuery.value("selling_price").toString()));
					m_cart->getUI().tableWidget->setItem(row, 4, new QTableWidgetItem(sOrderQuery.value("discount_type").toString()));
					//float dicount = sOrderQuery.value("discount_type").toFloat();

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
			m_cart->getUI().tableWidget->setCellWidget(row, 6, base);
			base->show();
			m_cart->getUI().tableWidget->setItem(row, 3, new QTableWidgetItem(queryBillTable.value("quantity").toString()));
			m_cart->getUI().tableWidget->setItem(row, 7, new QTableWidgetItem(saleId));	
		}
	}
	if (row >= 0)
	{
		ES::SaleLineEdit* le = new ES::SaleLineEdit(lastInsertedID);
		m_cart->getUI().tableWidget->setCellWidget(row, 3, le);
		le->setFocus();
		connect(le, SIGNAL(returnPressed()), le, SLOT(slotQuantityUpdate()));
		connect(le, SIGNAL(notifyQuantityUpdate(QString)), m_cart, SLOT(slotReturnPressed(QString)));
	}

	close();
}

void ESAddBillItem::slotRemove(QString)
{

}
