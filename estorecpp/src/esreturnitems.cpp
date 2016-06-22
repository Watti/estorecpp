#include "esreturnitems.h"
#include "QSqlQuery"
#include "utility\utility.h"
#include "QMessageBox"
#include "utility\esdbconnection.h"
#include "easylogging++.h"
#include "..\includes\utility\session.h"

ESReturnItems::ESReturnItems(QWidget *parent /*= 0*/) : QWidget(parent)
{
	ui.setupUi(this);
	QObject::connect(ui.addButton, SIGNAL(clicked()), this, SLOT(slotAddReturnedItem()));

	if (!ES::DbConnection::instance()->open())
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Critical);
		mbox.setText(QString("Cannot connect to the database : ESReturnItems"));
		mbox.exec();
	}

// 	QSqlQuery queryCategory("SELECT * FROM item_category WHERE deleted = 0");
// 	QString catCode = DEFAULT_DB_COMBO_VALUE;
// 	int catId = -1;
// 
// 	ui.itemCategoryComboBox->addItem(catCode, catId);
// 
// 	while (queryCategory.next())
// 	{
// 		catId = queryCategory.value(0).toInt();
// 		ui.itemCategoryComboBox->addItem(queryCategory.value(1).toString() + " / " + queryCategory.value("itemcategory_name").toString(), catId);
// 	}
	ui.qtyText->setText("1");
}

ESReturnItems::~ESReturnItems()
{

}

Ui::ReturnItems& ESReturnItems::getUI()
{
	return ui;
}

void ESReturnItems::slotAddReturnedItem()
{
	QString iName = ui.itemName->text();
	QString iCode = ui.itemCode->text();
	QString remarks = ui.remarks->toPlainText();
	//QString catId = ui.itemCategoryComboBox->itemData(ui.itemCategoryComboBox->currentIndex()).toString();
	QString iPrice = ui.itemPrice->text();
	QString qty = ui.qtyText->text();

	if (iCode == nullptr || iCode.isEmpty() ||
		qty == nullptr || qty.isEmpty() ||/* catId == "-1" ||*/ iPrice == nullptr || iPrice.isEmpty())
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Warning);
		mbox.setText(QString("Following fields should not be empty : Item Code, Item Name, Unit, Category, Item Price"));
		mbox.exec();
		return;
	}
	else
	{
		if (qty.toDouble() <= 0)
		{
			QMessageBox mbox;
			mbox.setIcon(QMessageBox::Critical);
			mbox.setText(QString("Invalid value for Quantity"));
			mbox.exec();
			return;
		}
		QString q("SELECT * FROM item WHERE item_code = "+iCode);
		QSqlQuery query(q);
		if (query.next())
		{
			QString itemId = query.value("item_id").toString();
			q = "SELECT * FROM stock WHERE item_id = "+itemId;
			query.exec(q);
			if (query.first())
			{
				double currentQty = query.value("qty").toDouble();
				double currentPrice = query.value("selling_price").toDouble();

				bool isValid = false;
				double returnedItemPrice = iPrice.toDouble(&isValid);
				if (!isValid)
				{
					QMessageBox mbox;
					mbox.setIcon(QMessageBox::Critical);
					mbox.setText(QString("Price should be a number"));
					mbox.exec();
					return;
				}
				if (returnedItemPrice != currentPrice)
				{
					ES::Utility::verifyUsingMessageBox(this, "Return Item", "Price of this item doesn't match with the stock. Do you want to proceed with this price? ");
					QMessageBox mbox;
					mbox.setIcon(QMessageBox::Warning);
					mbox.setText(QString("Price of the returned Item does not match with the prices of the stock"));
					mbox.exec();
				}
				isValid = false;
				double retunedQty = qty.toDouble(&isValid);
				if (!isValid)
				{
					QMessageBox mbox;
					mbox.setIcon(QMessageBox::Critical);
					mbox.setText(QString("Quantity should be a number"));
					mbox.exec();
					return;
				}
				currentQty += retunedQty;
				q = "UPDATE stock set qty = "+QString::number(currentQty)+" WHERE item_id = "+itemId;
				if (!query.exec(q))
				{
					QMessageBox mbox;
					mbox.setIcon(QMessageBox::Critical);
					mbox.setText(QString("Error has been occurred while updating the stock quantity"));
					mbox.exec();
					LOG(ERROR) << "Failed update stock when return item handling query = "<<q.toLatin1().data();
				}
				int uId = ES::Session::getInstance()->getUser()->getId();
				q = "INSERT INTO return_item (item_id, item_price, user_id, remarks) VALUES ("+ itemId + ", " + iPrice + ", " + QString::number(uId) + ",'" +remarks+"')";
				if (!query.exec(q))
				{
					QMessageBox mbox;
					mbox.setIcon(QMessageBox::Critical);
					mbox.setText(QString("Error has been occurred while updating the stock quantity"));
					mbox.exec();
					LOG(ERROR) << "Failed to insert in to return table. query = " << q.toLatin1().data();
				}
				//TODO print the bill
				

			}
			
		}
		else
		{
			QMessageBox mbox;
			mbox.setIcon(QMessageBox::Critical);
			mbox.setText(QString("Invalid item code."));
			mbox.exec();
			return;
		}
	}
	this->close();
}
