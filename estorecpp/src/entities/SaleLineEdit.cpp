#include "entities/SaleLineEdit.h"

namespace ES
{


	SaleLineEdit::SaleLineEdit(QString saleId, int row) : QLineEdit()
	{
		m_saleId = saleId;
		m_row = row;
	}

	SaleLineEdit::~SaleLineEdit()
	{

	}

	void SaleLineEdit::slotQuantityUpdate()
	{
		emit notifyQuantityUpdate(m_saleId, m_row);
	}

}