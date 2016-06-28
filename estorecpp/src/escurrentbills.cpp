#include "escurrentbills.h"
#include "utility/esmainwindowholder.h"
#include "utility/esdbconnection.h"
#include <QMessageBox>
#include "QPushButton"
#include "esaddbill.h"
#include "esmainwindow.h"
#include "utility/session.h"
#include "entities/user.h"
#include "utility/utility.h"

ESCurrentBills::ESCurrentBills(QWidget *parent)
: QWidget(parent)
{
	ui.setupUi(this);
	m_proceedButtonSignalMapper = new QSignalMapper(this);
	m_voidBillButtonSignalMapper = new QSignalMapper(this);
	QObject::connect(m_proceedButtonSignalMapper, SIGNAL(mapped(QString)), this, SLOT(slotProceed(QString)));
	QObject::connect(m_voidBillButtonSignalMapper, SIGNAL(mapped(QString)), this, SLOT(slotVoidBill(QString)));

	ui.startDate->setDisplayFormat("yyyy-MM-dd");
	ui.endDate->setDisplayFormat("yyyy-MM-dd");

	QStringList headerLabels;
	headerLabels.append("Bill ID");
	headerLabels.append("Date");
	//headerLabels.append("Payment Method");
	headerLabels.append("Amount");
	headerLabels.append("User");
	headerLabels.append("Status");
	headerLabels.append("Actions");

	ui.tableWidget->setHorizontalHeaderLabels(headerLabels);
	ui.tableWidget->horizontalHeader()->setStretchLastSection(true);
	ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);

	QObject::connect(ui.userComboBox, SIGNAL(activated(QString)), this, SLOT(slotSearch()));
	QObject::connect(ui.statusComboBox, SIGNAL(activated(QString)), this, SLOT(slotSearch()));
	QObject::connect(ui.startDate, SIGNAL(dateChanged(const QDate&)), this, SLOT(slotSearch()));
	QObject::connect(ui.endDate, SIGNAL(dateChanged(const QDate&)), this, SLOT(slotSearch()));

	if (!ES::DbConnection::instance()->open())
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Critical);
		mbox.setText(QString("Cannot connect to the database : ESManageStockItems::displayStockItems "));
		mbox.exec();
	}
	else
	{
		QSqlQuery queryUser("SELECT * FROM user");
		ui.userComboBox->addItem("ALL", 0);
		while (queryUser.next())
		{
			ui.userComboBox->addItem(queryUser.value(1).toString(), queryUser.value(0).toInt());
		}

		ui.statusComboBox->addItem("ALL", 0);
		ui.statusComboBox->addItem("COMMITTED", 1);
		ui.statusComboBox->addItem("PENDING", 2);
		ui.statusComboBox->addItem("CANCELED", 3);

		ui.startDate->setDate(QDate::currentDate().addMonths(-1));
		ui.endDate->setDate(QDate::currentDate());

	}

	slotSearch();
}

ESCurrentBills::~ESCurrentBills()
{

}

