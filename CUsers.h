#ifndef __USERS__
#define __USERS__

#include "CBotCore.h"

#define ST_USER 0
#define ST_VOICE 1 //+
#define ST_HOP 2 //%
#define ST_OP 4 //@

struct User
{
	int status;
	bool present;
	QString nick;
	QString name;
	QString mask;
	QString seen;
};

class CUsersSocket : public QTcpSocket
{
Q_OBJECT
public:
	CUsersSocket();
	~CUsersSocket();
	
	void updateList(QString host, QString packet);
};

class CUsers : public CBotPlugin
{
Q_OBJECT
	QVector<User> users;
	
	void aktualizuj();
	QString remove_prefix(QString);
	QString add_prefix(QString);
	
	QString date();
	
	void Load();
	void Save();
	
	void Join(QString nick, QString mask);
public:
	CUsers(CBotCore*, CBotSettings*);
	~CUsers();
	
	void executeCommand(QString, QStringList, QString, QString);
	
	int status(QString);
	int Find(QString nick);
	User operator[](int i);
	
public slots:
	void packMode(IrcParams);
	void packNick(IrcParams);
	void packJoin(IrcParams);
	void packPart(IrcParams);
	void packQuit(IrcParams);
	void packKick(IrcParams);
	void packPrivMsg(IrcParams);
	void evNameReply(IrcParams);
	void evWhoIsUser(IrcParams);
	void botDisconnected();
};

#endif
