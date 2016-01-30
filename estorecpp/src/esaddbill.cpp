#include "esaddbill.h"
#include "esaddbillitem.h"
#include <QShortcut>
#include <QDesktopWidget>
#include <QApplication>

ESAddBill::ESAddBill(QWidget *parent)
:QWidget(parent)
{
	ui.setupUi(this);

	QStringList headerLabels;
	headerLabels.append("Code");
	headerLabels.append("Item");
	headerLabels.append("Price");
	headerLabels.append("Qty");
	headerLabels.append("Discount");
	headerLabels.append("Amount");
	headerLabels.append("Actions");

	ui.tableWidget->setHorizontalHeaderLabels(headerLabels);
	ui.tableWidget->horizontalHeader()->setStretchLastSection(true);
	ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);

	new QShortcut(QKeySequence(Qt::Key_F4), this, SLOT(slotShowAddItem()));
}

ESAddBill::~ESAddBill()
{

}

void ESAddBill::slotShowAddItem()
{
	QRect rec = QApplication::desktop()->screenGeometry();
	int width = rec.width();
	int height = rec.height();

	width -= 200;
	height -= 200;

	ESAddBillItem* addBillItem = new ESAddBillItem(ui.tableWidget, this);
	addBillItem->resize(QSize(width, height));
	addBillItem->setWindowState(Qt::WindowActive);
	addBillItem->setWindowModality(Qt::ApplicationModal);
	addBillItem->setAttribute(Qt::WA_DeleteOnClose);
	addBillItem->setWindowFlags(Qt::CustomizeWindowHint | Qt::Window);
	addBillItem->show();
}
