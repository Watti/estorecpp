#ifndef SALELINEEDIT_H
#define SALELINEEDIT_H

#include <QLineEdit>

namespace ES
{
	class SaleLineEdit : public QLineEdit
	{
		Q_OBJECT
	public:
		SaleLineEdit(QString saleId, int row);

		~SaleLineEdit();

	signals:
		void notifyQuantityUpdate(QString, int);

	public slots:
		void slotQuantityUpdate();

	private:
		QString m_saleId;
		int m_row;
	};
}

#endif