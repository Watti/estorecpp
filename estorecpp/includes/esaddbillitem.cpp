#include "esaddbillitem.h"
#include <QKeyEvent>
#include <QShortcut>
#include <QMessageBox>

class TableWidgetEventFilter : public QObject
{
	QObject* m_target;
public:
	TableWidgetEventFilter(QObject* target = 0) : QObject(), m_target(target)
	{}
	~TableWidgetEventFilter()
	{}

	bool eventFilter(QObject* object, QEvent* event)
	{
		if (event->type() == QEvent::KeyPress)
		{
			QKeyEvent *keyEvent = dynamic_cast<QKeyEvent *>(event);

			return true;
		}

		return QObject::eventFilter(object, event);
	}
};

class LineEditEventFilter : public QObject
{
	QObject* m_target;
public:
	LineEditEventFilter(QObject* target = 0) : QObject(), m_target(target)
	{}
	~LineEditEventFilter()
	{}

	bool eventFilter(QObject* object, QEvent* event)
	{
		if (event->type() == QEvent::KeyPress)
		{
			QKeyEvent *keyEvent = dynamic_cast<QKeyEvent *>(event);

			return true;
		}

		return QObject::eventFilter(object, event);
	}
};

ESAddBillItem::ESAddBillItem(QWidget *parent)
{
	ui.setupUi(this);

	QStringList headerLabels;
	headerLabels.append("Item Code");
	headerLabels.append("Item Name");
	headerLabels.append("Price");
	headerLabels.append("Discount");

	ui.tableWidget->setHorizontalHeaderLabels(headerLabels);
	ui.tableWidget->horizontalHeader()->setStretchLastSection(true);
	ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);

	new QShortcut(QKeySequence(Qt::Key_Escape), this, SLOT(close()));
	ui.itemText->setFocus();
	ui.itemText->setFocusPolicy(Qt::StrongFocus);

	QObject::connect(ui.itemText, SIGNAL(textChanged(QString)), this, SLOT(slotSearch()));
}

ESAddBillItem::~ESAddBillItem()
{

}

void ESAddBillItem::slotShowAddItem()
{

}

void ESAddBillItem::slotSearch()
{
	QString searchText = ui.itemText->text();
	// TODO : populate the tableWidget
}

void ESAddBillItem::keyPressEvent(QKeyEvent * event)
{
	switch (event->key())
	{
	case Qt::Key_Return:
	{
		QList<QTableWidgetItem *> items = ui.tableWidget->selectedItems();
		QString number;
		number.setNum(items.size());
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Critical);
		mbox.setText(number);
		mbox.exec();
	}
		break;
	case Qt::Key_Up:
	case Qt::Key_Down:
	case Qt::Key_Left:
	case Qt::Key_Right:
	{
		ui.tableWidget->setFocus();
	}
		break;
	default:
		ui.itemText->setFocus();
		break;
	}
}
