#include "esloginwidget.h"

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
	
}