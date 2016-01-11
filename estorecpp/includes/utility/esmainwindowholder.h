#ifndef ES_MAINWINDOW_HOLDER_H
#define ES_MAINWINDOW_HOLDER_H

#include "esmainwindow.h"
#include <QtWidgets/QMainWindow>
#include <vector>
#include <map>

namespace ES
{

	class MainWindowHolder
	{
	public:
		~MainWindowHolder();

		static MainWindowHolder* instance();

		void setMainWindow(ESMainWindow* mainWindow);

		ESMainWindow* getMainWindow();

	private:
		MainWindowHolder();

		static MainWindowHolder* gObject;

		ESMainWindow* m_mainWindow;
	};

}

#endif