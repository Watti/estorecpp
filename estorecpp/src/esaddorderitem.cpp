#include "esaddorderitem.h"
#include "esmanageorderitems.h"
#include "utility/esmainwindowholder.h"
#include "utility/esdbconnection.h"
#include <QMessageBox>
#include <QPushButton>
#include "utility/session.h"
#include "utility/utility.h"
#include "entities/SaleLineEdit.h"
#include <KDReportsTextElement.h>
#include <QPrintPreviewDialog>
#include <set>

AddOrderItem::AddOrderItem(QWidget *parent/* = 0*/)
{
	m_update = false;
	ui.setupUi(this);
	m_removeButtonSignalMapper = new QSignalMapper(this);

	QStringList headerLabels;
	headerLabels.append("Item ID");
	headerLabels.append("Item Code");
	headerLabels.append("Item Name");
	headerLabels.append("Category");
	headerLabels.append("Min. Qty");
	headerLabels.append("Unit");

	ui.itemTableWidget->setHorizontalHeaderLabels(headerLabels);
	ui.itemTableWidget->horizontalHeader()->setStretchLastSection(true);
	ui.itemTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.itemTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.itemTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.itemTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.itemTableWidget->hideColumn(0);

	QStringList headerLabels1;
	headerLabels1.append("Item ID");
	headerLabels1.append("Item Code");
	headerLabels1.append("Item Name");
	headerLabels1.append("Category");
	headerLabels1.append("Purchasing Price");
	headerLabels1.append("Qty");
	headerLabels1.append("Actions");

	ui.selectedItemTableWidget->setHorizontalHeaderLabels(headerLabels1);
	ui.selectedItemTableWidget->horizontalHeader()->setStretchLastSection(true);
	ui.selectedItemTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.selectedItemTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.selectedItemTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.selectedItemTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.selectedItemTableWidget->hideColumn(0);

	QStringList headerLabels2;
	headerLabels2.append("Supplier ID");
	headerLabels2.append("Supplier Code");
	headerLabels2.append("Supplier Name");

	ui.supplierTableWidget->setHorizontalHeaderLabels(headerLabels2);
	ui.supplierTableWidget->horizontalHeader()->setStretchLastSection(true);
	ui.supplierTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.supplierTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.supplierTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.supplierTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.supplierTableWidget->hideColumn(0);

	ui.dateEdit->setDisplayFormat("yyyy-MM-dd");
	ui.dateEdit->setDate(QDate::currentDate());

	ui.showAllItems->setChecked(true);

	QObject::connect(ui.addOrderItemButton, SIGNAL(clicked()), this, SLOT(slotPlaceNewOrder()));
	QObject::connect(ui.searchTextBox, SIGNAL(textChanged(QString)), this, SLOT(slotSearch()));
	QObject::connect(ui.categoryComboBox, SIGNAL(activated(QString)), this, SLOT(slotSearch()));

	QObject::connect(ui.supplierTableWidget, SIGNAL(cellClicked(int, int)), this, SLOT(slotSupplierSelected(int, int)));
	QObject::connect(ui.itemTableWidget, SIGNAL(cellDoubleClicked(int, int)), this, SLOT(slotItemDoubleClicked(int, int)));
	QObject::connect(m_removeButtonSignalMapper, SIGNAL(mapped(QString)), this, SLOT(slotRemove(QString)));

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
		QString catCode = DEFAULT_DB_COMBO_VALUE;
		int catId = -1;

		ui.categoryComboBox->addItem(catCode, catId);

		while (queryCategory.next())
		{
			catId = queryCategory.value(0).toInt();
			ui.categoryComboBox->addItem(queryCategory.value(1).toString() + " / " + queryCategory.value("itemcategory_name").toString(), catId);
		}

		// Fill suppliers
		while (ui.supplierTableWidget->rowCount() > 0)
		{
			ui.supplierTableWidget->removeRow(0);
		}

		QSqlQuery querySuppliers("SELECT * FROM supplier WHERE deleted = 0");
		int row = 0;
		while (querySuppliers.next())
		{
			row = ui.supplierTableWidget->rowCount();
			ui.supplierTableWidget->insertRow(row);

			ui.supplierTableWidget->setItem(row, 0, new QTableWidgetItem(querySuppliers.value("supplier_id").toString()));
			ui.supplierTableWidget->setItem(row, 1, new QTableWidgetItem(querySuppliers.value("supplier_code").toString()));
			ui.supplierTableWidget->setItem(row, 2, new QTableWidgetItem(querySuppliers.value("supplier_name").toString()));
		}
	}
	
}

