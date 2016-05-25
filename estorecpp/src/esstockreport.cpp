#include "esstockreport.h"
#include <QSqlQuery>

ESStockReport::ESStockReport(QWidget *parent /*= 0*/) : QWidget(parent)
{
	ui.setupUi(this);

	QStringList headerLabels;
	headerLabels.append("Item Code");
	headerLabels.append("Current Stock Qty");
	headerLabels.append("Minimum Qty");
	headerLabels.append("Excess");
	headerLabels.append("Re-order");

	ui.tableWidget->setHorizontalHeaderLabels(headerLabels);
	ui.tableWidget->horizontalHeader()->setStretchLastSection(true);
	ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	
	while (ui.tableWidget->rowCount() > 0)
	{
		ui.tableWidget->removeRow(0);
	}

	QSqlQuery q("SELECT stock.qty, stock.min_qty, item.item_code FROM stock JOIN item ON stock.item_id = item.item_id WHERE stock.deleted = 0 ");
	while (q.next())
	{
		int row = ui.tableWidget->rowCount();
		ui.tableWidget->insertRow(row);

		QColor red(245, 169, 169);
		double qty = q.value("qty").toDouble();
		double minQty = q.value("min_qty").toDouble();
		double excess = qty - minQty;

		QTableWidgetItem* item = NULL;
		item = new QTableWidgetItem(q.value("item_code").toString());
		if (excess < 0)	item->setBackgroundColor(red);
		ui.tableWidget->setItem(row, 0, item);

		item = new QTableWidgetItem(QString::number(qty, 'f', 2));
		item->setTextAlignment(Qt::AlignRight);
		if (excess < 0)	item->setBackgroundColor(red);
		ui.tableWidget->setItem(row, 1, item);

		item = new QTableWidgetItem(QString::number(minQty, 'f', 2));
		item->setTextAlignment(Qt::AlignRight);
		if (excess < 0)	item->setBackgroundColor(red);
		ui.tableWidget->setItem(row, 2, item);
				
		QString s, reorder = "No";

		if (excess > 0)
		{
			s = QString::number(excess, 'f', 2);
			s.prepend("(+) ");
		}
		else if (excess < 0)
		{
			s = QString::number(-excess, 'f', 2);
			s.prepend("(-) ");
			reorder = "Yes";
		}
		else
		{
			s = QString::number(excess, 'f', 2);
		}
		
		item = new QTableWidgetItem(s);
		item->setTextAlignment(Qt::AlignRight);
		if (excess < 0)	item->setBackgroundColor(red);
		ui.tableWidget->setItem(row, 3, item);

		item = new QTableWidgetItem(reorder);
		item->setTextAlignment(Qt::AlignCenter);
		if (excess < 0)	item->setBackgroundColor(red);
		ui.tableWidget->setItem(row, 4, item);
	}
}

ESStockReport::~ESStockReport()
{

}

void ESStockReport::slotGenerate()
{

}
