#include "esreturnitems.h"

ESReturnItems::ESReturnItems(QWidget *parent /*= 0*/) : QWidget(parent)
{
	ui.setupUi(this);
}

ESReturnItems::~ESReturnItems()
{

}

Ui::ReturnItems& ESReturnItems::getUI()
{
	return ui;
}