AddOrderItem::~AddOrderItem()
{

}

void AddOrderItem::setUpdate(bool update)
{
	m_update = update;
}

void AddOrderItem::slotPlaceNewOrder()
{
	int userId = ES::Session::getInstance()->getUser()->getId();
	QString userIdStr;
	userIdStr.setNum(userId);

	int row = ui.supplierTableWidget->currentRow();
	QTableWidgetItem* idCell = ui.supplierTableWidget->item(row, 0);
	if (!idCell)
		return;

	QString supplierId = idCell->text();
	QDateTime d = QDateTime::fromString(ui.dateEdit->text(), Qt::ISODate);
	
	// Add data to 'purchase_order' table
	QString q = "INSERT INTO purchase_order (user_id, supplier_id, order_date, comments) VALUES (" +
		userIdStr + "," + supplierId + ",'" + d.toString("yyyy-MM-dd") + "','" + ui.comments->toPlainText() + "')";

	QSqlQuery query;
	if (!query.exec(q))
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Critical);
		mbox.setText(QString("Something goes wrong: order cannot be saved"));
		mbox.exec();
	}

	QString purchaseOrderId = query.lastInsertId().value<QString>();

	// Add data to 'purchase_order_item' table
	int rowCount = ui.selectedItemTableWidget->rowCount();
	for (int i = 0; i < rowCount; ++i)
	{
		QTableWidgetItem* idCell = ui.selectedItemTableWidget->item(i, 0);
		if (!idCell) continue;

		QString itemId = idCell->text();

		ES::SaleLineEdit* le = NULL;
		le = static_cast<ES::SaleLineEdit*>(ui.selectedItemTableWidget->cellWidget(i, 4));
		if (!le) continue;

		QString price = le->text();

		le = static_cast<ES::SaleLineEdit*>(ui.selectedItemTableWidget->cellWidget(i, 5));
		if (!le) continue;

		QString quantity = le->text();

		QString qry = "INSERT INTO purchase_order_item (purchaseorder_id, item_id, qty, purchasing_price) VALUES (" + 
			purchaseOrderId + "," + itemId + "," + quantity + "," + price + ")";

		QSqlQuery insertQuery;
		if (!insertQuery.exec(qry))
		{
			QMessageBox mbox;
			mbox.setIcon(QMessageBox::Critical);
			mbox.setText(QString("Something goes wrong: order items cannot be saved"));
			mbox.exec();
		}
	}

	if (ui.doPrint->isChecked() && false)
	{
		//KDReports::TextElement titleElement("Test");
		//titleElement.setPointSize(15);
		//report.addElement(titleElement, Qt::AlignHCenter);

		QPrinter printer;
		printer.setPaperSize(QPrinter::A4);

		printer.setFullPage(false);
		printer.setOrientation(QPrinter::Portrait);

		QPrintPreviewDialog *dialog = new QPrintPreviewDialog(&printer, 0);
		QObject::connect(dialog, SIGNAL(paintRequested(QPrinter*)), this, SLOT(slotPrint(QPrinter*)));
		dialog->setWindowTitle(tr("Print Document"));

		dialog->exec();
	}

	ESManageOrderItems* manageItems = new ESManageOrderItems(ES::MainWindowHolder::instance()->getMainWindow());
	ES::MainWindowHolder::instance()->getMainWindow()->setCentralWidget(manageItems);
	this->close();
	manageItems->show();

}

QString AddOrderItem::getOrderId() const
{
	return m_orderId;
}

void AddOrderItem::setOrderId(QString val)
{
	m_orderId = val;
}

void AddOrderItem::slotSearch()
{
	QString searchText = ui.searchTextBox->text();
	int categoryId = ui.categoryComboBox->currentData().toInt();

	while (ui.itemTableWidget->rowCount() > 0)
	{
		ui.itemTableWidget->removeRow(0);
	}

	QString searchQuery = "SELECT * FROM item ";
	bool categorySelected = false;
	if (categoryId != -1)
	{
		searchQuery.append(" WHERE deleted =0 AND itemcategory_id = ");
		QString catId;
		catId.setNum(categoryId);
		searchQuery.append(catId);
		categorySelected = true;
	}

	if (!searchText.isEmpty())
	{
		if (categorySelected)
		{
			searchQuery.append(" AND ");
		}
		else
		{
			searchQuery.append(" WHERE deleted = 0 AND ");
		}
		searchQuery.append(" (item_code LIKE '%" + searchText + "%' OR item_name LIKE '%" + searchText + "%')");
	}
	else
	{
		if (!categorySelected)
		{
			searchQuery.append(" WHERE deleted = 0");
		}
	}
	ui.itemTableWidget->setSortingEnabled(false);
	QSqlQuery queryItems(searchQuery);
	displayItems(queryItems);
	ui.itemTableWidget->setSortingEnabled(true);
}

