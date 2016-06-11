#ifndef tabletextwidget_h__
#define tabletextwidget_h__

#include <QLineEdit>
#include <QKeyEvent>
#include <QtGui>
#include <QTableWidget>
#include <QString>

class TableTextWidget : public QLineEdit
{
	Q_OBJECT

public:
	typedef QString(*TextFormatterFunc)(QString);

	TableTextWidget(QTableWidget* table, int row, int col, QWidget* parent = 0);
	~TableTextWidget();
	void setTextFormatterFunc(TextFormatterFunc func);

signals:
	void notifyEnterPressed(QString, int, int);

public slots:
	void update();
	void slotFinish();

protected:
	void keyPressEvent(QKeyEvent* evt);

private:
	int m_row;
	int m_col;
	QTableWidget* m_table;
	TextFormatterFunc m_formatterFunc;
};

#endif // tabletextwidget_h__
