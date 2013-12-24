/*
 * leavemsg.cpp
 *
 *  Created on: 24 gru 2013
 *      Author: bartek
 */

#include "leavemsg.h"
#include "permissions.h"
#include "CUsers.h"

CLeaveMsg::CLeaveMsg(CBotCore* c, CBotSettings* s)
	: CBotPlugin(c, s)
{
	core -> registerCommand("leavemsg", this);

	CPermissions* p = (CPermissions*) core -> getPlugin("permissions");
	p -> registerCommand("leavemsg", true);

	CUsers* u = (CUsers*) core -> getPlugin("users");
	connect(u, SIGNAL(userAuthed(QString,QString)), this, SLOT(userAuthed(QString,QString)));

	load();
}

CLeaveMsg::~CLeaveMsg()
{
	save();
}

void CLeaveMsg::load()
{
	QFile file("left_msgs.dat");
	if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return;

	QTextStream fin(&file);

	while(!fin.atEnd())
	{
		QString line = fin.readLine();
		QStringList fields = line.split("\t");
		if(fields.size() != 2) continue;
		msgs[fields[0]] = fields[1];
	}

	file.close();
}

void CLeaveMsg::save()
{
	QFile file("left_msgs.dat");
	if(!file.open(QIODevice::ReadWrite | QIODevice::Text))
		return;

	QTextStream fout(&file);

	QMap<QString, QString>::iterator it;

	for(it = msgs.begin(); it != msgs.end(); it++)
		fout << it.key() << "\t" << it.value() << endl;

	file.close();
}

void CLeaveMsg::executeCommand(QString, QStringList params, QString addr, QString sender)
{
	if(params.size() < 2) { core -> sendMsg(addr, "Zbyt mało parametrów!"); return; }
	QString nick = params.takeFirst();
	QString msg = params.join(" ");
	msgs[nick] = msg;
	msgs[nick + ":sender"] = sender;
	core -> sendMsg(addr, "Wiadomość zapisana.");
}

void CLeaveMsg::userAuthed(QString nick, QString account)
{
	if(msgs.contains(account))
	{
		QString sender = msgs[account + ":sender"];
		core -> sendMsg(nick, "Wiadomość od " + sender + ": " + msgs[account]);
		msgs.remove(account);
		msgs.remove(account + ":sender");
	}
}
