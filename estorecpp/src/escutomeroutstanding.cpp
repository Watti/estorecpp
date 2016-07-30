#include "escutomeroutstanding.h"
#include <QtGui>
#include "QSqlQuery"
#include "QPushButton"
#include "QMessageBox"
#include "eslatepayment.h"
#include "utility\session.h"
#include "QString"
#include "utility\utility.h"
#include "utility\esmainwindowholder.h"
#include "esmainwindow.h"

ESCustomerOutstanding::ESCustomerOutstanding(QWidget *parent /*= 0*/) : QWidget(parent), m_startingLimit(0),
m_pageOffset(15), m_nextCounter(0), m_maxNextCount(0)
{
	ui.setupUi(this);
	m_paymentButtonMapper = new QSignalMapper(this);

	QStringList headerLabels;
	headerLabels.append("CustomerID");
	headerLabels.append("Name");
	headerLabels.append("Phone");
	headerLabels.append("Address");
	headerLabels.append("Comments");
	headerLabels.append("Current Outstanding");
	headerLabels.append("Actions");

	ui.customers->setHorizontalHeaderLabels(headerLabels);
	ui.customers->horizontalHeader()->setStretchLastSection(true);
	ui.customers->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.customers->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.customers->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.customers->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.customers->verticalHeader()->hide();
	ui.customers->hideColumn(0);

	QObject::connect(ui.searchText, SIGNAL(textChanged(QString)), this, SLOT(slotSearchCustomers()));
	QObject::connect(m_paymentButtonMapper, SIGNAL(mapped(QString)), this, SLOT(slotPay(QString)));
	QObject::connect(ui.nextBtn, SIGNAL(clicked()), this, SLOT(slotNext()));
	QObject::connect(ui.prevBtn, SIGNAL(clicked()), this, SLOT(slotPrev()));

	ui.prevBtn->setDisabled(true);
	ui.nextBtn->setDisabled(true);

	slotSearchCustomers();

	ui.searchText->setFocus();
}

ESCustomerOutstanding::~ESCustomerOutstanding()
{

}

void ESCustomerOutstanding::slotSearchCustomers()
{
	while (ui.customers->rowCount() > 0)
	{
		ui.customers->removeRow(0);
	}
	QString searchText = ui.searchText->text();
	QString queryStr = "SELECT * FROM customer WHERE deleted = 0", qRecordCountStr;
	qRecordCountStr = "SELECT COUNT(*) as c FROM customer WHERE deleted = 0";
	if (!searchText.isEmpty())
	{
		queryStr += " AND (name LIKE '%" + searchText + "%' OR address LIKE '%" + searchText + "%' OR comments LIKE '%" + searchText + "%')";
		qRecordCountStr += " AND (name LIKE '%" + searchText + "%' OR address LIKE '%" + searchText + "%' OR comments LIKE '%" + searchText + "%')";
	}
	QSqlQuery queryRecordCount(qRecordCountStr);
	if (queryRecordCount.next())
	{
		m_totalRecords = queryRecordCount.value("c").toInt();
	}
	//pagination start
	queryStr.append(" LIMIT ").append(QString::number(m_startingLimit));
	queryStr.append(" , ").append(QString::number(m_pageOffset));
	//pagination end

	QSqlQuery q;

	int row = 0;
	if (q.exec(queryStr))
	{
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
		while (q.next())
		{
			row = ui.customers->rowCount();
			ui.customers->insertRow(row);

			QString customerId = q.value("customer_id").toString();
			double outstandingAmount = ES::Utility::getTotalCreditOutstanding(customerId);

			QColor rowColor;
			if (outstandingAmount > 0)
			{
				rowColor.setRgb(245, 169, 169);
			}
			else
			{
				rowColor.setRgb(169, 245, 208);
			}

			QTableWidgetItem* item = NULL;
			item = new QTableWidgetItem(customerId);
			item->setBackgroundColor(rowColor);
			ui.customers->setItem(row, 0, item);

			item = new QTableWidgetItem(q.value("name").toString());
			item->setBackgroundColor(rowColor);
			ui.customers->setItem(row, 1, item);

			item = new QTableWidgetItem(q.value("phone").toString());
			item->setBackgroundColor(rowColor);
			ui.customers->setItem(row, 2, item);

			item = new QTableWidgetItem(q.value("address").toString());
			item->setBackgroundColor(rowColor);
			ui.customers->setItem(row, 3, item);

			item = new QTableWidgetItem(q.value("comments").toString());
			item->setBackgroundColor(rowColor);
			ui.customers->setItem(row, 4, item);

			QTableWidgetItem* outstandingItem = new QTableWidgetItem(QString::number(outstandingAmount, 'f', 2));
			outstandingItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
			outstandingItem->setBackgroundColor(rowColor);
			ui.customers->setItem(row, 5, outstandingItem);

			if (outstandingAmount > 0)
			{
				//ui.customers->setItem(row, 6, new QTableWidgetItem("-"));
				QWidget* base = new QWidget(ui.customers);
				QPushButton* paymentBtn = new QPushButton("Settle", base);

				m_paymentButtonMapper->setMapping(paymentBtn, customerId);
				QObject::connect(paymentBtn, SIGNAL(clicked()), m_paymentButtonMapper, SLOT(map()));

				QHBoxLayout *layout = new QHBoxLayout;
				layout->setContentsMargins(0, 0, 0, 0);
				layout->addWidget(paymentBtn);
				layout->insertStretch(2);
				base->setLayout(layout);
				ui.customers->setCellWidget(row, 6, base);
				base->show();
			}
		}
	}
}

