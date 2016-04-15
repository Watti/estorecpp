#ifndef escalculator_h__
#define escalculator_h__

#include "ui_calculatorwidget.h"
#include <QtGui>

class ESCalculator : public QWidget
{
	Q_OBJECT

public:
	ESCalculator(QWidget *parent = 0);
	~ESCalculator();

public slots:
	void slotCalculate();

protected:
	void keyPressEvent(QKeyEvent * event);

private:
	Ui::CalculatorWidget ui;
	double m_result;

	void resetCalc();

};

#endif // escalculator_h__
