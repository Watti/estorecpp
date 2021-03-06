#include "esmanagesuppliers.h"
#include "esmanagesupplieritem.h"
#include "utility/esdbconnection.h"
#include "utility/esmainwindowholder.h"
#include "esmainwindow.h"
#include <QMessageBox>
#include <QSignalMapper>
#include "utility/utility.h"

ESManageSuppliers::ESManageSuppliers(QWidget *parent /*= 0*/)
: QWidget(parent), m_startingLimit(0), m_pageOffset(15), m_nextCounter(0), m_maxNextCount(0)
{
	ui.setupUi(this);
	ui.tableArea->show();
	ui.detailsArea->hide();
	ui.addSupplierItemBtn->hide();

	m_updateButtonSignalMapper = new QSignalMapper(this);
	m_removeButtonSignalMapper = new QSignalMapper(this);
	m_itemRemoveButtonSignalMapper = new QSignalMapper(this);

	QObject::connect(m_updateButtonSignalMapper, SIGNAL(mapped(QString)), this, SLOT(slotUpdate(QString)));
	QObject::connect(m_removeButtonSignalMapper, SIGNAL(mapped(QString)), this, SLOT(slotRemove(QString)));
	QObject::connect(m_itemRemoveButtonSignalMapper, SIGNAL(mapped(QString)), this, SLOT(slotRemoveItem(QString)));
	QObject::connect(ui.addNewSupplier, SIGNAL(clicked()), this, SLOT(slotShowAddSupplierView()));
	QObject::connect(ui.addButton, SIGNAL(clicked()), this, SLOT(slotAddSupplier()));
	QObject::connect(ui.updateButton, SIGNAL(clicked()), this, SLOT(slotUpdateSupplier()));
	QObject::connect(ui.addSupplierItemBtn, SIGNAL(clicked()), this, SLOT(slotShowAddSupplierItemView()));
	QObject::connect(ui.searchTextBox, SIGNAL(textChanged(QString)), this, SLOT(slotSearch()));
	QObject::connect(ui.categoryComboBox, SIGNAL(activated(QString)), this, SLOT(slotSearch()));

	QStringList headerLabels;
	headerLabels.append("Supplier Code");
	headerLabels.append("Supplier Name");
	headerLabels.append("Phone");
	headerLabels.append("Fax");
	headerLabels.append("E-mail");
	headerLabels.append("Address");
	headerLabels.append("Actions");

	ui.tableWidget->setHorizontalHeaderLabels(headerLabels);
	ui.tableWidget->horizontalHeader()->setStretchLastSection(true);
	ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);

	QStringList headerLabels1;
	headerLabels1.append("Item Code");
	headerLabels1.append("Item Name");
	headerLabels1.append("Category");
	headerLabels1.append("Purchasing Price");
	headerLabels1.append("Actions");

	ui.currentItems->setHorizontalHeaderLabels(headerLabels1);
	ui.currentItems->horizontalHeader()->setStretchLastSection(true);
	ui.currentItems->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.currentItems->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.currentItems->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.currentItems->setSelectionMode(QAbstractItemView::SingleSelection);

	if (!ES::DbConnection::instance()->open())
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Critical);
		mbox.setText(QString("Database Connection Error : Suppliers"));
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

ESManageSuppliers::~ESManageSuppliers()
{

}

void ESManageSuppliers::slotAddSupplier()
{
	if (ui.name->text() == nullptr || ui.name->text().isEmpty())
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Warning);
		mbox.setText(QString("Supplier Name Cannot be empty"));
		mbox.exec();
		return;
	}

	QString query;
	query.append("INSERT INTO supplier (supplier_code, supplier_name, phone, fax, email, address) VALUES ('");
	query.append(ui.code->text());
	query.append("', '");
	query.append(ui.name->text());
	query.append("', ");
	QString  phone = ui.phone->text();
	bool phoneOk = false;
	phone.toInt(&phoneOk, 10);
	
	if (phone == nullptr || phone.isEmpty())
	{
		phone = "''";
	}
	query.append(phone);
	query.append(", ");
	QString fax = ui.fax->text();
	bool faxOk = false;
	fax.toInt(&faxOk, 10);
	if (!phoneOk || !faxOk)
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Warning);
		mbox.setText(QString("Phone number or Fax number cannot contain text"));
		mbox.exec();
		return;
	}
	if (fax == nullptr || fax.isEmpty())
	{
		fax = "''";
	}
	query.append(fax);
	query.append(", '");
	query.append(ui.email->text());
	query.append("', '");
	query.append(ui.address->toPlainText());
	query.append("')");

	QSqlQuery q;
	if (!q.exec(query))
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Critical);
		mbox.setText(QString("Error: Insertion failed"));
		mbox.exec();
	}
	else
	{
		ui.tableArea->show();
		ui.detailsArea->hide();
		slotSearch();
	}
}

