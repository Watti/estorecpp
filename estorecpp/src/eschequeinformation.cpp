#include "eschequeinformation.h"
#include "QSqlQuery"
#include "utility\session.h"

ESChequeInformation::ESChequeInformation(QWidget *parent /*= 0*/) : QWidget(parent)
{
	ui.setupUi(this);

	ui.startDate->setDate(QDate::currentDate());
	ui.endDate->setDate(QDate::currentDate().addDays(1));

	m_processedButtonMapper = new QSignalMapper(this);
	QObject::connect(m_processedButtonMapper, SIGNAL(mapped(QString)), this, SLOT(slotSetProcessed(QString)));
	m_revertButtonMapper = new QSignalMapper(this);
	QObject::connect(m_revertButtonMapper, SIGNAL(mapped(QString)), this, SLOT(slotRevert(QString)));
	QObject::connect(ui.startDate, SIGNAL(dateChanged(const QDate &)), this, SLOT(slotSearch()));
	QObject::connect(ui.endDate, SIGNAL(dateChanged(const QDate &)), this, SLOT(slotSearch()));
	QObject::connect(ui.statusComboBox, SIGNAL(activated(QString)), this, SLOT(slotSearch()));
	QObject::connect(ui.customerSearchBox, SIGNAL(textChanged(QString)), this, SLOT(slotSearch()));

	QStringList headerLabels;
	headerLabels.append("Customer");
	headerLabels.append("Cheque No.");
	headerLabels.append("Bank");
	headerLabels.append("Due Date");
	headerLabels.append("Actions");

	ui.tableWidget->setHorizontalHeaderLabels(headerLabels);
	ui.tableWidget->horizontalHeader()->setStretchLastSection(true);
	ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);

	ui.statusComboBox->addItem("ALL", 2);
	ui.statusComboBox->addItem("PROCESSED", 1);
	ui.statusComboBox->addItem("WAITING", 0);

	slotSearch();
}

ESChequeInformation::~ESChequeInformation()
{

}

void ESChequeInformation::slotSearch()
{
	while (ui.tableWidget->rowCount() > 0)
	{
		ui.tableWidget->removeRow(0);
	}

	QString stardDateStr = ui.startDate->date().toString("yyyy-MM-dd");
	QString endDateStr = ui.endDate->date().toString("yyyy-MM-dd");
	int selectedStatus = ui.statusComboBox->currentData().toInt();
	QString customerName = ui.customerSearchBox->text();
	QString chequeQueryStr("SELECT * FROM cheque_information WHERE DATE(due_date) BETWEEN ' " + stardDateStr + "' AND '" + endDateStr + "'");
	if (selectedStatus != 2)
	{
		chequeQueryStr.append("AND processed = " + QString::number(selectedStatus));
	}
	QColor rowColor;
	QSqlQuery queryCheque(chequeQueryStr);
	while (queryCheque.next())
	{

		if (queryCheque.value("processed").toInt() == 0)
		{
			rowColor.setRgb(245, 169, 169);
		}
		else
		{
			rowColor.setRgb(169, 245, 208);
		}

		QTableWidgetItem* item = NULL;
		QString qCustomerStr("SELECT * FROM customer WHERE customer_id = " + queryCheque.value("customer_id").toString());
		if (!customerName.isEmpty())
		{
			qCustomerStr = "SELECT * FROM customer WHERE name LIKE '%" + customerName + "%'";
		}
		QSqlQuery queryCustomer(qCustomerStr);
		if (queryCustomer.next())
		{
			if (queryCustomer.value("customer_id").toInt() == queryCheque.value("customer_id").toInt())
			{
				int row = ui.tableWidget->rowCount();
				ui.tableWidget->insertRow(row);
				item = new QTableWidgetItem(queryCustomer.value("name").toString());
				item->setBackgroundColor(rowColor);
				ui.tableWidget->setItem(row, 0, item);
				item = new QTableWidgetItem(queryCheque.value("cheque_number").toString());
				item->setBackgroundColor(rowColor);
				ui.tableWidget->setItem(row, 1, item);

				item = new QTableWidgetItem(queryCheque.value("bank").toString());
				item->setBackgroundColor(rowColor);
				ui.tableWidget->setItem(row, 2, item);

				item = new QTableWidgetItem(queryCheque.value("due_date").toString());
				item->setBackgroundColor(rowColor);
				ui.tableWidget->setItem(row, 3, item);

				if (queryCheque.value("processed").toInt() == 0)
				{
					QWidget* base = new QWidget(ui.tableWidget);
					QPushButton* proceedBtn = new QPushButton("Processed", base);
					proceedBtn->setMaximumWidth(100);

					m_processedButtonMapper->setMapping(proceedBtn, queryCheque.value("cheque_id").toString());
					QObject::connect(proceedBtn, SIGNAL(clicked()), m_processedButtonMapper, SLOT(map()));

					QHBoxLayout *layout = new QHBoxLayout;
					layout->setContentsMargins(0, 0, 0, 0);
					layout->addWidget(proceedBtn);
					layout->insertStretch(2);
					base->setLayout(layout);
					ui.tableWidget->setCellWidget(row, 4, base);
					base->show();
				}
				else
				{
					if (ES::Session::getInstance()->getUser()->getType() == ES::User::SENIOR_MANAGER ||
						ES::Session::getInstance()->getUser()->getType() == ES::User::MANAGER ||
						ES::Session::getInstance()->getUser()->getType() == ES::User::DEV)
					{
						QWidget* base = new QWidget(ui.tableWidget);
						QPushButton* proceedBtn = new QPushButton("Revert", base);
						proceedBtn->setMaximumWidth(100);

						m_revertButtonMapper->setMapping(proceedBtn, queryCheque.value("cheque_id").toString());
						QObject::connect(proceedBtn, SIGNAL(clicked()), m_revertButtonMapper, SLOT(map()));

						QHBoxLayout *layout = new QHBoxLayout;
						layout->setContentsMargins(0, 0, 0, 0);
						layout->addWidget(proceedBtn);
						layout->insertStretch(2);
						base->setLayout(layout);
						ui.tableWidget->setCellWidget(row, 4, base);
						base->show();
					}
				}
			}
		}
	}
}

void ESChequeInformation::slotSetProcessed(QString rowId)
{
	QString queryUpdateStr("UPDATE cheque_information SET processed = 1 WHERE cheque_id = " + rowId);
	QSqlQuery query(queryUpdateStr);

	slotSearch();
}

void ESChequeInformation::slotRevert(QString rowId)
{
	QString queryUpdateStr("UPDATE cheque_information SET processed = 0 WHERE cheque_id = " + rowId);
	QSqlQuery query(queryUpdateStr);

	slotSearch();
}