float ESCustomerOutstanding::getTotalOutstanding(QString customerId)
{
	QString query;
	query.append("SELECT * FROM customer_outstanding WHERE customer_id = ");
	query.append(customerId);
	query.append(" AND settled = 0");

	QSqlQuery q(query);
	if (q.next())
	{
		return q.value("current_outstanding").toDouble();
	}
	return 0;
}

void ESCustomerOutstanding::slotPay(QString customerId)
{
	// 	QMessageBox mbox;
	// 	mbox.setIcon(QMessageBox::Critical);
	// 	mbox.setText(customerId);
	// 	mbox.exec();

	QSqlQuery q("SELECT * FROM customer_outstanding WHERE customer_id = " + customerId);
	if (q.next())
	{
		QSqlQuery userQ("SELECT name FROM customer WHERE customer_id =" + customerId);
		if (userQ.next())
		{
			ESLatePayment* latePayment = new ESLatePayment(this);
			ES::MainWindowHolder::instance()->getMainWindow()->setCentralWidget(latePayment);
			latePayment->setCustomerId(customerId);
			latePayment->getUI().cashierName->setText(ES::Session::getInstance()->getUser()->getName());
			latePayment->getUI().customerName->setText(userQ.value("name").toString());

			float creditOutstanding = ES::Utility::getTotalCreditOutstanding(customerId);
			latePayment->getUI().currentOutstandingCash->setText(QString::number(creditOutstanding, 'f', 2));
			latePayment->getUI().remainingAmountCash->setText(QString::number(creditOutstanding, 'f', 2));

			latePayment->getUI().currentOutstandingCheque->setText(QString::number(creditOutstanding, 'f', 2));
			latePayment->getUI().remainingAmountCheque->setText(QString::number(creditOutstanding, 'f', 2));

			latePayment->getUI().dueDate->setDate(QDate::currentDate());
			latePayment->show();
		}

	}
}

void ESCustomerOutstanding::slotPrev()
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
	slotSearchCustomers();
}

void ESCustomerOutstanding::slotNext()
{
	if (m_nextCounter < m_maxNextCount)
	{
		m_nextCounter++;
		ui.prevBtn->setEnabled(true);
		m_startingLimit += m_pageOffset;
	}
	slotSearchCustomers();
}


