#include "includes/esreturnsummary.h"

ESReturnSummary::ESReturnSummary(QWidget *parent /*= 0*/) : QWidget(parent)
{
	ui.setupUi(this);

	m_removeButtonSignalMapper = new QSignalMapper(this);

	QStringList headerLabels;
	headerLabels.append("User");
	headerLabels.append("# of Bills");
	headerLabels.append("Total");
	headerLabels.append("Actions");

	ui.tableWidgetByUser->setHorizontalHeaderLabels(headerLabels);
	ui.tableWidgetByUser->horizontalHeader()->setStretchLastSection(true);
	ui.tableWidgetByUser->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tableWidgetByUser->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableWidgetByUser->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableWidgetByUser->setSelectionMode(QAbstractItemView::SingleSelection);

	ui.fromDate->setDate(QDate::currentDate());
	ui.toDate->setDate(QDate::currentDate().addDays(1));

	QObject::connect(ui.fromDate, SIGNAL(dateChanged(const QDate &)), this, SLOT(slotDateChanged()));
	QObject::connect(ui.toDate, SIGNAL(dateChanged(const QDate &)), this, SLOT(slotDateChanged()));

	slotSearch();
}

ESReturnSummary::~ESReturnSummary()
{

}

void ESReturnSummary::slotSearch()
{
	while (ui.tableWidgetByUser->rowCount() > 0)
	{
		ui.tableWidgetByUser->removeRow(0);
	}

	QDateTime startDate = QDateTime::fromString(ui.fromDate->text(), Qt::ISODate);
	QDateTime endDate = QDateTime::fromString(ui.toDate->text(), Qt::ISODate);
	QString stardDateStr = startDate.date().toString("yyyy-MM-dd");
	QString endDateStr = endDate.date().toString("yyyy-MM-dd");

	QSqlQuery q("SELECT user_id, COUNT(bill_id) AS bills, SUM(return_total) AS total FROM return_item  DATE(date) BETWEEN '" + stardDateStr + "' AND '" + endDateStr + "'" + "GROUP BY(user_id)");
	while (q.next())
	{
		QString userId = q.value("user_id").toString();
		int bills = q.value("bills").toInt();
		double total = q.value("return_total").toDouble();

		int row = ui.tableWidgetByUser->rowCount();
		ui.tableWidgetByUser->insertRow(row);

		QSqlQuery userQuery("SELECT display_name FROM user WHERE user_id = " + userId);
		if (userQuery.next())
		{
			QTableWidgetItem* userIdItem = new QTableWidgetItem(userQuery.value("display_name").toString());
			ui.tableWidgetByUser->setItem(row, 0, userIdItem);
		}

		QTableWidgetItem* billsItem = new QTableWidgetItem(q.value("bills").toString());
		ui.tableWidgetByUser->setItem(row, 1, billsItem);

		double returnTotal = q.value("total").toDouble();
		QTableWidgetItem* totalItem = new QTableWidgetItem(QString::number(returnTotal, 'f', 2));
		totalItem->setTextAlignment(Qt::AlignRight);
		ui.tableWidgetByUser->setItem(row, 2, totalItem);

		QWidget* base = new QWidget(ui.tableWidgetByUser);

		QPushButton* removeBtn = new QPushButton(base);
		removeBtn->setIcon(QIcon("icons/detail.png"));
		removeBtn->setIconSize(QSize(24, 24));
		removeBtn->setMaximumWidth(100);

		m_removeButtonSignalMapper->setMapping(removeBtn, userId);
		QObject::connect(removeBtn, SIGNAL(clicked()), m_removeButtonSignalMapper, SLOT(map()));

		QHBoxLayout *layout = new QHBoxLayout;
		layout->setContentsMargins(0, 0, 0, 0);
		layout->addWidget(removeBtn);
		layout->insertStretch(2);
		base->setLayout(layout);
		ui.tableWidgetByUser->setCellWidget(row, 3, base);
		base->show();
	}
}

void ESReturnSummary::slotDateChanged()
{
	slotSearch();
}
