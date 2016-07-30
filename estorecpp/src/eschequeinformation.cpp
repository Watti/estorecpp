#include "eschequeinformation.h"
#include "QSqlQuery"
#include "utility\session.h"

ESChequeInformation::ESChequeInformation(QWidget *parent /*= 0*/) : QWidget(parent), m_startingLimit(0), m_pageOffset(15), m_nextCounter(0), m_maxNextCount(0)
{
	ui.setupUi(this);

	ui.startDate->setDate(QDate::currentDate());
	//ui.endDate->setDate(QDate::currentDate().addDays(1));

	m_processedButtonMapper = new QSignalMapper(this);
	QObject::connect(m_processedButtonMapper, SIGNAL(mapped(QString)), this, SLOT(slotSetProcessed(QString)));
	m_revertButtonMapper = new QSignalMapper(this);
	QObject::connect(m_revertButtonMapper, SIGNAL(mapped(QString)), this, SLOT(slotRevert(QString)));
	QObject::connect(ui.startDate, SIGNAL(dateChanged(const QDate &)), this, SLOT(slotSearch()));
	//QObject::connect(ui.endDate, SIGNAL(dateChanged(const QDate &)), this, SLOT(slotSearch()));
	QObject::connect(ui.statusComboBox, SIGNAL(activated(QString)), this, SLOT(slotSearch()));
	QObject::connect(ui.customerSearchBox, SIGNAL(textChanged(QString)), this, SLOT(slotSearch()));
	QObject::connect(ui.nextBtn, SIGNAL(clicked()), this, SLOT(slotNext()));
	QObject::connect(ui.prevBtn, SIGNAL(clicked()), this, SLOT(slotPrev()));

	QStringList headerLabels;
	headerLabels.append("Customer");
	headerLabels.append("Cheque No.");
	headerLabels.append("Bank");
	headerLabels.append("Due Date");
	headerLabels.append("Amount");
	headerLabels.append("Actions");

	ui.tableWidget->setHorizontalHeaderLabels(headerLabels);
	ui.tableWidget->horizontalHeader()->setStretchLastSection(true);
	ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);

	ui.prevBtn->setDisabled(true);
	ui.nextBtn->setDisabled(true);

	ui.statusComboBox->addItem("ALL", 2);
	ui.statusComboBox->addItem("FINALIZED", 1);
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

	float totalAmount = 0;
	QString stardDateStr = ui.startDate->date().toString("yyyy-MM-dd");
	//QString endDateStr = ui.endDate->date().toString("yyyy-MM-dd");
	int selectedStatus = ui.statusComboBox->currentData().toInt();
	QString customerName = ui.customerSearchBox->text();
	QString qRecordCountStr("SELECT COUNT(*) as c FROM cheque_information JOIN customer ON cheque_information.customer_id = customer.customer_id WHERE DATE(due_date) = ' " + stardDateStr + "'");
	QString chequeQueryStr("SELECT * FROM cheque_information JOIN customer ON cheque_information.customer_id = customer.customer_id WHERE DATE(due_date) = ' " + stardDateStr + "'");
	if (selectedStatus != 2)
	{
		chequeQueryStr.append("AND processed = " + QString::number(selectedStatus));
		qRecordCountStr.append("AND processed = " + QString::number(selectedStatus));
	}
	if (!customerName.isEmpty())
	{
		chequeQueryStr.append("AND customer.name LIKE '%" + customerName + "%'");
		qRecordCountStr.append("AND customer.name LIKE '%" + customerName + "%'");
	}
	QColor rowColor;

	QSqlQuery queryRecordCount(qRecordCountStr);
	if (queryRecordCount.next())
	{
		m_totalRecords = queryRecordCount.value("c").toInt();
	}
	//pagination start
	chequeQueryStr.append(" LIMIT ").append(QString::number(m_startingLimit));
	chequeQueryStr.append(" , ").append(QString::number(m_pageOffset));
	//pagination end

	QSqlQuery queryCheque(chequeQueryStr);

	//pagination start
	m_maxNextCount = m_totalRecords / m_pageOffset;
	if (m_maxNextCount > m_nextCounter)
	{
		ui.nextBtn->setEnabled(true);
	}
	int currentlyShowdItemCount = (m_nextCounter + 1)*m_pageOffset;
	if (currentlyShowdItemCount >= m_totalRecords)
	{
		ui.nextBtn->setDisabled(true);
	}
	//pagination end

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
		int row = ui.tableWidget->rowCount();
		ui.tableWidget->insertRow(row);
		item = new QTableWidgetItem(queryCheque.value("name").toString());
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

		QString chequeId = queryCheque.value("cheque_id").toString();
		float chequeAmount = queryCheque.value("amount").toFloat();
		totalAmount += chequeAmount;
		item = new QTableWidgetItem(QString::number(chequeAmount, 'f', 2));
		item->setBackgroundColor(rowColor);
		ui.tableWidget->setItem(row, 4, item);

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
			ui.tableWidget->setCellWidget(row, 5, base);
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
				ui.tableWidget->setCellWidget(row, 5, base);
				base->show();
			}
		}
	}
	ui.totalLbl->setText(QString::number(totalAmount, 'f', 2));
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

void ESChequeInformation::slotPrev()
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

void ESChequeInformation::slotNext()
{
	if (m_nextCounter < m_maxNextCount)
	{
		m_nextCounter++;
		ui.prevBtn->setEnabled(true);
		m_startingLimit += m_pageOffset;
	}
	slotSearch();
}
