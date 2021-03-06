/*
 * permissions.h
 *
 *  Created on: 22 gru 2013
 *      Author: bartek
 */

#ifndef PERMISSIONS_H_
#define PERMISSIONS_H_

#include "CBotCore.h"
#include <QVector>

struct PermGroup
{
	QString name;
	QStringList members;
};

class Command
{
public:
	QString command, full_path;
	Command* parent;
	QVector<Command*> subcommands;

	bool default_allow;
	QStringList exceptions;

	Command* operator[](QString subcommand);
};

class CPermissions : public CBotPlugin
{
	static PermGroup invalidGroup;

	Command* main;
	QVector<PermGroup> groups;

	void load();
	void save();
	void write(QTextStream&, Command*);
	Command* read(QTextStream&, Command*);

	PermGroup& group(QString);
	QStringList userGroups(QString);

	bool checkCommand(Command*, QString);
public:
	CPermissions(CBotCore*, CBotSettings*);
	~CPermissions();

	Command* operator[](QString);

	void registerCommand(QString path, bool default_allow = true);
	bool checkPrivilege(QString nick, QString command, QStringList params);

	void executeCommand(QString, QStringList, QString, QString);
};

#endif /* PERMISSIONS_H_ */
