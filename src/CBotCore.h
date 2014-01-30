/**************************************************************************************
 *
 *	CBotCore.h
 *
 *	Main header file to be included in plugins.
 *	Short guide to writing plugins:
 *	1. Subclass CBotPlugin
 *	2. Register a command in the constructor
 *	3. Overload CBotPlugin::executeCommand - it will be called when
 *	   someone says the command
 *	4. Use CBotPlugin::core (instance of CBotCore) to access IRC
 *	   functions and CBotPlugin::settings to access various settings
 *
 **************************************************************************************/

#ifndef __CBOTCORE__
#define __CBOTCORE__

#include "IrcLibQt.h"
#include "CWindow.h"
#include "CBotSettings.h"
#include <QApplication>

class CBotPlugin;

/**************************************************************************************
 *
 *	CBotCore
 *
 *	Core class. Provides access to the IRC interface to the plugins.
 *
 **************************************************************************************/

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
	CBotCore(QApplication*);
	~CBotCore();
	
	QString getNick();
	QString getChannel();
	void botQuit();

	// methods for registering handlers
	void handleRawEvent(const char* event, const CBotPlugin* handler, const char* slot);
	void handleEvent(const char* event, const CBotPlugin* handler, const char* slot);
	void registerCommand(QString command, CBotPlugin* handler);
	
	// plugin-plugin interface
	void registerPluginId(CBotPlugin*, QString);
	CBotPlugin* getPlugin(QString id);
	

	//basic IRC functions
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
	void quit();
};


/**************************************************************************************
 *
 *	CBotPlugin
 *
 *	Prototype class for plugins. Subclass to write a plugin.
 *
 **************************************************************************************/

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


/**************************************************************************************
 *
 *	CCorePlugin
 *
 *	Basic plugin responsible for handling "quit" and "rejoin" commands.
 *
 **************************************************************************************/

class CCorePlugin : public CBotPlugin
{
Q_OBJECT
public:
	CCorePlugin(CBotCore*, CBotSettings*);
	~CCorePlugin();

	void executeCommand(QString, QStringList, QString, QString);
};

#endif
