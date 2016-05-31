#include "esadditem.h"
#include "utility/esdbconnection.h"
#include <QMessageBox>
#include <QPushButton>
#include "utility/utility.h"
#include "esmanageitems.h"
#include "utility/esmainwindowholder.h"
#include <QFileDialog>


AddItem::AddItem(QWidget *parent /*= 0*/)
: QWidget(parent), m_isUpdate(false)
{
	ui.setupUi(this);
	QObject::connect(ui.addButton, SIGNAL(clicked()), this, SLOT(slotAddItem()));
	QObject::connect(ui.openImage, SIGNAL(clicked()), this, SLOT(slotAddImage()));
	QObject::connect(ui.tableWidget, SIGNAL(cellPressed(int, int)), this, SLOT(slotCategorySelected(int, int)));

	QStringList headerLabels;
	headerLabels.append("CatID");
	headerLabels.append("Category Code");
	headerLabels.append("Category Name");
	headerLabels.append("Description");

	ui.tableWidget->setHorizontalHeaderLabels(headerLabels);
	ui.tableWidget->horizontalHeader()->setStretchLastSection(true);
	ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.tableWidget->hideColumn(0);

	if (!ES::DbConnection::instance()->open())
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Critical);
		mbox.setText(QString("Cannot connect to the database : AddItem"));
		mbox.exec();
	}

	QSqlQuery queryCategory("SELECT * FROM item_category WHERE deleted = 0");
	QString catCode = DEFAULT_DB_COMBO_VALUE;
	
	int row = 0;
	while (queryCategory.next())
	{
		row = ui.tableWidget->rowCount();
		ui.tableWidget->insertRow(row);

		ui.tableWidget->setItem(row, 0, new QTableWidgetItem(queryCategory.value("itemcategory_id").toString()));
		ui.tableWidget->setItem(row, 1, new QTableWidgetItem(queryCategory.value("itemcategory_code").toString()));
		ui.tableWidget->setItem(row, 2, new QTableWidgetItem(queryCategory.value("itemcategory_name").toString()));
		ui.tableWidget->setItem(row, 3, new QTableWidgetItem(queryCategory.value("description").toString()));

		row++;
	}

	ui.barCode->setFocus();
}

AddItem::~AddItem()
{

}

void AddItem::slotAddItem()
{
	QString iName = ui.itemName->text();
	QString iCode = ui.itemCode->text();
	QString iDesc = ui.descriptionText->toPlainText();
	QString catId = m_categoryId;
	QString barCode = ui.barCode->text();
	QString imagePath = ui.imagePath->text();
	QString unit = ui.unitText->text();
	
	if (iName == nullptr || iName.isEmpty() || iCode == nullptr || iCode.isEmpty() ||
		unit == nullptr || unit.isEmpty() || catId == "-1")
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Warning);
		mbox.setText(QString("Following fields should not be empty : Item Code, Item Name, Unit, Category"));
		mbox.exec();
	}
	else
	{
		QString q;
		if (isUpdate())
		{
			q = "Update item SET item_code = '" + iCode + "', item_name = '" + iName + "', description = '" + 
				iDesc + "', bar_code = '"+barCode+"', item_image = '" + imagePath + "', unit = '"+unit+"' , itemcategory_id = "+catId+" WHERE item_id = " + m_itemId;
		}
		else
		{
			q = "INSERT INTO item (item_code, item_name, description, bar_code, item_image, unit, itemcategory_id, deleted) VALUES ('" + 
				iCode + "','" + iName + "','" + iDesc +"','"+barCode+"','" + imagePath + "','" +unit+"',"+catId+ ", 0)";
		}
		QSqlQuery query;
		if (query.exec(q))
		{
			this->close();
			ESManageItems* manageItems = new ESManageItems();
			ES::MainWindowHolder::instance()->getMainWindow()->setCentralWidget(manageItems);
			this->close();
			manageItems->show();
		}
		else
		{
			QMessageBox mbox;
			mbox.setIcon(QMessageBox::Critical);
			mbox.setText(QString("Something goes wrong:: Item cannot be saved"));
			mbox.exec();
		}
	}
}

void AddItem::setUpdate(bool val)
{
	m_isUpdate = val;
}

bool AddItem::isUpdate() const
{
	return m_isUpdate;
}

QString AddItem::getItemId() const
{
	return m_itemId;
}

void AddItem::setItemId(QString val)
{
	m_itemId = val;
}

void AddItem::slotAddImage()
{ 
	m_itemImage = QFileDialog::getOpenFileName(this, tr("Open Backup File"));
	ui.imagePath->setText(m_itemImage);
}

void AddItem::slotCategorySelected(int row, int col)
{
	m_categoryId = ui.tableWidget->item(row, 0)->text();
	QString categoryCode = ui.tableWidget->item(row, 1)->text();
	ui.itemCategoryLbl->setText(categoryCode);
}

void AddItem::setItemCategoryId(QString categoryId)
{
	m_categoryId = categoryId;
}
