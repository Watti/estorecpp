#include "esadditemcategory.h"
#include "utility/esdbconnection.h"
#include <QMessageBox>
#include <QPushButton>
#include "esitemcategories.h"
#include "utility/esmainwindowholder.h"
#include "esmainwindow.h"

AddItemCategory::AddItemCategory(QWidget *parent /*= 0*/)
: QWidget(parent), m_isUpdate(false)
{
	ui.setupUi(this);
	QObject::connect(ui.addItemCategoryButton, SIGNAL(clicked()), this, SLOT(slotAddItemCategory()));

	if (!ES::DbConnection::instance()->open())
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Critical);
		mbox.setText(QString("Cannot connect to the database : AddItemCategory"));
		mbox.exec();
	}
}

AddItemCategory::~AddItemCategory()
{

}

void AddItemCategory::slotAddItemCategory()
{
	QString text = ui.addItemCategoryButton->text();
	QString cName = ui.itemCategoryName->text();
	QString cCode = ui.itemCategoryCode->text();
	QString cDesc = ui.description->toPlainText();

	if (cName == nullptr || cName.isEmpty() || cCode == nullptr || cCode.isEmpty())
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Warning);
		mbox.setText(QString("Category Code or Name is Empty"));
		mbox.exec();
	}
	else
	{
		QString q;
		if (isUpdate())
		{
			q = "Update item_category SET itemcategory_code = '" + cCode + "', itemcategory_name = '" + cName + "', description = '" + cDesc + "' WHERE itemcategory_id = " + m_id;
		}
		else
		{
			QString qItem("SELECT * FROM item_category WHERE deleted = 0 AND itemcategory_code = '" + cCode + "'");
			QSqlQuery queryItem(qItem);
			if (queryItem.next())
			{
				QMessageBox mbox;
				mbox.setIcon(QMessageBox::Warning);
				mbox.setText(QString("Category code already exist"));
				mbox.exec();
				return;
			}

			q = "INSERT INTO item_category(itemcategory_code, itemcategory_name, description, deleted) VALUES ('" + cCode + "','" + cName + "','" + cDesc + "', 0)";
		}
		QSqlQuery query;
		if (query.exec(q))
		{
			this->close();
			ESItemCategories* itemCategories = new ESItemCategories();
			ES::MainWindowHolder::instance()->getMainWindow()->setCentralWidget(itemCategories);
			this->close();
			itemCategories->show();
		}
		else
		{
			QMessageBox mbox;
			mbox.setIcon(QMessageBox::Critical);
			mbox.setText(QString("Something goes wrong:: Category cannot be saved"));
			mbox.exec();
		}
	}
}

void AddItemCategory::slotUpdateItemCategory()
{

}

void AddItemCategory::setCategoryId(QString val)
{
	m_id = val;
}

void AddItemCategory::setUpdate(bool val)
{
	m_isUpdate = val;
}

bool AddItemCategory::isUpdate() const
{
	return m_isUpdate;
}
