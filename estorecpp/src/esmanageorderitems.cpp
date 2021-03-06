#include "esmanageorderitems.h"
#include "utility/esmainwindowholder.h"
#include "utility/esdbconnection.h"
#include <QMessageBox>
#include <QPushButton>
#include "utility/utility.h"
#include "esaddorderitem.h"
#include "esordercheckin.h"
#include "esmainwindow.h"

ESManageOrderItems::ESManageOrderItems(QWidget *parent/* = 0*/) : 
m_startingLimit(0), m_pageOffset(15), m_nextCounter(0), m_maxNextCount(0)
{
	ui.setupUi(this);

	m_updateButtonSignalMapper = new QSignalMapper(this);
	m_removeButtonSignalMapper = new QSignalMapper(this);

	QObject::connect(m_updateButtonSignalMapper, SIGNAL(mapped(QString)), this, SLOT(slotUpdate(QString)));
	QObject::connect(m_removeButtonSignalMapper, SIGNAL(mapped(QString)), this, SLOT(slotRemove(QString)));
	QObject::connect(ui.userSearch, SIGNAL(textChanged(QString)), this, SLOT(slotSearch()));
	QObject::connect(ui.supplierSearch, SIGNAL(textChanged(QString)), this, SLOT(slotSearch()));
	QObject::connect(ui.addOrderItemBtn, SIGNAL(clicked()), this, SLOT(slotAddNewOrderItem()));
	QObject::connect(ui.showAllOrders, SIGNAL(stateChanged(int)), this, SLOT(slotSearch()));
	QObject::connect(ui.nextBtn, SIGNAL(clicked()), this, SLOT(slotNext()));
	QObject::connect(ui.prevBtn, SIGNAL(clicked()), this, SLOT(slotPrev()));

	QStringList headerLabels;
	headerLabels.append("Order ID");	
	headerLabels.append("Supplier ID");
	headerLabels.append("Supplier Name");
	headerLabels.append("Placed Date");
	headerLabels.append("Comments");
	headerLabels.append("User");
	headerLabels.append("Actions");

	ui.tableWidget->setHorizontalHeaderLabels(headerLabels);
	ui.tableWidget->horizontalHeader()->setStretchLastSection(true);
	ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.prevBtn->setDisabled(true);
	ui.nextBtn->setDisabled(true);

	ui.showAllOrders->setChecked(true);

	if (!ES::DbConnection::instance()->open())
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Critical);
		mbox.setText(QString("Cannot connect to the database : ESManageOrderItems::ESManageOrderItems "));
		mbox.exec();
	}
	else
	{
		slotSearch();
	}
}

ESManageOrderItems::~ESManageOrderItems()
{

}

void ESManageOrderItems::slotAddNewOrderItem()
{
	AddOrderItem* addOrderItem = new AddOrderItem(this);
	ES::MainWindowHolder::instance()->getMainWindow()->setCentralWidget(addOrderItem);
	addOrderItem->show();
}