void AddOrderItem::displayItems(QSqlQuery& queryItems)
{
	// Get selected suppliers items
	std::set<QString> currentSuppliersItems;
	QSqlQuery supplierQuery("SELECT item_id FROM supplier_item WHERE supplier_id = " + m_selectedSupplierId);
	while (supplierQuery.next())
	{
		currentSuppliersItems.insert(supplierQuery.value("item_id").toString());
	}

	int row = 0;
	while (queryItems.next())
	{
		QString itemId = queryItems.value("item_id").toString();
		if (currentSuppliersItems.find(itemId) == currentSuppliersItems.end())
		{
			continue;
		}

		row = ui.itemTableWidget->rowCount();
		ui.itemTableWidget->insertRow(row);

		QSqlQuery queryCategories("SELECT * FROM item_category WHERE itemcategory_id = " + queryItems.value(1).toString());
		if (queryCategories.next())
		{
			ui.itemTableWidget->setItem(row, 3, new QTableWidgetItem(queryCategories.value(2).toString()));
		}

		ui.itemTableWidget->setItem(row, 0, new QTableWidgetItem(queryItems.value("item_id").toString()));
		ui.itemTableWidget->setItem(row, 1, new QTableWidgetItem(queryItems.value("item_code").toString()));
		ui.itemTableWidget->setItem(row, 2, new QTableWidgetItem(queryItems.value("item_name").toString()));

		QSqlQuery queryMinQty("SELECT min_qty FROM stock WHERE item_id = " + queryItems.value(0).toString());
		if (queryMinQty.next())
		{
			ui.itemTableWidget->setItem(row, 4, new QTableWidgetItem(queryMinQty.value("min_qty").toString()));
		}
		else
		{
			ui.itemTableWidget->setItem(row, 4, new QTableWidgetItem("N/A"));
		}
		ui.itemTableWidget->setItem(row, 5, new QTableWidgetItem(queryItems.value("unit").toString()));
	}
}

void AddOrderItem::slotSupplierSelected(int row, int col)
{
	QTableWidgetItem* idCell = ui.supplierTableWidget->item(row, 0);
	if (!idCell)
		return;

	// check whether this supplier ships selected items
	int rowCount = 0;
	rowCount = ui.selectedItemTableWidget->rowCount();
	for (int i = 0; i < rowCount; i++)
	{
		QTableWidgetItem* itemIdCell = ui.selectedItemTableWidget->item(i, 0);
		if (!itemIdCell)
			continue;

		QSqlQuery q("SELECT * FROM supplier_item WHERE supplier_id = " + idCell->text() + " AND item_id = " + itemIdCell->text());
		if (!q.next())
		{
			QMessageBox mbox;
			mbox.setIcon(QMessageBox::Critical);
			mbox.setText(QString("This Supplier doesn't deliver selected items"));
			mbox.exec();

			ui.supplierTableWidget->blockSignals(true);
			if (!m_selectedSupplierId.isEmpty())
			{
				for (int j = 0; j < ui.supplierTableWidget->rowCount(); j++)
				{
					QTableWidgetItem* supplierIdCell = ui.supplierTableWidget->item(j, 0);
					if (supplierIdCell && supplierIdCell->text() == m_selectedSupplierId)
					{
						ui.supplierTableWidget->setCurrentCell(j, 1);
						ui.supplierTableWidget->setFocus();
						break;
					}
				}
			}
			else
			{
				ui.supplierTableWidget->clearSelection();
				ui.supplierTableWidget->clearFocus();
			}
			ui.supplierTableWidget->blockSignals(false);
			return;
		}
	}

	QString query("SELECT * FROM supplier WHERE deleted = 0 AND supplier_id = ");
	query.append(idCell->text());
	QSqlQuery supplierQry(query);
	if (supplierQry.next())
	{
		ui.supplierCode->setText(supplierQry.value("supplier_code").toString());
		ui.supplierName->setText(supplierQry.value("supplier_name").toString());
	}

	rowCount = ui.selectedItemTableWidget->rowCount();
	for (int i = 0; i < rowCount; i++)
	{
		QTableWidgetItem* itemIdCell = ui.selectedItemTableWidget->item(i, 0);
		if (!itemIdCell)
			continue;

		QSqlQuery q("SELECT * FROM supplier_item WHERE supplier_id = " + idCell->text() + " AND item_id = " + itemIdCell->text());
		if (q.next())
		{
			ES::SaleLineEdit* le = static_cast<ES::SaleLineEdit*>(ui.selectedItemTableWidget->cellWidget(i, 4));
			if (le)
			{
				double purchasingPrice = q.value("purchasing_price").toDouble();
				QString st = QString::number(purchasingPrice, 'f', 2);

				le->setText(st);
				//le->setReadOnly(true);
			}
		}
	}

	m_selectedSupplierId = idCell->text();

	slotSearch();
}

