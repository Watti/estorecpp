#include "entities/tabletextwidget.h"
#include <QLabel>
#include <QTableWidgetItem>

TableTextWidget::TableTextWidget(QTableWidget* table, int row, int col, QWidget* parent) : 
QLineEdit(parent)
{
	m_row = row;
	m_col = col;
	m_table = table;
	m_formatterFunc = NULL;

	QObject::connect(this, SIGNAL(editingFinished()), this, SLOT(slotFinish()));
}

TableTextWidget::~TableTextWidget()
{

}

void TableTextWidget::update()
{

}

void TableTextWidget::keyPressEvent(QKeyEvent* evt)
{
	switch (evt->key())
	{
	case Qt::Key_Return:
	case Qt::Key_Enter:
	{
		slotFinish();
		break;
	}
	default:
		QLineEdit::keyPressEvent(evt);
	}
}

void TableTextWidget::slotFinish()
{
	QString str = (m_formatterFunc) ? m_formatterFunc(text()) : text();

	QTableWidgetItem* priceItem = new QTableWidgetItem(str);
	priceItem->setTextAlignment(Qt::AlignRight);
	m_table->setCellWidget(m_row, m_col, NULL);
	m_table->setItem(m_row, m_col, priceItem);
	
	emit notifyEnterPressed(text(), m_row, m_col);
}

void TableTextWidget::setTextFormatterFunc(TextFormatterFunc func)
{
	m_formatterFunc = func;
}