void ESManageOrderItems::slotSearch()
{
	QString searchUser = ui.userSearch->text();
	QString searchSupplier = ui.supplierSearch->text();

	while (ui.tableWidget->rowCount() > 0)
	{
		ui.tableWidget->removeRow(0);
	}

	QString uId = nullptr;
	bool userGiven = false, supplierGiven = false;
	if (searchUser != nullptr && !searchUser.isEmpty())
	{
		userGiven = true;
	}
	if (searchSupplier != nullptr && !searchSupplier.isEmpty())
	{
		supplierGiven = true;
	}

	QString searchQuery = "(SELECT * FROM purchase_order WHERE deleted = 0 ";
	QString countQueryStr = "SELECT COUNT(*) AS c FROM purchase_order WHERE deleted = 0 ";//to count no of records in the above query
	if (!ui.showAllOrders->isChecked())
	{
		searchQuery.append("AND checked_in = 0");
		countQueryStr.append("AND checked_in = 0");
	}

	if (!supplierGiven && userGiven)
	{
		searchQuery = "(SELECT * FROM purchase_order so, user u WHERE so.user_id = u.user_id AND u.username LIKE '%" + searchUser + "%' AND so.deleted = 0 ";
		countQueryStr = "SELECT COUNT(*) as c FROM purchase_order so, user u WHERE so.user_id = u.user_id AND u.username LIKE '%" + searchUser + "%' AND so.deleted = 0 ";
		if (!ui.showAllOrders->isChecked())
		if (!ui.showAllOrders->isChecked())
		{
			searchQuery.append("AND checked_in = 0");
			countQueryStr.append("AND checked_in = 0");
		}
	}
	else if (supplierGiven && !userGiven)
	{
		searchQuery = "(SELECT * FROM purchase_order so, supplier i WHERE so.supplier_id = i.supplier_id AND i.supplier_name LIKE '%" + searchSupplier + "%' AND so.deleted = 0 ";
		countQueryStr = "SELECT COUNT(*) as c  FROM purchase_order so, supplier i WHERE so.supplier_id = i.supplier_id AND i.supplier_name LIKE '%" + searchSupplier + "%' AND so.deleted = 0 ";
		if (!ui.showAllOrders->isChecked())
		{
			searchQuery.append("AND checked_in = 0");
			countQueryStr.append("AND checked_in = 0");
		}
	}
	else if (supplierGiven && userGiven)
	{
		searchQuery = "(SELECT * FROM purchase_order so, user u , supplier i WHERE (so.user_id = u.user_id AND so.supplier_id = i.supplier_id) AND u.username LIKE '%" + searchUser + "%' AND i.supplier_name LIKE '%" + searchSupplier + "%' AND so.deleted = 0 ";
		countQueryStr = "SELECT COUNT(*) as c FROM purchase_order so, user u , supplier i WHERE (so.user_id = u.user_id AND so.supplier_id = i.supplier_id) AND u.username LIKE '%" + searchUser + "%' AND i.supplier_name LIKE '%" + searchSupplier + "%' AND so.deleted = 0 ";
		if (!ui.showAllOrders->isChecked())
		{
			searchQuery.append("AND checked_in = 0");
			countQueryStr.append("AND checked_in = 0");
		}
	}
	ui.tableWidget->setSortingEnabled(false);
	searchQuery.append(") ORDER BY order_date DESC ");
	//pagination start
	QSqlQuery queryCount(countQueryStr);
	if (queryCount.next())
	{
		m_totalRecords = queryCount.value("c").toInt();
	}
	searchQuery.append(" LIMIT ").append(QString::number(m_startingLimit));
	searchQuery.append(" , ").append(QString::number(m_pageOffset));
	//pagination end
	QSqlQuery queryItems(searchQuery);
	displayItems(queryItems);
	ui.tableWidget->setSortingEnabled(true);
}

