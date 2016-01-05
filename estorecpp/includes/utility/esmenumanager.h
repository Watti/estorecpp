#ifndef ES_MENUMANAGER_H
#define ES_MENUMANAGER_H

#include <QtWidgets/QMenu>
#include <QtWidgets/QAction>
#include <vector>
#include <map>

namespace ES
{

	class MenuManager
	{
	public:
		~MenuManager();

		static MenuManager* instance();

		void reload(QString permissions);

	private:
		friend class ESMainWindow;

		void addMenu(QString name, QMenu* ptr);

		void addAction(QString name, QAction* ptr);

		void addMenuActionMapping(QString menu, QString action);

		void disableAll();

		MenuManager();

		static MenuManager* gObject;

		std::map<QString, QMenu*> m_menuNames;
		std::map<QString, QAction*> m_actionNames;
		std::map<QString, std::vector<QString> > m_menuActionMappings;
	};

}

#endif