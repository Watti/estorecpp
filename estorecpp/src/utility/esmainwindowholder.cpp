#include "utility/esmainwindowholder.h"
#include "esmainwindow.h"
#include "essecondarydisplay.h"
#include "QApplication"
#include "QDesktopWidget"

namespace ES
{
	MainWindowHolder* MainWindowHolder::gObject = NULL;

	MainWindowHolder::MainWindowHolder()
	{
		m_mainWindow = NULL;
		m_secondaryDisplay = new ESSecondaryDisplay(0);
	}

	MainWindowHolder::~MainWindowHolder()
	{
		delete m_secondaryDisplay;
		m_secondaryDisplay = NULL;
	}

	MainWindowHolder* MainWindowHolder::instance()
	{
		if (!gObject)
		{
			gObject = new MainWindowHolder;
		}
		return gObject;
	}

	void MainWindowHolder::setMainWindow(ESMainWindow* mainWindow)
	{
		m_mainWindow = mainWindow;
	}

	ESMainWindow* MainWindowHolder::getMainWindow()
	{
		return m_mainWindow;
	}

	void MainWindowHolder::openSecondaryDisplay()
	{		
		QRect rect2 = QApplication::desktop()->screenGeometry(1);
		m_secondaryDisplay->move(QPoint(rect2.x(), rect2.y()));
		//m_secondaryDisplay->resize(rect2.width(), rect2.height());
		m_secondaryDisplay->showMaximized();
	}

	ESSecondaryDisplay* MainWindowHolder::getSecondaryDisplay()
	{
		return m_secondaryDisplay;
	}

	void MainWindowHolder::hideSecondaryDisplay()
	{
		m_secondaryDisplay->hide();
	}

}