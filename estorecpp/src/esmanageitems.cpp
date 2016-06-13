#include "esmanageitems.h"
#include "esadditem.h"
#include "utility/esmainwindowholder.h"
#include "utility/esdbconnection.h"
#include <QMessageBox>
#include <QPushButton>
#include "utility/utility.h"

ESManageItems::ESManageItems(QWidget *parent /*= 0*/)  
: QWidget(parent), m_startingLimit(0), m_pageOffset(15), m_nextCounter(0), m_maxNextCount(0)
{
	ui.setupUi(this);
	m_updateButtonSignalMapper = new QSignalMapper(this);
	m_removeButtonSignalMapper = new QSignalMapper(this);
	m_imagesHidden = false;

	QObject::connect(m_updateButtonSignalMapper, SIGNAL(mapped(QString)), this, SLOT(slotUpdate(QString)));
	QObject::connect(m_removeButtonSignalMapper, SIGNAL(mapped(QString)), this, SLOT(slotRemove(QString)));
	QObject::connect(ui.searchTextBox, SIGNAL(textChanged(QString)), this, SLOT(slotSearch()));
	QObject::connect(ui.categoryComboBox, SIGNAL(activated(QString)), this, SLOT(slotSearch()));
	QObject::connect(ui.fitImages, SIGNAL(stateChanged(int)), this, SLOT(slotFitImages()));
	QObject::connect(ui.hideImages, SIGNAL(stateChanged(int)), this, SLOT(slotHideImages()));
	QObject::connect(ui.nextBtn, SIGNAL(clicked()), this, SLOT(slotNext()));
	QObject::connect(ui.prevBtn, SIGNAL(clicked()), this, SLOT(slotPrev()));

	QStringList headerLabels;
	headerLabels.append("Item Code");
	headerLabels.append("Item Name");
	headerLabels.append("Category");
	headerLabels.append("Image");
	//headerLabels.append("Min. Qty");
	headerLabels.append("Unit");
	//headerLabels.append("Unit Price");
	headerLabels.append("Description");
	headerLabels.append("Actions");

	ui.tableWidget->setHorizontalHeaderLabels(headerLabels);
	ui.tableWidget->horizontalHeader()->setStretchLastSection(true);
	ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.prevBtn->setDisabled(true);
	ui.nextBtn->setDisabled(true);

	QObject::connect(ui.addNewItemButton, SIGNAL(clicked()), this, SLOT(slotAddNewItem()));
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

ESManageItems::~ESManageItems()
{

}

void ESManageItems::slotAddNewItem()
{
	AddItem* addItem = new AddItem(this);
	addItem->getUI().groupBox->setTitle("Add Item");
	ES::MainWindowHolder::instance()->getMainWindow()->setCentralWidget(addItem);
	addItem->show();
}

void ESManageItems::slotUpdate(QString itemId)
{
	AddItem* addItem = new AddItem(this);
	addItem->getUI().groupBox->setTitle("Update Item");
	addItem->getUI().addButton->setText("Update");
	QSqlQuery queryItem("SELECT * FROM item WHERE item_id = " + itemId);
	if (queryItem.next())
	{
		QString itemCatId = queryItem.value("itemcategory_id").toString();
		addItem->setItemCategoryId(itemCatId);
		int rowCount = ui.tableWidget->rowCount();
		for (int i = 0; i < rowCount; ++i)
		{
			if (itemCatId == addItem->getUI().tableWidget->item(i, 0)->text())
			{
				addItem->getUI().tableWidget->setCurrentCell(i, 1);
				addItem->getUI().itemCategoryLbl->setText(addItem->getUI().tableWidget->item(i, 1)->text());
				break;
			}
		}
		addItem->getUI().itemCode->setText(queryItem.value("item_code").toString());
		addItem->getUI().barCode->setText(queryItem.value("bar_code").toString());
		addItem->getUI().imagePath->setText(queryItem.value("item_image").toString());
		addItem->getUI().unitText->setText(queryItem.value("unit").toString());
		addItem->getUI().descriptionText->setText(queryItem.value("description").toString());
		addItem->getUI().itemName->setText(queryItem.value("item_name").toString());
	}
	addItem->setUpdate(true);
	addItem->setItemId(itemId);
	ES::MainWindowHolder::instance()->getMainWindow()->setCentralWidget(addItem);
	addItem->show();
}

void ESManageItems::slotRemove(QString itemId)
{
	if (ES::Utility::verifyUsingMessageBox(this, "EStore", "Do you really want to remove this?"))
	{
		QString str("UPDATE item SET deleted = 1 WHERE item_id = " + itemId);
		QSqlQuery q;
		if (q.exec(str))
		{
			while (ui.tableWidget->rowCount() > 0)
			{
				ui.tableWidget->removeRow(0);
			}
			QString displayQueryStr("SELECT * FROM Item WHERE deleted = 0");
			displayQueryStr.append(" LIMIT ").append(QString::number(m_startingLimit));
			displayQueryStr.append(" , ").append(QString::number(m_pageOffset));
			QSqlQuery displayQuery(displayQueryStr);
			QSqlQuery countQuery("SELECT COUNT(*) as c FROM Item WHERE deleted = 0");
			if (countQuery.next())
			{
				m_totalRecords = countQuery.value("c").toInt();
			}
			displayItems(displayQuery);
		}
	}
}

void ESManageItems::displayItems(QSqlQuery& queryItems)
{
	m_maxNextCount = m_totalRecords / m_pageOffset;

	if (m_maxNextCount > m_nextCounter)
	{
		ui.nextBtn->setEnabled(true);
	}
	if (m_maxNextCount == m_nextCounter)
	{
		ui.nextBtn->setDisabled(true);
	}
	int row = 0;
	while (queryItems.next())
	{
		row = ui.tableWidget->rowCount();
		ui.tableWidget->insertRow(row);
		QString itemId = queryItems.value(0).toString();

		ui.tableWidget->setItem(row, 0, new QTableWidgetItem(queryItems.value("item_code").toString()));
		ui.tableWidget->setItem(row, 1, new QTableWidgetItem(queryItems.value("item_name").toString()));
		ui.tableWidget->setItem(row, 4, new QTableWidgetItem(queryItems.value("unit").toString()));
		ui.tableWidget->setItem(row, 5, new QTableWidgetItem(queryItems.value("description").toString()));
		
		QString imagePath = queryItems.value("item_image").toString();
		if (imagePath.isNull() || imagePath.isEmpty())
		{
			imagePath = "images/default.png";
		}
		QTableWidgetItem* imageItem = new QTableWidgetItem();
		imageItem->setData(Qt::DecorationRole, QPixmap::fromImage(QImage(imagePath)).scaledToHeight(100));
		ui.tableWidget->setItem(row, 3, imageItem);

		QSqlQuery queryCategories("SELECT * FROM item_category WHERE itemcategory_id = " + queryItems.value("itemcategory_id").toString());
		if (queryCategories.next())
		{
			ui.tableWidget->setItem(row, 2, new QTableWidgetItem(queryCategories.value("itemcategory_name").toString()));
		}

// 		QSqlQuery queryStocks("SELECT * FROM stock WHERE item_id = " + itemId);
// 		if (queryStocks.next())
// 		{
// 			ui.tableWidget->setItem(row, 3, new QTableWidgetItem(queryStocks.value(3).toString()));
// 			ui.tableWidget->setItem(row, 4, new QTableWidgetItem(queryStocks.value(4).toString()));
// 
// 		}
// 		QSqlQuery queryPrices("SELECT * FROM stock_order WHERE item_id = " + itemId);
// 		if (queryPrices.next())
// 		{
// 			ui.tableWidget->setItem(row, 6, new QTableWidgetItem(queryPrices.value("selling_price").toString()));
// 		}
// 		else
// 		{
// 			ui.tableWidget->setItem(row, 3, new QTableWidgetItem(DEFAULT_DB_NUMERICAL_TO_DISPLAY));
// 			ui.tableWidget->setItem(row, 4, new QTableWidgetItem(DEFAULT_DB_NUMERICAL_TO_DISPLAY));
// 			ui.tableWidget->setItem(row, 6, new QTableWidgetItem(DEFAULT_DB_NUMERICAL_TO_DISPLAY));
// 		}

		QWidget* base = new QWidget(ui.tableWidget);
		QPushButton* updateBtn = new QPushButton(base);
		updateBtn->setIcon(QIcon("icons/update.png"));
		updateBtn->setIconSize(QSize(24, 24));
		updateBtn->setMaximumWidth(100);

		QPushButton* removeBtn = new QPushButton(base);
		removeBtn->setMaximumWidth(100);
		removeBtn->setIcon(QIcon("icons/delete.png"));
		removeBtn->setIconSize(QSize(24, 24));

		m_updateButtonSignalMapper->setMapping(updateBtn, itemId);
		QObject::connect(updateBtn, SIGNAL(clicked()), m_updateButtonSignalMapper, SLOT(map()));

		QObject::connect(removeBtn, SIGNAL(clicked()), m_removeButtonSignalMapper, SLOT(map()));
		m_removeButtonSignalMapper->setMapping(removeBtn, itemId);

		QHBoxLayout *layout = new QHBoxLayout;
		layout->setContentsMargins(0, 0, 0, 0);
		layout->addWidget(updateBtn);
		layout->addWidget(removeBtn);
		layout->insertStretch(2);
		base->setLayout(layout);
		ui.tableWidget->setCellWidget(row, 6, base);
		base->show();
	}

}

void ESManageItems::slotSearch()
{
	QString searchText = ui.searchTextBox->text();
	int categoryId = ui.categoryComboBox->currentData().toInt();
	
	while (ui.tableWidget->rowCount() > 0)
	{
		ui.tableWidget->removeRow(0);
	}

	QString searchQuery = "SELECT * FROM item ";
	QString countQueryStr("SELECT COUNT(*) as c FROM item");
	bool categorySelected = false;
	if (categoryId != -1)
	{
		searchQuery.append(" WHERE deleted =0 AND itemcategory_id = ");
		countQueryStr.append(" WHERE deleted = 0 AND itemcategory_id = ");
		QString catId;
		catId.setNum(categoryId);
		searchQuery.append(catId);
		countQueryStr.append(catId);
		categorySelected = true;
	}

	if (!searchText.isEmpty())
	{
		if (categorySelected)
		{
			searchQuery.append(" AND ");
			countQueryStr.append(" AND ");
		}
		else
		{
			searchQuery.append(" WHERE deleted = 0 AND ");
			countQueryStr.append(" WHERE deleted = 0 AND ");
		}
		searchQuery.append(" (item_code LIKE '%" + searchText + "%' OR item_name LIKE '%" + searchText + "%')");
		countQueryStr.append(" (item_code LIKE '%" + searchText + "%' OR item_name LIKE '%" + searchText + "%')");
	}
	else
	{
		if (!categorySelected)
		{
			searchQuery.append(" WHERE deleted = 0");
			countQueryStr.append(" WHERE deleted = 0");
		}
	}
	QSqlQuery queryCount(countQueryStr);
	if (queryCount.next())
	{
		m_totalRecords = queryCount.value("c").toInt();
	}
	searchQuery.append(" LIMIT ").append(QString::number(m_startingLimit));
	searchQuery.append(" , ").append(QString::number(m_pageOffset));
	ui.tableWidget->setSortingEnabled(false);
	QSqlQuery queryItems(searchQuery);
	displayItems(queryItems);
	ui.tableWidget->setSortingEnabled(true);
}

void ESManageItems::slotFitImages()
{
	if (m_imagesHidden)
	{
		ui.tableWidget->verticalHeader()->setDefaultSectionSize(30);
		return;
	}
	if (ui.fitImages->isChecked())
	{
		ui.tableWidget->verticalHeader()->setDefaultSectionSize(100);
	}
	else
	{
		ui.tableWidget->verticalHeader()->setDefaultSectionSize(30);
	}
}

void ESManageItems::slotHideImages()
{
	if (!ui.hideImages->isChecked())
	{
		ui.tableWidget->showColumn(3);
		m_imagesHidden = false;
	}
	else
	{
		ui.tableWidget->hideColumn(3);
		m_imagesHidden = true;
	}
	slotFitImages();
}

void ESManageItems::slotPrev()
{
	if (m_nextCounter == 1)
	{
		ui.prevBtn->setDisabled(true);
	}
	if (m_nextCounter > 0)
	{
		m_nextCounter--;
		m_startingLimit -= m_pageOffset;
		ui.nextBtn->setEnabled(true);
	}
	slotSearch();
}

void ESManageItems::slotNext()
{
	if (m_nextCounter < m_maxNextCount)
	{
		m_nextCounter++;
		ui.prevBtn->setEnabled(true);
		m_startingLimit += m_pageOffset;
	}
	if (m_maxNextCount == m_nextCounter)
	{
		ui.nextBtn->setDisabled(true);
	}
	slotSearch();
}
