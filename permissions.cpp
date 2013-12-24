/*
 * permissions.cpp
 *
 *  Created on: 22 gru 2013
 *      Author: bartek
 */

#include "permissions.h"
#include "CUsers.h"

PermGroup CPermissions::invalidGroup = { "<invalid>", QStringList() };

Command* Command::operator[](QString command)
{
	for(int i = 0; i < subcommands.size(); i++)
		if(subcommands[i]->command == command)
			return subcommands[i];
	return NULL;
}

/**********************************************************************/

CPermissions::CPermissions(CBotCore* c, CBotSettings* s)
	: CBotPlugin(c, s)
{
	load();

	core -> registerPluginId(this, "permissions");
	core -> registerCommand("perm", this);
	registerCommand("perm", true);
	registerCommand("perm:show", true);
	registerCommand("perm:default", false);
	registerCommand("perm:add_except", false);
	registerCommand("perm:del_except", false);
}

CPermissions::~CPermissions()
{
	save();
}

void CPermissions::save()
{
	QFile file("permissions.dat");
	if(!file.open(QIODevice::ReadWrite | QIODevice::Text))
		return;

	QTextStream fout(&file);

	write(fout, main);

	file.close();

	QFile file2("perm_groups.dat");
	if(!file.open(QIODevice::ReadWrite | QIODevice::Text))
		return;

	QTextStream fout2(&file2);

	for(int i = 0; i < groups.size(); i++)
		fout2 << groups[i].name << "\t" << groups[i].members.join(",") << endl;

	file2.close();
}

void CPermissions::write(QTextStream& fout, Command* command)
{
	if(command -> full_path != "")
		fout << command->full_path << "\t" << (command->default_allow ? "true" : "false") << "\t" << command->exceptions.join("|") << endl;
	int i;
	for(i=0; i < command->subcommands.size(); i++)
		write(fout, command->subcommands[i]);
}

void CPermissions::load()
{
	main = new Command;
	main -> full_path = "";
	main -> command = "";
	main -> default_allow = true;

	Command* parent = main;

	QFile file("permissions.dat");
	if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return;

	QTextStream fin(&file);

	while(!fin.atEnd())
		parent = read(fin, parent);

	file.close();

	QFile file2("perm_groups.dat");
	if(!file2.open(QIODevice::ReadOnly | QIODevice::Text))
		return;

	QTextStream fin2(&file2);

	while(!fin2.atEnd())
	{
		QString line = fin2.readLine();
		QStringList fields = line.split("\t");
		PermGroup g;
		g.name = fields[0];
		g.members = fields[1].split(",", QString::SkipEmptyParts);
		groups.push_back(g);
	}

	file2.close();
}

Command* CPermissions::read(QTextStream& fin, Command* parent)
{
	QString parent_path = (parent == NULL) ? "" : parent->full_path;
	QString line = fin.readLine();
	QStringList fields = line.split("\t");

	Command* new_c = new Command;

	new_c -> full_path = fields[0];

	// command and parent
	int last_colon = fields[0].lastIndexOf(':');
	if(last_colon < 0)
		parent = main;
	else
	{
		QString parent_path = fields[0].left(last_colon);
		if(parent_path != parent->full_path) parent = (*this)[parent_path];
	}
	new_c -> command = fields[0].mid(last_colon+1);
	new_c -> parent = parent;

	new_c -> default_allow = (fields[1] == "true");

	new_c -> exceptions = fields[2].split("|", QString::SkipEmptyParts);

	// save in tree
	parent -> subcommands.push_back(new_c);

	return new_c;
}

Command* CPermissions::operator[](QString path)
{
	QStringList tree_path = path.split(":");
	if(tree_path.size() == 0) return NULL;

	Command* current = main;

	while(tree_path.size() > 0)
	{
		QString command = tree_path.takeFirst();
		current = (*current)[command];
		if(!current) return NULL;
	}

	return current;
}

PermGroup& CPermissions::group(QString name)
{
	int i;
	for(i = 0; i < groups.size(); i++)
		if(groups[i].name == name) return groups[i];
	return invalidGroup;
}

