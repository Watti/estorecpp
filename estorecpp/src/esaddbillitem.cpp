#include "esaddbillitem.h"
#include <QKeyEvent>
#include <QShortcut>
#include <QMessageBox>
#include <QSqlQuery>
#include "utility\session.h"
#include "entities\SaleLineEdit.h"
#include "QWidget"
#include "utility\utility.h"
#include "entities\tabletextwidget.h"
#include "utility\esmainwindowholder.h"
#include "essecondarydisplay.h"

namespace
{
	QString convertToQuantityFormat(QString text, int row, int col, QTableWidget* table)
	{
// 		double val = text.toDouble();
// 		return QString::number(val, 'f', 3);
		return text;
	}
}

ESAddBillItem::ESAddBillItem(ESAddBill* cart, QWidget *parent)
:QWidget(parent)
{
	m_cart = cart;
	ui.setupUi(this);
	QStringList headerLabels;
	headerLabels.append("Stock ID");
	headerLabels.append("Item Code");
	headerLabels.append("Item Name");
	headerLabels.append("Item Image");
	headerLabels.append("Price");
	headerLabels.append("Discount");

	ui.tableWidget->setHorizontalHeaderLabels(headerLabels);
	ui.tableWidget->horizontalHeader()->setStretchLastSection(true);
	ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.tableWidget->hideColumn(0);

	slotSearch();
	slotHideImages();

	new QShortcut(QKeySequence(Qt::Key_Escape), this, SLOT(close()));
	ui.itemText->setFocusPolicy(Qt::StrongFocus);
	setFocus();

	QObject::connect(ui.itemText, SIGNAL(textChanged(QString)), this, SLOT(slotSearch()));
	QObject::connect(ui.hideImages, SIGNAL(stateChanged(int)), this, SLOT(slotHideImages()));

#ifdef HW_TOUCH_SCREEN
	QObject::connect(ui.tableWidget, SIGNAL(cellClicked(int, int)), this, SLOT(slotItemDoubleClicked(int, int)));
#endif

#ifndef SECOND_DISPLAY
	ui.hideImages->setChecked(true);
#endif
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

	QString q;
	//q.append("SELECT stock.stock_id, item.item_code, item.item_name, item.item_image, stock.selling_price FROM item JOIN stock ON item.item_id = stock.item_id WHERE stock.deleted = 0 ");
	q.append("SELECT stock.stock_id, item.item_code, item.item_name, item.item_image, item.itemcategory_id, item_category.itemcategory_code , stock.selling_price, stock.discount FROM item JOIN stock ON item.item_id = stock.item_id  JOIN item_category ON item.itemcategory_id = item_category.itemcategory_id WHERE item.deleted = 0 AND stock.qty > 0");
	if (!searchText.isEmpty())
	{
		//q.append(" AND (item.item_code LIKE '%" + searchText + "%' OR item_category.itemcategory_code LIKE '%" + searchText + "%')");
		q.append(" AND (item_category.itemcategory_code LIKE '%" + searchText + "%')");//ONLY the items matched with the category code are shown
	}
	else
	{
		q.append(" LIMIT 25");
	}

	ui.tableWidget->setSortingEnabled(false);
	//q.append(" LIMIT 75");
	QSqlQuery queryStocks(q);
	while (queryStocks.next())
	{
		int row = ui.tableWidget->rowCount();
		ui.tableWidget->insertRow(row);

		ui.tableWidget->setItem(row, 0, new QTableWidgetItem(queryStocks.value("stock_id").toString()));
		ui.tableWidget->setItem(row, 1, new QTableWidgetItem(queryStocks.value("item_code").toString()));
		ui.tableWidget->setItem(row, 2, new QTableWidgetItem(queryStocks.value("item_name").toString()));

		QString imagePath = queryStocks.value("item_image").toString();
		if (imagePath.isNull() || imagePath.isEmpty())
		{
			imagePath = "images/default.png";
		}
		QTableWidgetItem* imageItem = new QTableWidgetItem();
		imageItem->setData(Qt::DecorationRole, QPixmap::fromImage(QImage(imagePath)).scaledToHeight(100));
		ui.tableWidget->setItem(row, 3, imageItem);

		QTableWidgetItem* sellingPrice = new QTableWidgetItem();
		sellingPrice->setTextAlignment(Qt::AlignRight);
		double price = queryStocks.value("selling_price").toDouble();
		QString formattedPrice = QString::number(price, 'f', 2);
		sellingPrice->setText(formattedPrice);

		ui.tableWidget->setItem(row, 4, sellingPrice);


		double discount = queryStocks.value("discount").toDouble();
		QString formattedDiscount = QString::number(discount, 'f', 2);
		QTableWidgetItem* discountWidget = new QTableWidgetItem();
		discountWidget->setTextAlignment(Qt::AlignRight);
		discountWidget->setText(formattedDiscount);
		ui.tableWidget->setItem(row, 5, discountWidget);
	}
	ui.tableWidget->setSortingEnabled(true);
	ui.tableWidget->selectRow(0);
	//ui.tableWidget->setFocus();

	ui.itemText->setFocus();
	ui.itemText->setFocusPolicy(Qt::StrongFocus);
}

