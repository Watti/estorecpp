#include "utility/esmainwindowholder.h"

namespace ES
{
	MainWindowHolder* MainWindowHolder::gObject = NULL;

	MainWindowHolder::MainWindowHolder()
	{
		m_mainWindow = NULL;
	}

	MainWindowHolder::~MainWindowHolder()
	{

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

}