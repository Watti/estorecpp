#include "escurrentbills.h"
#include "utility/esmainwindowholder.h"
#include "utility/esdbconnection.h"
#include <QMessageBox>
#include "QPushButton"
#include "QBoxLayout"

ESCurrentBills::ESCurrentBills(QWidget *parent)
: QWidget(parent)
{
	ui.setupUi(this);

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

	if (!ES::DbConnection::instance()->open())
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Critical);
		mbox.setText(QString("Cannot connect to the database : ESCurrentBills::ESCurrentBills "));
		mbox.exec();
	}
	else
	{
		
		ui.pmComboBox->addItem("Select", BillStatus::UNDEFINED_BILL);
		ui.pmComboBox->addItem("PENDING", BillStatus::PENDING_BILL);
		ui.pmComboBox->addItem("SUSPENDED", BillStatus::SUSPENDED_BILL);
		ui.pmComboBox->addItem("FINISHED", BillStatus::FINISHED_BILL);
		
	}

	m_proceedButtonSignalMapper = new QSignalMapper(this);
	QObject::connect(m_proceedButtonSignalMapper, SIGNAL(mapped(QString)), this, SLOT(slotProceed(QString)));
	QObject::connect(ui.pmComboBox, SIGNAL(activated(QString)), this, SLOT(slotTypeSelected()));
}

ESCurrentBills::~ESCurrentBills()
{

}

void ESCurrentBills::slotTypeSelected()
{
	while (ui.tableWidget->rowCount() > 0)
	{
		ui.tableWidget->removeRow(0);
	}
	int i = 0;
	int row = 0;

// 	headerLabels.append("Bill ID");
// 	headerLabels.append("Date");
// 	headerLabels.append("Payment Method");
// 	headerLabels.append("Amount");
// 	headerLabels.append("User");
// 	headerLabels.append("Status");
// 	headerLabels.append("Actions");

	int status = ui.pmComboBox->currentData().toInt();
	if (status != BillStatus::UNDEFINED_BILL)
	{
		QString s = QString::number(status);
		QString quearyStr("SELECT * FROM bill WHERE deleted = 0 AND status = " + s);
		QSqlQuery query(quearyStr);
		while (query.next())
		{
			QSqlQuery paymentQuery("SELECT type FROM payment WHERE type_id = " + query.value("payment_method").toString());
			QString paymentType(""), user("");
			while (paymentQuery.next())
			{
				paymentType = paymentQuery.value("type").toString();
			}
			QSqlQuery userQuery("SELECT * FROM user WHERE user_id = " + query.value("user_id").toString());
			while (userQuery.next())
			{
				user = userQuery.value("display_name").toString();
			}
			int statusInt = query.value("status").toInt();
			QString statusStr = getStatusInString(static_cast<BillStatus>(statusInt));
			row = ui.tableWidget->rowCount();
			ui.tableWidget->insertRow(row);
			ui.tableWidget->setItem(row, 0, new QTableWidgetItem(query.value("bill_id").toString()));
			ui.tableWidget->setItem(row, 1, new QTableWidgetItem(query.value("date").toString()));
			ui.tableWidget->setItem(row, 2, new QTableWidgetItem(paymentType));
			ui.tableWidget->setItem(row, 3, new QTableWidgetItem(query.value("amount").toString()));
			ui.tableWidget->setItem(row, 4, new QTableWidgetItem(user));
			ui.tableWidget->setItem(row, 5, new QTableWidgetItem(statusStr));

			if (statusInt == BillStatus::PENDING_BILL || statusInt == BillStatus::SUSPENDED_BILL)
			{
				QWidget* base = new QWidget(ui.tableWidget);
				QPushButton* proceedBtn = new QPushButton("Proceed", base);
				proceedBtn->setMaximumWidth(100);

				QObject::connect(proceedBtn, SIGNAL(clicked()), m_proceedButtonSignalMapper, SLOT(map()));
				m_proceedButtonSignalMapper->setMapping(proceedBtn, query.value("bill_id").toString());

				QHBoxLayout *layout = new QHBoxLayout;
				layout->setContentsMargins(0, 0, 0, 0);
				layout->addWidget(proceedBtn);
				layout->insertStretch(2);
				base->setLayout(layout);
				ui.tableWidget->setCellWidget(row, 6, base);
				base->show();
			}
			else
			{
				ui.tableWidget->setItem(row, 6, new QTableWidgetItem(""));
			}
		}
	}


	//for (auto col : rowContent)
	{
	//	ui.tableWidget->setItem(row, i++, new QTableWidgetItem(col));
	}
}

QString ESCurrentBills::getStatusInString(BillStatus status) const
{
	switch (status)
	{
	case FINISHED_BILL : 
		return "FINISHED";
	case PENDING_BILL :
		return "PENDING";
	case SUSPENDED_BILL :
		return "SUSPENDED";
	default:
		return "UNDEFINED";
	}
	return "UNDEFINED";
}

void ESCurrentBills::slotProceed(QString billId)
{

}
