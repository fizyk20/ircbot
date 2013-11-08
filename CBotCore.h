#ifndef __CBOTCORE__
#define __CBOTCORE__

#include "IrcLibQt.h"
#include "CWindow.h"
#include "CBotSettings.h"
#include <QtCore/QCoreApplication>

class CBotPlugin;

class CBotCore : public QObject
{
Q_OBJECT
	CIrcSession* sess;
	CWindow* wnd;
	CBotSettings* settings;
	
	QVector<CBotPlugin*> plugins;
	QMap<QString,CBotPlugin*> commands;
	QMap<QString,CBotPlugin*> idPlugins;
	
	QString orig_nick,nick,email,imie,serwer,port,kanal;
public:
	CBotCore(QCoreApplication*);
	~CBotCore();
	
	void handleRawEvent(const char* event, const CBotPlugin* handler, const char* slot);
	void handleEvent(const char* event, const CBotPlugin* handler, const char* slot);
	void registerCommand(QString command, CBotPlugin* handler);
	
	void registerPluginId(CBotPlugin*, QString);
	CBotPlugin* getPlugin(QString id);
	
	bool master(QString);
	void sendMsg(QString, QString);
	void sendMsgChannel(QString);
	void channelMode(QString, QString);
	void kickUser(QString, QString);
	
	CIrcSession* session();

public slots:
	void botConnect();
	void botDisconnect();
	
	void connected();
	void disconnected();
	
	void evWelcome();
	void evMOTD(QString);
	void evNickInUse();
	
	void packPrivMsg(IrcParams);
	void packNotice(IrcParams);
	void packError(QString);
	void packJoin(IrcParams);
	void packPart(IrcParams);
	void packNick(IrcParams);
	void packQuit(IrcParams);
	void packMode(IrcParams);
	void packKick(IrcParams);
	void packOther(QString);
	
	void Log(QString, bool file = true);
	
	void programEnd();
	
signals:
	void ircMessage(QString sender, QString addr, QString msg);
	void ircNickChange(QString oldNick, QString newNick);
};

class CBotPlugin : public QObject
{
Q_OBJECT
protected:
	CBotCore* core;
	CBotSettings* settings;
public:
	CBotPlugin(CBotCore*, CBotSettings*);
	virtual ~CBotPlugin();
	
	virtual void executeCommand(QString command, QStringList params, QString addr, QString sender) = 0;
};

#endif
