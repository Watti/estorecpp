#ifndef SALELINEEDIT_H
#define SALELINEEDIT_H

#include <QLineEdit>

namespace ES
{
	class SaleLineEdit : public QLineEdit
	{
		Q_OBJECT
	public:
		SaleLineEdit(QString saleId);

		~SaleLineEdit();

	signals:
		void notifyQuantityUpdate(QString);

	public slots:
		void slotQuantityUpdate();

	private:
		QString m_saleId;
	};
}

#endif