void ESCurrentBills::slotSearch()
{
	while (ui.tableWidget->rowCount() > 0)
	{
		ui.tableWidget->removeRow(0);
	}

	int selectedUser = ui.userComboBox->currentData().toInt();
	int selectedStatus = ui.statusComboBox->currentData().toInt();

	QDateTime startDate = QDateTime::fromString(ui.startDate->text(), Qt::ISODate);
	QDateTime endDate = QDateTime::fromString(ui.endDate->text(), Qt::ISODate);
	endDate.setTime(QTime(23,59, 59));

	int row = 0;
	QSqlQuery allBillQuery("SELECT * FROM bill WHERE deleted = 0 AND DATE(date) = DATE(CURDATE())");
	while (allBillQuery.next())
	{		
		if (selectedUser > 0)
		{
			if (selectedUser != allBillQuery.value("user_id").toInt())
			{
				continue;
			}
		}
		QDateTime billedDate = QDateTime::fromString(allBillQuery.value("date").toString(), Qt::ISODate);
		if (billedDate < startDate || billedDate > endDate)
		{
			continue;
		}
		int statusId = allBillQuery.value("status").toInt();
		if (selectedStatus > 0)
		{
			if (selectedStatus != statusId)
			{
				continue;
			}
		}

		QTableWidgetItem* tableItem = NULL;
		row = ui.tableWidget->rowCount();
		ui.tableWidget->insertRow(row);		
		QColor rowColor;
		QString billId = allBillQuery.value("bill_id").toString();
		switch (statusId)
		{
		case 1:
		{
			rowColor.setRgb(169, 245, 208);
			tableItem = new QTableWidgetItem("COMMITTED");
			tableItem->setTextAlignment(Qt::AlignHCenter);
			tableItem->setBackgroundColor(rowColor);
			ui.tableWidget->setItem(row, 4, tableItem);

			ES::User::UserType uType = ES::Session::getInstance()->getUser()->getType();
			if (uType == ES::User::SENIOR_MANAGER || uType == ES::User::DEV || uType == ES::User::MANAGER)
			{
				QWidget* base = new QWidget(ui.tableWidget);
				QPushButton* voidBtn = new QPushButton("Cancel", base);
				voidBtn->setMaximumWidth(100);
				m_voidBillButtonSignalMapper->setMapping(voidBtn, billId);
				QObject::connect(voidBtn, SIGNAL(clicked()), m_voidBillButtonSignalMapper, SLOT(map()));

				QHBoxLayout *layout = new QHBoxLayout;
				layout->setContentsMargins(0, 0, 0, 0);
				layout->addWidget(voidBtn);
				layout->insertStretch(2);
				base->setLayout(layout);
				ui.tableWidget->setCellWidget(row, 5, base);
				base->show();
			}
		}
			break;
		case 2:
		{
			rowColor.setRgb(254, 239, 179);
			tableItem = new QTableWidgetItem("PENDING");
			tableItem->setTextAlignment(Qt::AlignHCenter);
			tableItem->setBackgroundColor(rowColor);
			ui.tableWidget->setItem(row, 4, tableItem);

			QWidget* base = new QWidget(ui.tableWidget);
			QPushButton* proceedBtn = new QPushButton("Proceed", base);
			proceedBtn->setMaximumWidth(100);

			m_proceedButtonSignalMapper->setMapping(proceedBtn, billId);
			QObject::connect(proceedBtn, SIGNAL(clicked()), m_proceedButtonSignalMapper, SLOT(map()));
			
			QHBoxLayout *layout = new QHBoxLayout;
			layout->setContentsMargins(0, 0, 0, 0);
			layout->addWidget(proceedBtn);
			layout->insertStretch(2);
			base->setLayout(layout);
			ui.tableWidget->setCellWidget(row, 5, base);
			base->show();
		}
			break;
		case 3:
		{
			rowColor.setRgb(245, 169, 169);
			tableItem = new QTableWidgetItem("CANCELED");
			tableItem->setTextAlignment(Qt::AlignHCenter);
			tableItem->setBackgroundColor(rowColor);
			ui.tableWidget->setItem(row, 4, tableItem);
		}
			break;
		default:
		{
			rowColor.setRgb(255, 255, 255);
			tableItem = new QTableWidgetItem("UNSPECIFIED");
			tableItem->setTextAlignment(Qt::AlignHCenter);
			tableItem->setBackgroundColor(rowColor);
			ui.tableWidget->setItem(row, 4, tableItem);
		}
			
			break;
		}

		tableItem = new QTableWidgetItem(billId);
		tableItem->setBackgroundColor(rowColor);
		ui.tableWidget->setItem(row, 0, tableItem);
		QDateTime datetime = QDateTime::fromString(allBillQuery.value("date").toString(), Qt::ISODate);
		tableItem = new QTableWidgetItem(datetime.toString(Qt::SystemLocaleShortDate));
		tableItem->setBackgroundColor(rowColor);
		ui.tableWidget->setItem(row, 1, tableItem);


		QSqlQuery qSales("SELECT SUM(total) as GTot FROM sale WHERE bill_id = " + billId);
		if (qSales.next())
		{
			tableItem = new QTableWidgetItem(QString::number(qSales.value("GTot").toFloat(), 'f', 2));
			tableItem->setTextAlignment(Qt::AlignRight);
			tableItem->setBackgroundColor(rowColor);
			ui.tableWidget->setItem(row, 2, tableItem);
		}

		QString userQueryStr("SELECT * FROM user WHERE user_id=");
		userQueryStr.append(allBillQuery.value("user_id").toString());
		QSqlQuery userQuery(userQueryStr);
		if (userQuery.first())
		{
			tableItem = new QTableWidgetItem(userQuery.value("display_name").toString());
			tableItem->setTextAlignment(Qt::AlignHCenter);
			tableItem->setBackgroundColor(rowColor);
			ui.tableWidget->setItem(row, 3, tableItem);
		}
	}

}

void ESCurrentBills::slotProceed(QString billId)
{
	ESAddBill* addBill = new ESAddBill(this);
	ES::MainWindowHolder::instance()->getMainWindow()->setCentralWidget(addBill);
	addBill->proceedPendingBill(billId);
	addBill->show();
}

void ESCurrentBills::slotVoidBill(QString billId)
{
	if (ES::Utility::verifyUsingMessageBox(this, "ProgexPOS", "Do you really want to cancel this?"))
	{
		QString billQryStr = "SELECT * FROM bill WHERE bill_id  = " + billId;
		QSqlQuery BillQry(billQryStr);
		if (BillQry.next())
		{
			QString queryUpdateStr("UPDATE bill set status = 3 WHERE bill_id = " + billId);
			QSqlQuery query(queryUpdateStr);

			QString saleQryStr("SELECT * FROM sale WHERE deleted = 0 AND bill_id = " + billId);
			QSqlQuery querySale(saleQryStr);
			while (querySale.next())
			{
				QString stockId = querySale.value("stock_id").toString();
				double qty = querySale.value("quantity").toDouble();


				QString stockQryStr("SELECT * FROM stock WHERE stock_id = " + stockId);
				QSqlQuery queryStock(stockQryStr);
				if (queryStock.next())
				{
					double currentQty = queryStock.value("qty").toDouble();
					double newQty = currentQty + qty;

					QSqlQuery updateStock("UPDATE stock set qty = " + QString::number(newQty) + " WHERE stock_id = " + stockId);
				}
			}
		}
		slotSearch();
	}
	
}
