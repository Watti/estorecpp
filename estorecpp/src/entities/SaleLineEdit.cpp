#include "entities/SaleLineEdit.h"

namespace ES
{


	SaleLineEdit::SaleLineEdit(QString saleId) : QLineEdit()
	{
		m_saleId = saleId;
	}

	SaleLineEdit::~SaleLineEdit()
	{

	}

	void SaleLineEdit::slotQuantityUpdate()
	{
		emit notifyQuantityUpdate(m_saleId);
	}

}