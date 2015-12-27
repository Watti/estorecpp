#include "esmanagestockitems.h"
#include "esdbconnection.h"
#include <QMessageBox>
#include <QPushButton>

ESManageStockItems::ESManageStockItems(QWidget *parent /*= 0*/)
	: QWidget(parent)
{
	ui.setupUi(this);

	QStringList headerLabels;
	headerLabels.append("Item Code");
	headerLabels.append("Item Name");
	headerLabels.append("Category");
	headerLabels.append("Qty");
	headerLabels.append("Min.Qty");
	headerLabels.append("Unit");
	headerLabels.append("Unit Price");
	headerLabels.append("Description");
	headerLabels.append("Actions");
	
	ui.tableWidget->setHorizontalHeaderLabels(headerLabels);
	ui.tableWidget->horizontalHeader()->setStretchLastSection(true);
	ui.tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.prevBtn->setDisabled(true);
	ui.nextBtn->setDisabled(true);

	// populate with data
	if (!ES::DbConnection::instance()->open())
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Critical);
		mbox.setText(QString("Cannot connect to the database"));
		mbox.exec();
	}
	else
	{
		QSqlQuery query1("SELECT * FROM item");
		while (query1.next())
		{
			QString itemId = query1.value(0).toString();
			QString itemCategoryId = query1.value(1).toString();

			int row = ui.tableWidget->rowCount();
			ui.tableWidget->insertRow(row);
			ui.tableWidget->setItem(row, 0, new QTableWidgetItem(query1.value(2).toString()));
			ui.tableWidget->setItem(row, 1, new QTableWidgetItem(query1.value(3).toString()));

			QSqlQuery query3("SELECT * FROM item_category WHERE itemcategory_id=" + itemCategoryId);
			if (query3.first())
			{
				ui.tableWidget->setItem(row, 2, new QTableWidgetItem(query3.value(2).toString()));
			}

			QString qty = "Not in stock";
			QString minQty = "Not in stock";
			QString unitPrice = "Not in stock";

			QSqlQuery query2("SELECT * FROM stock WHERE item_id=" + itemId);
			if (query2.first())
			{
				qty = query2.value(3).toString();
				minQty = query2.value(4).toString();

				QSqlQuery query4("SELECT * from item_price WHERE itemprice_id=" + query2.value(2).toString());
				if (query4.first())
				{
					unitPrice = query4.value(2).toString();
				}
			}

			ui.tableWidget->setItem(row, 3, new QTableWidgetItem(qty));
			ui.tableWidget->setItem(row, 4, new QTableWidgetItem(minQty));
			ui.tableWidget->setItem(row, 5, new QTableWidgetItem(query1.value(5).toString()));
			ui.tableWidget->setItem(row, 6, new QTableWidgetItem(unitPrice));
			ui.tableWidget->setItem(row, 7, new QTableWidgetItem(query1.value(6).toString()));

			if (qty == "Not in stock")
			{
				QWidget* base = new QWidget(ui.tableWidget);
				QPushButton* addToStockBtn = new QPushButton("Add To Stock", base);
				addToStockBtn->setMaximumWidth(120);
				addToStockBtn->setMinimumWidth(120);
				QHBoxLayout *layout = new QHBoxLayout;
				layout->setContentsMargins(0, 0, 0, 0);
				layout->addWidget(addToStockBtn);
				layout->insertStretch(1);
				base->setLayout(layout);
				ui.tableWidget->setCellWidget(row, 8, base);
				base->show();
			}
			else
			{
				QWidget* base = new QWidget(ui.tableWidget);
				QPushButton* updateBtn = new QPushButton("Update", base);
				updateBtn->setMaximumWidth(100);
				QPushButton* removeBtn = new QPushButton("Remove", base);
				removeBtn->setMaximumWidth(100);

				QHBoxLayout *layout = new QHBoxLayout;
				layout->setContentsMargins(0, 0, 0, 0);
				layout->addWidget(updateBtn);
				layout->addWidget(removeBtn);
				layout->insertStretch(2);
				base->setLayout(layout);
				ui.tableWidget->setCellWidget(row, 8, base);
				base->show();
			}
		}
	}
}

ESManageStockItems::~ESManageStockItems()
{

}

void ESManageStockItems::slotUpdate()
{

}

void ESManageStockItems::slotRemove()
{

}
