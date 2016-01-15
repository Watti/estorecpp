#include "esadditemcategory.h"
#include "utility/esdbconnection.h"
#include <QMessageBox>
#include <QPushButton>


AddItemCategory::AddItemCategory(QWidget *parent /*= 0*/)
: QWidget(parent)
{
	ui.setupUi(this);
	QObject::connect(ui.addItemCategoryButton, SIGNAL(clicked()), this, SLOT(slotAddItemCategory()));
}

AddItemCategory::~AddItemCategory()
{

}

void AddItemCategory::slotAddItemCategory()
{
	if (ES::DbConnection::instance()->open())
	{
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
		QString q("INSERT INTO item_category(itemcategory_code, itemcategory_name, description, deleted) VALUES ('"+cCode+"','"+cName+"','"+cDesc+"', 0)");

		QSqlQuery query;
		if (query.exec(q))
		{
			this->close();
		}
		else
		{
			QMessageBox mbox;
			mbox.setIcon(QMessageBox::Critical);
			mbox.setText(QString("Something goes wrong:: Category cannot be saved"));
			mbox.exec();
		}
	}
	else
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Critical);
		mbox.setText(QString("Cannot connect to the database : AddItemCategory::slotAddItemCategory "));
		mbox.exec();
	}

}
