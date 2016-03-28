#include "esadditem.h"
#include "utility/esdbconnection.h"
#include <QMessageBox>
#include <QPushButton>
#include "utility/utility.h"
#include "esmanageitems.h"
#include "utility/esmainwindowholder.h"


AddItem::AddItem(QWidget *parent /*= 0*/)
: QWidget(parent), m_isUpdate(false)
{
	ui.setupUi(this);
	QObject::connect(ui.addButton, SIGNAL(clicked()), this, SLOT(slotAddItem()));

	if (!ES::DbConnection::instance()->open())
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Critical);
		mbox.setText(QString("Cannot connect to the database : AddItem"));
		mbox.exec();
	}

	QSqlQuery queryCategory("SELECT * FROM item_category WHERE deleted = 0");
	QString catCode = DEFAULT_DB_COMBO_VALUE;
	int catId = -1;

	ui.itemCategoryComboBox->addItem(catCode, catId);

	while (queryCategory.next())
	{
		catId = queryCategory.value(0).toInt();
		ui.itemCategoryComboBox->addItem(queryCategory.value(1).toString() + " / " + queryCategory.value("itemcategory_name").toString(), catId);
	}
}

AddItem::~AddItem()
{

}

void AddItem::slotAddItem()
{
	QString iName = ui.itemName->text();
	QString iCode = ui.itemCode->text();
	QString iDesc = ui.descriptionText->toPlainText();
	QString catId = ui.itemCategoryComboBox->itemData(ui.itemCategoryComboBox->currentIndex()).toString();
	QString barCode = ui.barCode->text();
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
			q = "Update item SET item_code = '" + iCode + "', item_name = '" + iName + "', description = '" + iDesc + "', bar_code = '"+barCode+"', unit = '"+unit+"' , itemcategory_id = "+catId+" WHERE item_id = " + m_itemId;
		}
		else
		{
			q = "INSERT INTO item (item_code, item_name, description, bar_code, unit, itemcategory_id, deleted) VALUES ('" + iCode + "','" + iName + "','" + iDesc +"','"+barCode+"','"+unit+"',"+catId+ ", 0)";
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
