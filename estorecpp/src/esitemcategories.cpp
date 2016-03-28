#include "esitemcategories.h"
#include "esadditemcategory.h"
#include "utility/esmainwindowholder.h"
#include "utility/esdbconnection.h"
#include <QMessageBox>
#include <QPushButton>
#include "utility/utility.h"


ESItemCategories::ESItemCategories(QWidget *parent /*= 0*/)
: QWidget(parent)
{
	ui.setupUi(this);
	m_updateButtonSignalMapper = new QSignalMapper(this);
	m_removeButtonSignalMapper = new QSignalMapper(this);

	QObject::connect(m_updateButtonSignalMapper, SIGNAL(mapped(QString)), this, SLOT(slotUpdate(QString)));
	QObject::connect(m_removeButtonSignalMapper, SIGNAL(mapped(QString)), this, SLOT(slotRemove(QString)));

	QStringList headerLabels;
	headerLabels.append("Item Category Code");
	headerLabels.append("Item Category Name");
	headerLabels.append("Description");
	headerLabels.append("Actions");

	ui.tableWidget->setHorizontalHeaderLabels(headerLabels);
	ui.tableWidget->horizontalHeader()->setStretchLastSection(true);
	ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);

	QObject::connect(ui.addNewItemCategoryButton, SIGNAL(clicked()), this, SLOT(slotAddNewCategory()));
	if (!ES::DbConnection::instance()->open())
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Critical);
		mbox.setText(QString("Cannot connect to the database : ESManageStockItems::displayStockItems "));
		mbox.exec();
	}
	else
	{
		ui.tableWidget->setSortingEnabled(false);
		displayCategories();
		ui.tableWidget->setSortingEnabled(true);
	}
	
}

ESItemCategories::~ESItemCategories()
{

}

void ESItemCategories::slotAddNewCategory()
{
	AddItemCategory* addItemCategory = new AddItemCategory(this);
	addItemCategory->getUI().groupBox->setTitle("Add Item Category");
	ES::MainWindowHolder::instance()->getMainWindow()->setCentralWidget(addItemCategory);
	addItemCategory->show();
}

void ESItemCategories::slotUpdate(QString itemCategoryId)
{
	AddItemCategory* addItemCategory = new AddItemCategory(this);
	addItemCategory->getUI().groupBox->setTitle("Update Item Category");
	QSqlQuery queryCategory("SELECT itemcategory_code, 	itemcategory_name, description FROM item_category WHERE itemcategory_id ="+itemCategoryId);
	while (queryCategory.next())
	{
		addItemCategory->getUI().itemCategoryCode->setText(queryCategory.value(0).toString());
		addItemCategory->getUI().itemCategoryName->setText(queryCategory.value(1).toString());
		addItemCategory->getUI().description->setText(queryCategory.value(2).toString());
		addItemCategory->setCategoryId(itemCategoryId);
	}
	addItemCategory->getUI().addItemCategoryButton->setText("Update");
	addItemCategory->setUpdate(true);
	ES::MainWindowHolder::instance()->getMainWindow()->setCentralWidget(addItemCategory);
	addItemCategory->show();
}

void ESItemCategories::slotRemove(QString itemCategoryId)
{
	if (ES::Utility::verifyUsingMessageBox(this, "EStore", "Do you really want to remove this?"))
	{
		QString str("UPDATE item_category SET deleted = 1 WHERE itemcategory_id = " + itemCategoryId);
		QSqlQuery q;
		if (q.exec(str))
		{
			while (ui.tableWidget->rowCount() > 0)
			{
				ui.tableWidget->removeRow(0);
			}
			displayCategories();
		}
	}
}

void ESItemCategories::displayCategories()
{
	int row = 0;
	QSqlQuery queryCategory("SELECT * FROM item_category WHERE deleted = 0");
	while (queryCategory.next())
	{
		row = ui.tableWidget->rowCount();
		ui.tableWidget->insertRow(row);
		QString itemCategoryId = queryCategory.value(0).toString();

		int i = 0;
		while (i < 3)
		{
			ui.tableWidget->setItem(row, i, new QTableWidgetItem(queryCategory.value(i+1).toString()));
			i++; 
		}
		
		QWidget* base = new QWidget(ui.tableWidget);
		QPushButton* updateBtn = new QPushButton("Update", base);
		updateBtn->setMaximumWidth(100);
		QPushButton* removeBtn = new QPushButton("Remove", base);
		removeBtn->setMaximumWidth(100);

		m_updateButtonSignalMapper->setMapping(updateBtn, itemCategoryId);
		QObject::connect(updateBtn, SIGNAL(clicked()), m_updateButtonSignalMapper, SLOT(map()));

		QObject::connect(removeBtn, SIGNAL(clicked()), m_removeButtonSignalMapper, SLOT(map()));
		m_removeButtonSignalMapper->setMapping(removeBtn, itemCategoryId);

		QHBoxLayout *layout = new QHBoxLayout;
		layout->setContentsMargins(0, 0, 0, 0);
		layout->addWidget(updateBtn);
		layout->addWidget(removeBtn);
		layout->insertStretch(2);
		base->setLayout(layout);
		ui.tableWidget->setCellWidget(row, i, base);
		base->show();
	}
	
}