void AddOrderItem::slotItemDoubleClicked(int rowi, int col)
{
	QTableWidgetItem* idCell = ui.itemTableWidget->item(rowi, 0);
	if (!idCell)
		return;

	if (!m_selectedSupplierId.isEmpty())
	{
		QSqlQuery q("SELECT * FROM supplier_item WHERE supplier_id = " + m_selectedSupplierId + " AND item_id = " + idCell->text());
		if (!q.next())
		{
			QMessageBox mbox;
			mbox.setIcon(QMessageBox::Critical);
			mbox.setText(QString("Selected supplier doesn't deliver selected items"));
			mbox.exec();
			return;
		}
	}
	else
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Critical);
		mbox.setText(QString("Please select supplier first"));
		mbox.exec();
		return;
	}
		
	QSqlQuery queryItems("SELECT * FROM item WHERE item_id = " + idCell->text());
	if (queryItems.next())
	{
		// If already a selected item
		for (int i = 0; i < ui.selectedItemTableWidget->rowCount(); i++)
		{
			QTableWidgetItem* cell = ui.selectedItemTableWidget->item(i, 0);
			if (cell->text() == queryItems.value("item_id").toString())
			{
				ui.selectedItemTableWidget->setCurrentCell(i, 5);
				ES::SaleLineEdit* le = static_cast<ES::SaleLineEdit*>(ui.selectedItemTableWidget->cellWidget(i, 5));
				if (le) le->setFocus();
				return;
			}
		}

		int row = ui.selectedItemTableWidget->rowCount();
		ui.selectedItemTableWidget->insertRow(row);

		ui.selectedItemTableWidget->setItem(row, 0, new QTableWidgetItem(queryItems.value("item_id").toString()));
		ui.selectedItemTableWidget->setItem(row, 1, new QTableWidgetItem(queryItems.value("item_code").toString()));
		ui.selectedItemTableWidget->setItem(row, 2, new QTableWidgetItem(queryItems.value("item_name").toString()));

		QSqlQuery queryCategories("SELECT * FROM item_category WHERE itemcategory_id = " + queryItems.value("itemcategory_id").toString());
		if (queryCategories.next())
		{
			ui.selectedItemTableWidget->setItem(row, 3, new QTableWidgetItem(queryCategories.value("itemcategory_name").toString()));
		}

		ES::SaleLineEdit* lePrice = new ES::SaleLineEdit(idCell->text(), row);
		ui.selectedItemTableWidget->setCellWidget(row, 4, lePrice);
		lePrice->setFocus();

		QSqlQuery q("SELECT * FROM supplier_item WHERE supplier_id = " + m_selectedSupplierId + " AND item_id = " + idCell->text());
		if (q.next())
		{
			double purchasingPrice = q.value("purchasing_price").toDouble();
			QString st = QString::number(purchasingPrice, 'f', 2);
			lePrice->setText(st);
		}

		ES::SaleLineEdit* leQty = new ES::SaleLineEdit(idCell->text(), row);
		ui.selectedItemTableWidget->setCellWidget(row, 5, leQty);
		leQty->setFocus();

		QPushButton* removeBtn = new QPushButton(" X ");
		removeBtn->setMaximumWidth(30);
		m_removeButtonSignalMapper->setMapping(removeBtn, queryItems.value("item_id").toString());
		QObject::connect(removeBtn, SIGNAL(clicked()), m_removeButtonSignalMapper, SLOT(map()));

		ui.selectedItemTableWidget->setCellWidget(row, 6, removeBtn);
	}
}

void AddOrderItem::slotRemove(QString itemId)
{
	int row = 0;
	while (ui.selectedItemTableWidget->rowCount() > 0)
	{
		QTableWidgetItem* item = ui.selectedItemTableWidget->item(row, 0);
		if (item->text() == itemId)
		{
			ui.selectedItemTableWidget->removeRow(row);
			return;
		}
		row++;
	}
}

void AddOrderItem::slotPrint(QPrinter* printer)
{
	report.print(printer);
}

