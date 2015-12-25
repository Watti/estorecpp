#include "esloginwidget.h"
#include "esdbconnection.h"
#include <QMessageBox>

ESLoginWidget::ESLoginWidget(QWidget* parent) 
	: QWidget(parent)
{
	ui.setupUi(this);

	QObject::connect(ui.loginButton, SIGNAL(clicked()), this, SLOT(slotLogin()));
}

ESLoginWidget::~ESLoginWidget()
{

}

void ESLoginWidget::slotLogin()
{
	bool ok = ES::DbConnection::instance()->open();
	if (!ok)
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Critical);
		mbox.setText(QString("Cannot connect to the database"));
		mbox.exec();
	}
	else
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Information);
		mbox.setText(QString("Successfully connected to the database"));
		mbox.exec();
	}
}