QStringList CPermissions::userGroups(QString name)
{
	int i;
	QStringList result;
	for(i = 0; i < groups.size(); i++)
		if(groups[i].members.contains(name)) result << "group:" + groups[i].name;

	return result;
}

void CPermissions::registerCommand(QString path, bool default_allow)
{
	QStringList l_path = path.split(":");
	if(l_path.size() < 1) return;
	Command* current = main;
	while(l_path.size() > 1)
	{
		if(!current) return;
		current = (*current)[l_path.takeFirst()];
	}
	Command* new_c = (*current)[l_path[0]];
	if(!new_c)
	{
		new_c = new Command;
		new_c -> command = l_path[0];
		new_c -> full_path = path;
		new_c -> default_allow = default_allow;
		current->subcommands.push_back(new_c);
		save();
	}
}

bool CPermissions::checkCommand(Command* command, QString nick)
{
	QStringList names, groups;
	int i;

	names << "nick:" + nick;
	CUsers* users = (CUsers*) core -> getPlugin("users");
	QString account = users -> getAccount(nick);
	if(account != "") names << "account:" + account;

	for(i = 0; i < names.size(); i++)
		groups += userGroups(names[i]);

	names += groups;

	bool result = command->default_allow;

	for(i = 0; i < names.size(); i++)
		if(command->exceptions.contains(names[i])) return !result;
	return result;
}

bool CPermissions::checkPrivilege(QString nick, QString command, QStringList params)
{
	Command* current = main;

	QStringList commands;
	commands << command;
	commands.append(params);

	while(checkCommand(current, nick))
	{
		if(commands.size() == 0) return true;
		current = (*current)[commands.takeFirst()];
		if(!current) return true;
	}

	return false;
}

void CPermissions::executeCommand(QString, QStringList params, QString addr, QString)
{
	if(params.size() == 0)
	{
		core -> sendMsg(addr, "Za mało parametrów!");
		return;
	}

	if(params[0] == "default")
	{
		if(params.size() < 3)
		{
			core -> sendMsg(addr, "Wymagane parametry: komenda użytkownik");
			return;
		}

		Command* c = (*this)[params[1]];
		if(!c)
		{
			core -> sendMsg(addr, "Niepoprawna komenda: " + params[1]);
			return;
		}

		c -> default_allow = (params[2] == "true" || params[2] == "allow");
		core -> sendMsg(addr, "Zmieniono domyślne uprawnienie.");

		return;
	}

	if(params[0] == "add_except")
	{
		if(params.size() < 3)
		{
			core -> sendMsg(addr, "Wymagane parametry: komenda użytkownik");
			return;
		}

		Command* c = (*this)[params[1]];
		if(!c)
		{
			core -> sendMsg(addr, "Niepoprawna komenda: " + params[1]);
			return;
		}

		c -> exceptions.push_back(params[2]);
		core -> sendMsg(addr, "Dodano wyjątek.");

		return;
	}

	if(params[0] == "del_except")
	{
		if(params.size() < 3)
		{
			core -> sendMsg(addr, "Wymagane parametry: komenda użytkownik");
			return;
		}

		Command* c = (*this)[params[1]];
		if(!c)
		{
			core -> sendMsg(addr, "Niepoprawna komenda: " + params[1]);
			return;
		}

		c -> exceptions.removeOne(params[2]);
		core -> sendMsg(addr, "Usunięto wyjątek.");

		return;
	}

	if(params[0] == "show")
	{
		if(params.size() < 2)
		{
			core -> sendMsg(addr, "Wymagane parametry: komenda");
			return;
		}

		Command* c = (*this)[params[1]];
		if(!c)
		{
			core -> sendMsg(addr, "Niepoprawna komenda: " + params[1]);
			return;
		}

		core -> sendMsg(addr, "Domyślnie: " + QString(c->default_allow ? "zezwól" : "zabroń"));
		core -> sendMsg(addr, "Wyjątki: " + c->exceptions.join(", "));

		return;
	}

	if(params[0] == "group")
	{

	}

	save();
}
