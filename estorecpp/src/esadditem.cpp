#include "esadditem.h"
#include "utility/esdbconnection.h"
#include <QMessageBox>
#include <QPushButton>


AddItem::AddItem(QWidget *parent /*= 0*/)
: QWidget(parent)
{
	ui.setupUi(this);

	QSqlQuery queryCategory("SELECT * FROM item_category");
	QString catCode = "select";
	int catId = -1;

	ui.itemCategoryComboBox->addItem(catCode, catId);

	while (queryCategory.next())
	{
		catId = queryCategory.value(0).toInt();
		ui.itemCategoryComboBox->addItem(queryCategory.value(1).toString(), catId);
	}
}

AddItem::~AddItem()
{

}

void AddItem::slotAddItem()
{

}
