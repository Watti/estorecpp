#include "escurrentbills.h"
#include "utility/esmainwindowholder.h"
#include "utility/esdbconnection.h"
#include <QMessageBox>
#include "QPushButton"
#include "esaddbill.h"

ESCurrentBills::ESCurrentBills(QWidget *parent)
: QWidget(parent)
{
	ui.setupUi(this);
	m_proceedButtonSignalMapper = new QSignalMapper(this);
	QObject::connect(m_proceedButtonSignalMapper, SIGNAL(mapped(QString)), this, SLOT(slotProceed(QString)));

	ui.startDate->setDisplayFormat("yyyy-MM-dd");
	ui.endDate->setDisplayFormat("yyyy-MM-dd");

	QStringList headerLabels;
	headerLabels.append("Bill ID");
	headerLabels.append("Date");
	headerLabels.append("Payment Method");
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
	QSqlQuery allBillQuery("SELECT * FROM bill WHERE deleted = 0");
	while (allBillQuery.next())
	{		
		if (selectedUser > 0)
		{
			if (selectedUser != allBillQuery.value(2).toInt())
			{
				continue;
			}
		}
		QDateTime billedDate = QDateTime::fromString(allBillQuery.value(1).toString(), Qt::ISODate);
		if (billedDate < startDate || billedDate > endDate)
		{
			continue;
		}
		int statusId = allBillQuery.value(5).toInt();
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
		QString billId = allBillQuery.value(0).toString();
		switch (statusId)
		{
		case 1:
		{
			rowColor.setRgb(51, 254, 84);
			tableItem = new QTableWidgetItem("COMMITTED");
			tableItem->setBackgroundColor(rowColor);
			ui.tableWidget->setItem(row, 5, tableItem);
		}
			break;
		case 2:
		{
			rowColor.setRgb(255, 153, 52);
			tableItem = new QTableWidgetItem("PENDING");
			tableItem->setBackgroundColor(rowColor);
			ui.tableWidget->setItem(row, 5, tableItem);

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
			ui.tableWidget->setCellWidget(row, 6, base);
			base->show();
		}
			break;
		case 3:
		{
			rowColor.setRgb(246, 65, 65);
			tableItem = new QTableWidgetItem("CANCELED");
			tableItem->setBackgroundColor(rowColor);
			ui.tableWidget->setItem(row, 5, tableItem);
		}
			break;
		default:
		{
			rowColor.setRgb(255, 255, 255);
			tableItem = new QTableWidgetItem("UNSPECIFIED");
			tableItem->setBackgroundColor(rowColor);
			ui.tableWidget->setItem(row, 5, tableItem);
		}
			
			break;
		}

		tableItem = new QTableWidgetItem(billId);
		tableItem->setBackgroundColor(rowColor);
		ui.tableWidget->setItem(row, 0, tableItem);
		QDateTime datetime = QDateTime::fromString(allBillQuery.value(1).toString(), Qt::ISODate);
		tableItem = new QTableWidgetItem(datetime.toString(Qt::SystemLocaleShortDate));
		tableItem->setBackgroundColor(rowColor);
		ui.tableWidget->setItem(row, 1, tableItem);

		QString pmQueryStr("SELECT * FROM payment WHERE type_id=");
		pmQueryStr.append(allBillQuery.value(4).toString());
		QSqlQuery pmQuery(pmQueryStr);
		if (pmQuery.first())
		{
			tableItem = new QTableWidgetItem(pmQuery.value(1).toString());
			tableItem->setBackgroundColor(rowColor);
			ui.tableWidget->setItem(row, 2, tableItem);
		}
		tableItem = new QTableWidgetItem(QString::number(allBillQuery.value(3).toFloat(), 'f', 2));
		tableItem->setBackgroundColor(rowColor);
		ui.tableWidget->setItem(row, 3, tableItem);

		QString userQueryStr("SELECT * FROM user WHERE user_id=");
		userQueryStr.append(allBillQuery.value(2).toString());
		QSqlQuery userQuery(userQueryStr);
		if (userQuery.first())
		{
			tableItem = new QTableWidgetItem(userQuery.value(1).toString());
			tableItem->setBackgroundColor(rowColor);
			ui.tableWidget->setItem(row, 4, tableItem);
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
