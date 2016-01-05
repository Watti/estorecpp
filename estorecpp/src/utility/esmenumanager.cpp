#include "utility/esmenumanager.h"

namespace ES
{
	MenuManager* MenuManager::gObject = NULL;

	MenuManager::MenuManager()
	{

	}

	MenuManager::~MenuManager()
	{

	}

	MenuManager* MenuManager::instance()
	{
		if (!gObject)
		{
			gObject = new MenuManager;
		}
		return gObject;
	}

	void MenuManager::addMenu(QString name, QMenu* ptr)
	{
		m_menuNames[name] = ptr;
	}

	void MenuManager::addAction(QString name, QAction* ptr)
	{
		m_actionNames[name] = ptr;
	}

	void MenuManager::addMenuActionMapping(QString menu, QString action)
	{
		std::vector<QString>& menuActions = m_menuActionMappings[menu];
		menuActions.push_back(action);
	}

	void MenuManager::reload(QString permissions)
	{
		QStringList allPerms = permissions.split(",");
		QStringListIterator iter(allPerms);

		while (iter.hasNext())
		{
			QString menuPermission = iter.next();
			QStringList actionPermissons = menuPermission.split("/");
			if (!actionPermissons.isEmpty())
			{
				QStringListIterator iter2(actionPermissons);

				while (iter2.hasNext())
				{
					QString s = iter2.next();
					for (std::map<QString, QMenu*>::iterator it = m_menuNames.begin(), eit = m_menuNames.end(); it != eit; ++it)
					{
						if (it->first == s.trimmed())
							(it->second)->menuAction()->setVisible(true);
					}
					for (std::map<QString, QAction*>::iterator it = m_actionNames.begin(), eit = m_actionNames.end(); it != eit; ++it)
					{
						if (it->first == s.trimmed())
							(it->second)->setVisible(true);
					}
				}
			}
		}
	}

	void MenuManager::disableAll()
	{
		for (std::map<QString, QMenu*>::iterator it = m_menuNames.begin(), eit = m_menuNames.end(); it != eit; ++it)
		{
			(it->second)->menuAction()->setVisible(false);
		}	
		for (std::map<QString, QAction*>::iterator it = m_actionNames.begin(), eit = m_actionNames.end(); it != eit; ++it)
		{
			(it->second)->setVisible(false);
		}
	}

}