void ESAddBillItem::keyPressEvent(QKeyEvent * event)
{
	switch (event->key())
	{
	case Qt::Key_Return:
	case Qt::Key_Enter:
	{
		int row = ui.tableWidget->currentRow();
		QTableWidgetItem* idCell = ui.tableWidget->item(row, 0);
		if (!idCell)
		{
			QWidget::keyPressEvent(event);
			return;
		}

		addToBill(idCell->text());
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
		QWidget::keyPressEvent(event);
	}
}

void ESAddBillItem::addToBill(QString stockId)
{
	QString billId = ES::Session::getInstance()->getBillId();
	QString lastInsertedID;
	QSqlQuery queryStock("SELECT discount, selling_price FROM stock WHERE stock_id = " + stockId);
	if (queryStock.next())
	{
		QString discount = queryStock.value("discount").toString();
		QString itemPrice = queryStock.value("selling_price").toString();
		QString q = "INSERT INTO sale (stock_id, bill_id, discount, item_price) VALUES(" + stockId + ", " + 
			billId + ", " + discount + ", " + itemPrice + ")";
		QSqlQuery query;
		if (query.exec(q))
		{
			lastInsertedID = query.lastInsertId().value<QString>();
		}
	}

	// Clear table
	while (m_cart->getUI().tableWidget->rowCount() > 0)
	{
		m_cart->getUI().tableWidget->removeRow(0);
	}
	
	// Populate table
	QString qStr = "SELECT * FROM sale WHERE bill_id = " + billId + " AND deleted = 0";
	QSqlQuery queryBillTable(qStr);	

	int row = m_cart->getUI().tableWidget->rowCount();
	while (queryBillTable.next())
	{
		row = m_cart->getUI().tableWidget->rowCount();
		m_cart->getUI().tableWidget->insertRow(row);

		QString saleId = queryBillTable.value("sale_id").toString();
		QString stockId = queryBillTable.value("stock_id").toString();
		double discount = queryBillTable.value("discount").toDouble();
		double itemPrice = queryBillTable.value("item_price").toDouble();
		double quantity = queryBillTable.value("quantity").toDouble();

		QSqlQuery queryItem("SELECT item.*, stock.selling_price, stock.discount FROM item JOIN stock ON item.item_id = stock.item_id WHERE stock.stock_id = " + stockId);
		if (queryItem.first())
		{
			m_cart->getUI().tableWidget->setItem(row, 0, new QTableWidgetItem(queryItem.value("item_code").toString()));
			m_cart->getUI().tableWidget->setItem(row, 1, new QTableWidgetItem(queryItem.value("item_name").toString()));

			QTableWidgetItem* sellingPriceItem = new QTableWidgetItem();
			sellingPriceItem->setTextAlignment(Qt::AlignRight);
			sellingPriceItem->setText(QString::number(itemPrice, 'f', 2));
			m_cart->getUI().tableWidget->setItem(row, 2, sellingPriceItem);

			QTableWidgetItem* quentityItem = new QTableWidgetItem();
			quentityItem->setTextAlignment(Qt::AlignRight);
			quentityItem->setText(QString::number(quantity));
			m_cart->getUI().tableWidget->setItem(row, 3, quentityItem);

			QTableWidgetItem* discountItem = new QTableWidgetItem();
			discountItem->setTextAlignment(Qt::AlignRight);
			discountItem->setText(QString::number(discount, 'f', 2));
			m_cart->getUI().tableWidget->setItem(row, 4, discountItem);
			
			QTableWidgetItem* totalItem = new QTableWidgetItem();
			totalItem->setTextAlignment(Qt::AlignRight);
			totalItem->setText(QString::number(queryBillTable.value("total").toDouble(), 'f', 2));
			m_cart->getUI().tableWidget->setItem(row, 5, totalItem);

			QWidget* base = new QWidget(m_cart->getUI().tableWidget);
			QPushButton* removeBtn = new QPushButton(base);
			removeBtn->setIcon(QIcon("icons/delete.png"));
			removeBtn->setIconSize(QSize(24, 24));
			removeBtn->setMaximumWidth(100);
			
			QObject::connect(removeBtn, SIGNAL(clicked()), m_cart->getRemoveButtonSignalMapper(), SLOT(map()));
			(m_cart->getRemoveButtonSignalMapper())->setMapping(removeBtn, saleId);

			QHBoxLayout *layout = new QHBoxLayout;
			layout->setContentsMargins(0, 0, 0, 0);
			layout->addWidget(removeBtn);
			layout->insertStretch(2);
			base->setLayout(layout);
			m_cart->getUI().tableWidget->setCellWidget(row, 6, base);
			base->show();
			m_cart->getUI().tableWidget->setItem(row, 7, new QTableWidgetItem(saleId));	
		}
	}
	if (row >= 0)
	{
		TableTextWidget* textWidget = new TableTextWidget(m_cart->getUI().tableWidget, row, 3, m_cart->getUI().tableWidget);
		QObject::connect(textWidget, SIGNAL(notifyEnterPressed(QString, int, int)), m_cart, SLOT(slotQuantityCellUpdated(QString, int, int)));
		textWidget->setTextFormatterFunc(convertToQuantityFormat);
		m_cart->getUI().tableWidget->setCellWidget(row, 3, textWidget);
		m_cart->getUI().tableWidget->setCurrentCell(row, 3);
		m_cart->getUI().tableWidget->setFocus();
		textWidget->setFocus();
	}

	ES::MainWindowHolder::instance()->getSecondaryDisplay()->update();

	close();
}

void ESAddBillItem::slotHideImages()
{
	if (ui.hideImages->isChecked())
	{
		ui.tableWidget->hideColumn(3);
		ui.tableWidget->verticalHeader()->setDefaultSectionSize(30);
	}
	else
	{
		ui.tableWidget->showColumn(3);
		ui.tableWidget->verticalHeader()->setDefaultSectionSize(100);
	}
}

void ESAddBillItem::slotItemDoubleClicked(int row, int col)
{
	QTableWidgetItem* idCell = ui.tableWidget->item(row, 0);
	if (!idCell)
		return;

	addToBill(idCell->text());
}

void ESAddBillItem::focus()
{
	ui.itemText->setFocus();
}


