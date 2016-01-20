#include "esaddorderitem.h"
#include "esmanageorderitems.h"
#include "utility/esmainwindowholder.h"
#include "utility/esdbconnection.h"
#include <QMessageBox>
#include <QPushButton>
#include "utility/session.h"

AddOrderItem::AddOrderItem(QWidget *parent/* = 0*/)
{
	m_update = false;
	ui.setupUi(this);

	QObject::connect(ui.addOrderItemButton, SIGNAL(clicked()), this, SLOT(slotAddOrderItem()));
}

AddOrderItem::~AddOrderItem()
{

}

void AddOrderItem::setUpdate(bool update)
{
	m_update = update;
}

void AddOrderItem::slotAddOrderItem()
{
	QString itemCode = ui.itemCode->text();
	QString itemId = "-1";
	QSqlQuery query1("SELECT * FROM item WHERE item_code = '" + itemCode + "'");
	if (query1.next())
	{
		itemId = query1.value(0).toString();
	}

	int userId = ES::Session::getInstance()->getUser()->getId();
	QString userIdStr;
	userIdStr.setNum(userId);
	QString price = ui.unitPrice->text();
	QString qty = ui.qty->text();
	QString description = ui.description->toPlainText();

	QString q;
	if (m_update)
	{
		
	}
	else
	{
		q = "INSERT INTO stock_order (item_id, user_id, price, quantity, description) VALUES (" +
			itemId + "," + userIdStr + "," + price + "," + qty + ",'" + description + "')";
	}
	QSqlQuery query;
	if (query.exec(q))
	{
		this->close();
		ESManageOrderItems* manageItems = new ESManageOrderItems();
		ES::MainWindowHolder::instance()->getMainWindow()->setCentralWidget(manageItems);
		this->close();
		manageItems->show();
	}
	else
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Critical);
		mbox.setText(QString("Something goes wrong:: Item cannot be saved"));
		mbox.exec();
	}

}