#include "esmanageitems.h"
#include "utility/esdbconnection.h"
#include <QMessageBox>
#include <QPushButton>


ESManageItems::ESManageItems(QWidget *parent /*= 0*/)
: QWidget(parent)
{
	ui.setupUi(this);
	m_updateButtonSignalMapper = new QSignalMapper(this);
	m_removeButtonSignalMapper = new QSignalMapper(this);

	QObject::connect(m_updateButtonSignalMapper, SIGNAL(mapped(QString)), this, SLOT(slotUpdate(QString)));
	QObject::connect(m_removeButtonSignalMapper, SIGNAL(mapped(QString)), this, SLOT(slotRemove(QString)));

	QStringList headerLabels;
	headerLabels.append("Item Code");
	headerLabels.append("Item Name");
	headerLabels.append("Category");
	headerLabels.append("Qty");
	headerLabels.append("Min. Qty");
	headerLabels.append("Unit");
	headerLabels.append("Unit Price");
	headerLabels.append("Description");
	headerLabels.append("Actions");

	ui.tableWidget->setHorizontalHeaderLabels(headerLabels);
	ui.tableWidget->horizontalHeader()->setStretchLastSection(true);
	ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);

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
		ui.tableWidget->setSortingEnabled(false);
		displayStockItems();
		ui.tableWidget->setSortingEnabled(true);
	}

}

ESManageItems::~ESManageItems()
{

}

void ESManageItems::slotAddNewItem()
{
	int row = ui.tableWidget->rowCount();
	ui.tableWidget->insertRow(row);
}

void ESManageItems::slotUpdate(QString itemCategoryId)
{
	QMessageBox mbox;
	mbox.setIcon(QMessageBox::Critical);
	mbox.setText(QString("slotUpdate : ") + itemCategoryId);
	mbox.exec();
}

void ESManageItems::slotRemove(QString itemCategoryId)
{
	QMessageBox mbox;
	mbox.setIcon(QMessageBox::Critical);
	mbox.setText(QString("slotRemove : ") + itemCategoryId);
	mbox.exec();
}

void ESManageItems::displayStockItems()
{
	int row = 0;
	QSqlQuery queryCategory("SELECT * FROM item_category");
	while (queryCategory.next())
	{
		row = ui.tableWidget->rowCount();
		ui.tableWidget->insertRow(row);
		QString itemCategoryId = queryCategory.value(0).toString();

		int i = 0;
		while (i < 3)
		{
			ui.tableWidget->setItem(row, i, new QTableWidgetItem(queryCategory.value(i + 1).toString()));
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
