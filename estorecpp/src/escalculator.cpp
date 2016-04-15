#include "escalculator.h"

ESCalculator::ESCalculator(QWidget *parent /*= 0*/)
: QWidget(parent, Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint)
{
	ui.setupUi(this);

	m_result = 0.0;
}

ESCalculator::~ESCalculator()
{

}

void ESCalculator::slotCalculate()
{

}

void ESCalculator::keyPressEvent(QKeyEvent * event)
{
	switch (event->key())
	{
	case Qt::Key_Escape:
		hide();
	case Qt::Key_C:
		resetCalc();
	default:
		QWidget::keyPressEvent(event);
	}
}

void ESCalculator::resetCalc()
{
	m_result = 0.0;
}
