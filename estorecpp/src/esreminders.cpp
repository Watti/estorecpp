#include "esreminders.h"
#include <QSqlQuery>
#include "QLabel"
#include "QFormLayout"
#include "QScrollArea"

ESReminders::ESReminders(QWidget *parent /*= 0*/) : QWidget(parent)
{
	ui.setupUi(this);

	while (ui.toolBox->count() > 0)
	{
		ui.toolBox->removeItem(0);
	}

	QObject::connect(ui.toolBox, SIGNAL(currentChanged(int)), this, SLOT(slotPaint(int)));

	QSqlQuery q("SELECT stock.qty, stock.min_qty, item.item_code, item.item_name FROM stock \
				JOIN item ON stock.item_id = item.item_id WHERE \
				stock.deleted = 0 AND stock.min_qty >= stock.qty ");
	while (q.next())
	{
		QPixmap pixmap("icons/warning.png");
		QIcon icon; icon.addPixmap(pixmap);

		QWidget *window = new QWidget(ui.toolBox);
		QPalette p(window->palette());
		p.setBrush(QPalette::Background, Qt::yellow);
		window->setPalette(p);
		window->setAutoFillBackground(true);
		QVBoxLayout* vbox = new QVBoxLayout;
		QLabel* desc = new QLabel(window);
		desc->setText("Following item quantity is not sufficient. Please re-order this item");

		QFormLayout* form = new QFormLayout;
		form->setLabelAlignment(Qt::AlignRight);
		QFont font;
		font.setPointSize(10);
		font.setBold(true);

		QLabel* lb1 = new QLabel("Item Code : ");
		lb1->setFont(font);
		QLabel* lb2 = new QLabel(q.value("item_code").toString());
		lb2->setFont(font);
		form->addRow(lb1, lb2);

		QLabel* lb3 = new QLabel("Item Name : ");
		lb3->setFont(font);
		QLabel* lb4 = new QLabel(q.value("item_name").toString());
		lb4->setFont(font);
		form->addRow(lb3, lb4);

		QLabel* lb5 = new QLabel("Current Qty : ");
		lb5->setFont(font);
		QLabel* lb6 = new QLabel(q.value("qty").toString());
		lb6->setFont(font);
		form->addRow(lb5, lb6);

		QLabel* lb7 = new QLabel("Minimum Qty : ");
		lb7->setFont(font);
		QLabel* lb8 = new QLabel(q.value("min_qty").toString());
		lb8->setFont(font);
		form->addRow(lb7, lb8);

		vbox->addWidget(desc);
		vbox->addLayout(form);
		vbox->addStretch();

		window->setLayout(vbox);
		window->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
		window->setMinimumHeight(400);
		ui.toolBox->addItem(window, icon, "Low Stock Reminder !");
	}

}

ESReminders::~ESReminders()
{

}

void ESReminders::slotShowAddItem()
{

}

void ESReminders::slotPaint(int index)
{
	QWidget* widget = ui.toolBox->widget(index);
	QObject* pr = widget->parent();
	if (dynamic_cast<QScrollArea*>(pr))
	{
		int x = 0;
	}
	widget->setMinimumHeight(400);
}

void ESReminders::setCurrentIndex(int index)
{
	int y = 0;
}