void ESManageSuppliers::slotShowAddSupplierView()
{
	ui.tableArea->hide();
	ui.updateButton->hide();
	ui.detailsArea->show();
	ui.addButton->show();
}

void ESManageSuppliers::slotSearch()
{
	QString searchText = ui.searchTextBox->text();
	int categoryId = ui.categoryComboBox->currentData().toInt();

	if (categoryId == -1)
	{
		QString qry("SELECT * FROM supplier WHERE deleted = 0 ");
		QString countQueryStr("SELECT COUNT(*) AS c FROM supplier WHERE deleted = 0 ");
		if (!searchText.isEmpty())
		{
			qry.append(" AND (supplier_code LIKE '%" + searchText + "%' OR supplier_name LIKE '%" + searchText + "%')");
			countQueryStr.append(" AND (supplier_code LIKE '%" + searchText + "%' OR supplier_name LIKE '%" + searchText + "%')");
		}

		while (ui.tableWidget->rowCount() > 0)
		{
			ui.tableWidget->removeRow(0);
		}
		//pagination start
		QSqlQuery queryCount(countQueryStr);
		if (queryCount.next())
		{
			m_totalRecords = queryCount.value("c").toInt();
		}
		qry.append(" LIMIT ").append(QString::number(m_startingLimit));
		qry.append(" , ").append(QString::number(m_pageOffset));
		//pagination end
		QSqlQuery q(qry);

		//pagination start
		m_maxNextCount = m_totalRecords / m_pageOffset;

		if (m_maxNextCount > m_nextCounter)
		{
			ui.nextBtn->setEnabled(true);
		}
		int currentlyShowdItemCount = (m_nextCounter + 1)*m_pageOffset;
		int displayMaxBound = (m_nextCounter + 1)*m_pageOffset;
		if (m_nextCounter == 0)
		{
			displayMaxBound = m_pageOffset;
		}
		int displayMinBound = 0;
		if (m_nextCounter >= 1)
		{
			displayMinBound = (m_nextCounter)* m_pageOffset;
		}
		if (currentlyShowdItemCount >= m_totalRecords)
		{
			displayMaxBound = m_totalRecords;
			ui.nextBtn->setDisabled(true);
		}
		QString displayPaginationStr = QString::number(displayMinBound);
		displayPaginationStr.append(" to ").append(QString::number(displayMaxBound));
		ui.noOfRecordsLbl->setText(displayPaginationStr);
		//pagination end
		while (q.next())
		{
			int row = ui.tableWidget->rowCount();
			ui.tableWidget->insertRow(row);

			ui.tableWidget->setItem(row, 0, new QTableWidgetItem(q.value("supplier_code").toString()));
			ui.tableWidget->setItem(row, 1, new QTableWidgetItem(q.value("supplier_name").toString()));
			ui.tableWidget->setItem(row, 2, new QTableWidgetItem(q.value("phone").toString()));
			ui.tableWidget->setItem(row, 3, new QTableWidgetItem(q.value("fax").toString()));
			ui.tableWidget->setItem(row, 4, new QTableWidgetItem(q.value("email").toString()));
			ui.tableWidget->setItem(row, 5, new QTableWidgetItem(q.value("address").toString()));

			QString supplierId = q.value("supplier_id").toString();

			QWidget* base = new QWidget(ui.tableWidget);
			QPushButton* updateBtn = new QPushButton(base);
			updateBtn->setIcon(QIcon("icons/update.png"));
			updateBtn->setIconSize(QSize(24, 24));
			updateBtn->setMaximumWidth(100);

			QPushButton* removeBtn = new QPushButton(base);
			removeBtn->setIcon(QIcon("icons/delete.png"));
			removeBtn->setIconSize(QSize(24, 24));
			removeBtn->setMaximumWidth(100);

			m_updateButtonSignalMapper->setMapping(updateBtn, supplierId);
			QObject::connect(updateBtn, SIGNAL(clicked()), m_updateButtonSignalMapper, SLOT(map()));

			QObject::connect(removeBtn, SIGNAL(clicked()), m_removeButtonSignalMapper, SLOT(map()));
			m_removeButtonSignalMapper->setMapping(removeBtn, supplierId);

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
	else
	{
		while (ui.tableWidget->rowCount() > 0)
		{
			ui.tableWidget->removeRow(0);
		}
		QString qry("SELECT supplier_item.item_id, supplier.supplier_code, supplier.supplier_name, supplier.phone, supplier.fax, supplier.email, supplier.address FROM supplier_item JOIN supplier ON supplier_item.supplier_id = supplier.	supplier_id WHERE supplier.deleted = 0 AND supplier_item.deleted = 0");
		QSqlQuery q(qry);
		while (q.next())
		{
			QString itemId = q.value("item_id").toString();
			qry = "SELECT * FROM item WHERE item.itemcategory_id = " + QString::number(categoryId);
			QSqlQuery queryFilteredItems(qry);
			while (queryFilteredItems.next())
			{
				if (queryFilteredItems.value("item_id").toString() == itemId)
				{
					int row = ui.tableWidget->rowCount();
					ui.tableWidget->insertRow(row);

					ui.tableWidget->setItem(row, 0, new QTableWidgetItem(q.value("supplier_code").toString()));
					ui.tableWidget->setItem(row, 1, new QTableWidgetItem(q.value("supplier_name").toString()));
					ui.tableWidget->setItem(row, 2, new QTableWidgetItem(q.value("phone").toString()));
					ui.tableWidget->setItem(row, 3, new QTableWidgetItem(q.value("fax").toString()));
					ui.tableWidget->setItem(row, 4, new QTableWidgetItem(q.value("email").toString()));
					ui.tableWidget->setItem(row, 5, new QTableWidgetItem(q.value("address").toString()));

					QString supplierId = q.value("supplier_id").toString();

					QWidget* base = new QWidget(ui.tableWidget);
					QPushButton* updateBtn = new QPushButton("Update", base);
					updateBtn->setMaximumWidth(100);
					QPushButton* removeBtn = new QPushButton("Remove", base);
					removeBtn->setMaximumWidth(100);

					m_updateButtonSignalMapper->setMapping(updateBtn, supplierId);
					QObject::connect(updateBtn, SIGNAL(clicked()), m_updateButtonSignalMapper, SLOT(map()));

					QObject::connect(removeBtn, SIGNAL(clicked()), m_removeButtonSignalMapper, SLOT(map()));
					m_removeButtonSignalMapper->setMapping(removeBtn, supplierId);

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
		}

	}
}

void ESManageSuppliers::slotUpdate(QString id)
{
	m_supplierId = id;

	ui.tableArea->hide();
	ui.addButton->hide();
	ui.detailsArea->show();
	ui.updateButton->show();
	ui.addSupplierItemBtn->show();

	QSqlQuery qs("SELECT * FROM supplier WHERE deleted = 0 AND supplier_id = " + m_supplierId);
	if (qs.next())
	{
		ui.code->setText(qs.value("supplier_code").toString());
		ui.name->setText(qs.value("supplier_name").toString());
		ui.phone->setText(qs.value("phone").toString());
		ui.fax->setText(qs.value("fax").toString());
		ui.email->setText(qs.value("email").toString());
		ui.address->setPlainText(qs.value("address").toString());
	}

	// populate current items
	QSqlQuery q("SELECT * FROM supplier_item WHERE deleted = 0 AND supplier_id = " + m_supplierId);

	while (ui.currentItems->rowCount() > 0)
	{
		ui.currentItems->removeRow(0);
	}

	while (q.next())
	{
		int row = ui.currentItems->rowCount();
		ui.currentItems->insertRow(row);

		QString itemId = q.value("item_id").toString();

		QSqlQuery qItems("SELECT * FROM item WHERE deleted= 0 AND item_id = " + itemId);
		if (qItems.next())
		{
			QSqlQuery qCategory("SELECT * FROM item_category WHERE deleted= 0 AND itemcategory_id = " + qItems.value("itemcategory_id").toString());
			if (qCategory.next())
			{
				ui.currentItems->setItem(row, 0, new QTableWidgetItem(qItems.value("item_code").toString()));
				ui.currentItems->setItem(row, 1, new QTableWidgetItem(qItems.value("item_name").toString()));
				ui.currentItems->setItem(row, 2, new QTableWidgetItem(qCategory.value("itemcategory_name").toString()));

				double price = q.value("purchasing_price").toDouble();
				QString formattedPrice = QString::number(price, 'f', 2);
				ui.currentItems->setItem(row, 3, new QTableWidgetItem(formattedPrice));

				//
				QWidget* base = new QWidget(ui.currentItems);
				QPushButton* removeBtn = new QPushButton("Remove", base);
				removeBtn->setMaximumWidth(100);

 				QObject::connect(removeBtn, SIGNAL(clicked()), m_itemRemoveButtonSignalMapper, SLOT(map()));
				m_itemRemoveButtonSignalMapper->setMapping(removeBtn, itemId.append(";").append(m_supplierId));

				QHBoxLayout *layout = new QHBoxLayout;
				layout->setContentsMargins(0, 0, 0, 0);
				layout->addWidget(removeBtn);
				layout->insertStretch(2);
				base->setLayout(layout);
				ui.currentItems->setCellWidget(row, 4, base);
				base->show();
				//
			}
		}
	}
}

void ESManageSuppliers::slotRemove(QString id)
{
	if (ES::Utility::verifyUsingMessageBox(this, "Progex", "Do you really want to remove this?"))
	{
		QString str("UPDATE supplier SET deleted = 1 WHERE supplier_id = " + id);
		QSqlQuery q;
		if (q.exec(str))
		{
			while (ui.tableWidget->rowCount() > 0)
			{
				ui.tableWidget->removeRow(0);
			}

			slotSearch();
		}
	}
}

void ESManageSuppliers::slotShowAddSupplierItemView()
{
	ESManageSupplierItem* manageSupplierItems = new ESManageSupplierItem(m_supplierId, this);
	ES::MainWindowHolder::instance()->getMainWindow()->setCentralWidget(manageSupplierItems);
	manageSupplierItems->show();
}

void ESManageSuppliers::slotUpdateSupplier()
{
	QString query;
	query.append("UPDATE supplier SET supplier_code = '");
	query.append(ui.code->text());
	query.append("', supplier_name = '");
	query.append(ui.name->text());
	query.append("', phone = ");
	QString phone = ui.phone->text();
	if (phone == nullptr || phone.isEmpty())
	{
		phone = "''";
	}
	bool phoneOk = false;
	phone.toInt(&phoneOk, 10);

	query.append(phone);
	query.append(", fax = ");
	//query.append(ui.fax->text());

	QString fax = ui.fax->text();
	bool faxOk = false;
	fax.toInt(&faxOk, 10);
	if (!phoneOk || !faxOk)
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Warning);
		mbox.setText(QString("Phone number or Fax number cannot contain text"));
		mbox.exec();
		return;
	}
	if (fax == nullptr || fax.isEmpty())
	{
		fax = "''";
	}
	query.append(fax);
	query.append(", email = '");
	query.append(ui.email->text());
	query.append("', address = '");
	query.append(ui.address->toPlainText());
	query.append("' WHERE supplier_id = ");
	query.append(m_supplierId);

	QSqlQuery q;
	if (!q.exec(query))
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Critical);
		mbox.setText(QString("Error: Update failed"));
		mbox.exec();
	}
	else
	{
		ui.tableArea->show();
		ui.detailsArea->hide();
		slotSearch();
	}

}

void ESManageSuppliers::slotPrev()
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

void ESManageSuppliers::slotNext()
{
	if (m_nextCounter < m_maxNextCount)
	{
		m_nextCounter++;
		ui.prevBtn->setEnabled(true);
		m_startingLimit += m_pageOffset;
	}
	slotSearch();
}



 void ESManageSuppliers::slotRemoveItem(QString itemId)
 {
	 if (ES::Utility::verifyUsingMessageBox(this, "Progex", "Do you really want to remove this?"))
	 {
		 QStringList tokens = itemId.split(";");

		 if (tokens.count() == 2) {
			 QString iId = tokens.at(0);
			 QString supplierId = tokens.at(1);
			 QString queryStr = "DELETE FROM supplier_item WHERE item_id = " + iId + " AND supplier_id =" + supplierId;
			 QSqlQuery q(queryStr);
			 slotUpdate(m_supplierId);

			 
		 }

	 }
}
