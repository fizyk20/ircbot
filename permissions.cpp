/*
 * permissions.cpp
 *
 *  Created on: 22 gru 2013
 *      Author: bartek
 */

#include "permissions.h"

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

	core -> registerCommand("perm", this);
	registerCommand("perm", false);
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

	int i;
	for(i=0; i < main -> subcommands.size(); i++)
		write(fout, main -> subcommands[i]);

	file.close();
}

void CPermissions::write(QTextStream& fout, Command* command)
{
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

	new_c -> exceptions = fields[2].split("|");

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
	}
}

bool CPermissions::checkCommand(Command* command, QString nick)
{
	bool result = command->default_allow;
	int i;
	for(i = 0; i < command->exceptions.size(); i++)
		if(nick == command->exceptions[i]) return !result;
	return result;
}

bool CPermissions::checkPrivilege(QString nick, QString command, QStringList params)
{
	Command* current = main;

	while(checkCommand(current, nick))
	{
		current = (*current)[params.takeFirst()];
		if(!current) return true;
	}

	return false;
}

void CPermissions::executeCommand(QString command, QStringList params, QString addr, QString sender)
{

}
