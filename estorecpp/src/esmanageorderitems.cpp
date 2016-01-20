#include "esmanageorderitems.h"
#include "utility/esmainwindowholder.h"
#include "utility/esdbconnection.h"
#include <QMessageBox>
#include <QPushButton>
#include "utility/utility.h"


ESManageOrderItems::ESManageOrderItems(QWidget *parent/* = 0*/)
{
	ui.setupUi(this);

	QObject::connect(ui.usernameSearch, SIGNAL(textChanged(QString)), this, SLOT(slotSearch()));
	QObject::connect(ui.itemCodeSearch, SIGNAL(textChanged(QString)), this, SLOT(slotSearch()));

	QStringList headerLabels;
	headerLabels.append("Item Code");
	headerLabels.append("User");
	headerLabels.append("Date");
	headerLabels.append("Price");
	headerLabels.append("Qty");
	headerLabels.append("Comment");
	headerLabels.append("Actions");

	ui.tableWidget->setHorizontalHeaderLabels(headerLabels);
	ui.tableWidget->horizontalHeader()->setStretchLastSection(true);
	ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.prevBtn->setDisabled(true);
	ui.nextBtn->setDisabled(true);
	
	if (!ES::DbConnection::instance()->open())
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Critical);
		mbox.setText(QString("Cannot connect to the database : ESManageOrderItems::ESManageOrderItems "));
		mbox.exec();
	}
	else
	{
		/*QSqlQuery queryCategory("SELECT * FROM order");
		QString catCode = "select";
		int catId = -1;

		ui.categoryComboBox->addItem(catCode, catId);

		while (queryCategory.next())
		{
			catId = queryCategory.value(0).toInt();
			ui.categoryComboBox->addItem(queryCategory.value(1).toString(), catId);
		}

		slotSearch();*/
	}
}

ESManageOrderItems::~ESManageOrderItems()
{

}

void ESManageOrderItems::slotAddNewOrderItem()
{

}

void ESManageOrderItems::slotSearch()
{

}