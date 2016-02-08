#include "esaddbill.h"
#include "esaddbillitem.h"
#include "utility/session.h"
#include <QShortcut>
#include <QDesktopWidget>
#include <QApplication>
#include <QSqlQuery>
#include "QDateTime"
#include "QTimer"

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

	ui.paymentMethodComboBox->addItem("CASH", 1);
	ui.paymentMethodComboBox->addItem("CREDIT", 2);
	ui.paymentMethodComboBox->addItem("CARD", 3);
	ui.paymentMethodComboBox->addItem("CHEQUE", 4);

	ui.billedByLabel->setText(ES::Session::getInstance()->getUser()->getName());
	ui.branchLabel->setText("NUGEGODA");

	QTimer *timer = new QTimer(this);
	timer->start(1000);

	new QShortcut(QKeySequence(Qt::Key_F4), this, SLOT(slotShowAddItem()));
	new QShortcut(QKeySequence(Qt::Key_F3), this, SLOT(slotStartNewBill()));
	connect(timer, SIGNAL(timeout()), this, SLOT(showTime()));
}

ESAddBill::~ESAddBill()
{

}

void ESAddBill::slotShowAddItem()
{
	if (!ES::Session::getInstance()->isBillStarted())
		return;

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

void ESAddBill::slotStartNewBill()
{
	ES::Session::getInstance()->startBill();

	QString insertBillQuery("INSERT INTO Bill (user_id) VALUES(");
	QString userID;
	userID.setNum(ES::Session::getInstance()->getUser()->getId());
	insertBillQuery.append(userID);
	insertBillQuery.append(")");

	QSqlQuery insertNewBill(insertBillQuery);
	int id = (insertNewBill.lastInsertId()).value<int>();

	QString billID;
	billID.setNum(id);
	ui.billIdLabel->setText(billID);
	ui.billedByLabel->setText(ES::Session::getInstance()->getUser()->getName());
	ui.branchLabel->setText("NUGEGODA");

// 	QDateTime dateTime = QDateTime::currentDateTime();
// 	ui.dateLabel->setText(dateTime.toString("yyyy/MM/dd  HH:mm:ss"));

}

void ESAddBill::showTime()
{
	ui.dateLabel->setText(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
}

Ui::AddBillWidget& ESAddBill::getUI()
{
	return ui;
}