void ESManageOrderItems::displayItems(QSqlQuery& queryOrder)
{
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
	int row = 0;
	while (queryOrder.next())
	{
		row = ui.tableWidget->rowCount();
		ui.tableWidget->insertRow(row);
		QString poId = queryOrder.value("purchaseorder_id").toString();
		bool checkedIn = (1 == queryOrder.value("checked_in").toInt());
		QColor red(245, 169, 169);
		QColor green(169, 245, 208);
		QTableWidgetItem* tbItem = NULL;
		tbItem = new QTableWidgetItem(poId);
		tbItem->setBackgroundColor(checkedIn ? red : green);
		
		ui.tableWidget->setItem(row, 0, tbItem);

		QSqlQuery q("SELECT * FROM supplier WHERE deleted = 0 AND supplier_id = " + queryOrder.value("supplier_id").toString());
		if (q.next())
		{
			tbItem = new QTableWidgetItem(q.value("supplier_code").toString());
			tbItem->setBackgroundColor(checkedIn ? red : green);
			ui.tableWidget->setItem(row, 1, tbItem);
			
			tbItem = new QTableWidgetItem(q.value("supplier_name").toString());
			tbItem->setBackgroundColor(checkedIn ? red : green);
			ui.tableWidget->setItem(row, 2, tbItem);
		}

		QDate orderDate = QDate::fromString(queryOrder.value("order_date").toString(), "yyyy-MM-dd");
		tbItem = new QTableWidgetItem(queryOrder.value("order_date").toString().append(" (").append(orderDate.toString("MMMM dd")).append(")"));
		tbItem->setBackgroundColor(checkedIn ? red : green);
		ui.tableWidget->setItem(row, 3, tbItem);
		
		tbItem = new QTableWidgetItem(queryOrder.value("comments").toString());
		tbItem->setBackgroundColor(checkedIn ? red : green);
		ui.tableWidget->setItem(row, 4, tbItem);
		QSqlQuery queryUser("SELECT * FROM user WHERE user_id = " + queryOrder.value("user_id").toString());
		if (queryUser.next())
		{
			tbItem = new QTableWidgetItem(queryUser.value("display_name").toString());
			tbItem->setBackgroundColor(checkedIn ? red : green);
			ui.tableWidget->setItem(row, 5, tbItem);
		}

		QWidget* base = new QWidget(ui.tableWidget);
		QPalette p(base->palette());
		p.setColor(QPalette::Background, checkedIn ? red : green);
		base->setAutoFillBackground(true);
		base->setPalette(p);
		QPushButton* checkInBtn = NULL;

		if (!checkedIn)
		{
			checkInBtn = new QPushButton(base);
			checkInBtn->setIcon(QIcon("icons/checkin.png"));
			checkInBtn->setIconSize(QSize(24, 24));
			checkInBtn->setMaximumWidth(100);
		}
		QPushButton* removeBtn = new QPushButton(base);
		removeBtn->setIcon(QIcon("icons/delete.png"));
		removeBtn->setIconSize(QSize(24, 24));
		removeBtn->setMaximumWidth(100);

		if (!checkedIn)
		{
			m_updateButtonSignalMapper->setMapping(checkInBtn, poId);
			QObject::connect(checkInBtn, SIGNAL(clicked()), m_updateButtonSignalMapper, SLOT(map()));
		}

		m_removeButtonSignalMapper->setMapping(removeBtn, poId);
		QObject::connect(removeBtn, SIGNAL(clicked()), m_removeButtonSignalMapper, SLOT(map()));

		QHBoxLayout *layout = new QHBoxLayout;
		layout->setContentsMargins(0, 0, 0, 0);
		if (!checkedIn)
		{
			layout->addWidget(checkInBtn);
		}
		layout->addWidget(removeBtn);
		layout->insertStretch(2);
		base->setLayout(layout);
		ui.tableWidget->setCellWidget(row, 6, base);
		base->show();
	}
}

void ESManageOrderItems::slotUpdate(QString orderId)
{
	ESOrderCheckIn* orderCheckin = new ESOrderCheckIn(orderId, this);
	ES::MainWindowHolder::instance()->getMainWindow()->setCentralWidget(orderCheckin);
	orderCheckin->show();
}

void ESManageOrderItems::slotRemove(QString order_id)
{
	if (ES::Utility::verifyUsingMessageBox(this, "Progex", "Do you really want to remove this?"))
	{
		QString str("UPDATE purchase_order SET deleted = 1 WHERE 	purchaseorder_id = " + order_id);
		QSqlQuery q;
		if (q.exec(str))
		{
			while (ui.tableWidget->rowCount() > 0)
			{
				ui.tableWidget->removeRow(0);
			}

			QString displayQueryStr("(SELECT * from purchase_order WHERE deleted = 0 ) ORDER BY order_date DESC ");
			displayQueryStr.append(" LIMIT ").append(QString::number(m_startingLimit));
			displayQueryStr.append(" , ").append(QString::number(m_pageOffset));
			QSqlQuery countQuery("SELECT COUNT(*) as c FROM purchase_order WHERE deleted = 0");
			if (countQuery.next())
			{
				m_totalRecords = countQuery.value("c").toInt();
			}
			QSqlQuery displayQuery(displayQueryStr);
			displayItems(displayQuery);
		}
	}
}

void ESManageOrderItems::slotPrev()
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

void ESManageOrderItems::slotNext()
{
	if (m_nextCounter < m_maxNextCount)
	{
		m_nextCounter++;
		ui.prevBtn->setEnabled(true);
		m_startingLimit += m_pageOffset;
	}
	slotSearch();
}
