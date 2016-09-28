#include "esitemwisesales.h"
#include "QSqlQuery"
#include "KDReportsTextElement.h"
#include "utility\session.h"
#include "KDReportsHtmlElement.h"
#include "KDReportsTableElement.h"
#include "KDReportsCell.h"
#include "QPrintPreviewDialog"
#include "QPrinter"
#include "utility\esmainwindowholder.h"
#include "QMainWindow"
#include "QObject"
#include "esmainwindow.h"
#include "QMessageBox"
#include "qnamespace.h"

ItemWiseSales::ItemWiseSales(QWidget *parent /*= 0*/) : QWidget(parent), report(NULL), m_startingLimit(0), m_pageOffset(15), m_nextCounter(0), m_maxNextCount(0)
{
	ui.setupUi(this);
	m_generateReportSignalMapper = new QSignalMapper(this);
	ui.fromDate->setDate(QDate::currentDate());
	ui.toDate->setDate(QDate::currentDate().addDays(1));

	QStringList headerLabels;
	headerLabels.append("Quantity");
	headerLabels.append("Avg. Price");
	headerLabels.append("Line Total");

	QFont font = this->font();
	font.setBold(true);
	font.setPointSize(12);

	ui.tableWidget->setHorizontalHeaderLabels(headerLabels);
	ui.tableWidget->horizontalHeader()->setStretchLastSection(true);
	ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.tableWidget->verticalHeader()->setMinimumWidth(200);

	QObject::connect(ui.generateButton, SIGNAL(clicked()), this, SLOT(slotGenerateReport()));
	QObject::connect(ui.fromDate, SIGNAL(dateChanged(const QDate &)), this, SLOT(slotSearch()));
	QObject::connect(ui.toDate, SIGNAL(dateChanged(const QDate &)), this, SLOT(slotSearch()));
	QObject::connect(ui.nextBtn, SIGNAL(clicked()), this, SLOT(slotNext()));
	QObject::connect(ui.prevBtn, SIGNAL(clicked()), this, SLOT(slotPrev()));

	ui.prevBtn->setDisabled(true);
	ui.nextBtn->setDisabled(true);
	//slotSearch();
}

ItemWiseSales::~ItemWiseSales()
{

}

void ItemWiseSales::slotPrint(QPrinter* printer)
{
	report->print(printer);
	this->close();
}

void ItemWiseSales::slotSearch()
{
	while (ui.tableWidget->rowCount() > 0)
	{
		ui.tableWidget->removeRow(0);
	}

	QString stardDateStr = ui.fromDate->date().toString("yyyy-MM-dd");
	QString endDateStr = ui.toDate->date().toString("yyyy-MM-dd");
	QString itemWiseSalesQStr("SELECT stock_id, SUM(total) AS tot, SUM(quantity) AS qty FROM sale WHERE deleted = 0 AND  DATE(date) BETWEEN '" + stardDateStr + "' AND '" + endDateStr + "' GROUP BY stock_id ORDER BY qty DESC");
	QString qRecordCountStr("SELECT SQL_CALC_FOUND_ROWS stock_id FROM sale WHERE deleted = 0 AND  DATE(date) BETWEEN '" + stardDateStr + "' AND '" + endDateStr + "' GROUP BY stock_id ");
	QSqlQuery queryRecordCount(qRecordCountStr);
	queryRecordCount.exec("SELECT FOUND_ROWS() as rowCount");
	if (queryRecordCount.next())
	{
		m_totalRecords = queryRecordCount.value("rowCount").toInt();
	}
	//pagination start
	itemWiseSalesQStr.append(" LIMIT ").append(QString::number(m_startingLimit));
	itemWiseSalesQStr.append(" , ").append(QString::number(m_pageOffset));
	//pagination end
	QSqlQuery qItemWiseSales;
	if (qItemWiseSales.exec(itemWiseSalesQStr))
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

		while (qItemWiseSales.next())
		{
			//stock_id, SUM(total) AS tot, SUM(quantity) AS qty
			QString stockId = qItemWiseSales.value("stock_id").toString();
			double total = qItemWiseSales.value("tot").toDouble();
			double quantity = qItemWiseSales.value("qty").toDouble();
			
			int row = ui.tableWidget->rowCount();
			ui.tableWidget->insertRow(row);
			QSqlQuery queryStockItem("SELECT * FROM stock JOIN item ON stock.item_id = item.item_id WHERE stock.deleted = 0 AND item.deleted= 0 AND stock.stock_id = " + stockId);
			if (queryStockItem.next())
			{
				QString itemName = queryStockItem.value("item_name").toString();
				ui.tableWidget->setVerticalHeaderItem(row, new QTableWidgetItem(itemName));
				QTableWidgetItem* qtyTableItemWidget = new QTableWidgetItem(QString::number(quantity));
				qtyTableItemWidget->setTextAlignment(Qt::AlignRight);
				ui.tableWidget->setItem(row, 0, qtyTableItemWidget);
				double avgPrice = total / quantity;
				QTableWidgetItem* avgTableItemWidget = new QTableWidgetItem(QString::number(avgPrice, 'f', 2));
				avgTableItemWidget->setTextAlignment(Qt::AlignRight);
				ui.tableWidget->setItem(row, 1, avgTableItemWidget);
				QTableWidgetItem* totalTableItemWidget = new QTableWidgetItem(QString::number(total, 'f', 2));
				totalTableItemWidget->setTextAlignment(Qt::AlignRight);
				ui.tableWidget->setItem(row, 2, totalTableItemWidget);
			}
		}
	}
	else
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Critical);
		mbox.setText(QString("Something went wrong: Cannot acquire stock data"));
		mbox.exec();
	}

}

void ItemWiseSales::printRow(KDReports::TableElement& tableElement, int row, int col, QString elementStr, Qt::AlignmentFlag alignment /*= Qt::AlignLeft*/)
{
	KDReports::Cell& cell = tableElement.cell(row, col);
	KDReports::TextElement te(elementStr);
	te.setPointSize(ES::Session::getInstance()->getBillItemFontSize());
	cell.addElement(te, alignment);
}

Ui::ItemWiseSales& ItemWiseSales::getUI()
{
	return ui;
}

void ItemWiseSales::slotGenerateReport()
{
	
}

void ItemWiseSales::slotPrev()
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

void ItemWiseSales::slotNext()
{
	if (m_nextCounter < m_maxNextCount)
	{
		m_nextCounter++;
		ui.prevBtn->setEnabled(true);
		m_startingLimit += m_pageOffset;
	}
	slotSearch();
}
