#ifndef ES_MAINWINDOW_HOLDER_H
#define ES_MAINWINDOW_HOLDER_H

#include <QtWidgets/QMainWindow>
#include <vector>
#include <map>

class ESMainWindow;
class ESSecondaryDisplay;

namespace ES
{
	
	class MainWindowHolder
	{
	public:
		~MainWindowHolder();

		static MainWindowHolder* instance();

		void setMainWindow(ESMainWindow* mainWindow);
		ESMainWindow* getMainWindow();

		void openSecondaryDisplay();
		void hideSecondaryDisplay();
		ESSecondaryDisplay* getSecondaryDisplay();

	private:
		MainWindowHolder();

		static MainWindowHolder* gObject;

		ESMainWindow* m_mainWindow;
		ESSecondaryDisplay* m_secondaryDisplay;
	};

}

#endif