#include "CBotCore.h"
#include <QDateTime>
#include <QRegExp>
#include "permissions.h"
#include "CUsers.h"
#include "roll.h"
#include "randomchat.h"
#include "CWzorce.h"
#include "CEightball.h"
#include "russian.h"
#include "badwords.h"
#include "antiflood.h"
#include "CKickBan.h"
#include "leavemsg.h"

CBotCore::CBotCore(QApplication* app)
{
	settings = new CBotSettings("settings.ini");
	sess = new CIrcSession;
	
	wnd = new CWindow;
	
	// Setup quitting signals
	connect(app, SIGNAL(aboutToQuit()), this, SLOT(programEnd()));
	connect(this, SIGNAL(quit()), app, SLOT(quit()));
	
	// Setup Connect/Disconnect buttons
	connect(wnd, SIGNAL(botConnect()), this, SLOT(botConnect()));
	connect(wnd, SIGNAL(botDisconnect()), this, SLOT(botDisconnect()));
	
	// Setup IRC signal handlers
	connect(sess, SIGNAL(connected()), this, SLOT(connected()));
	connect(sess, SIGNAL(disconnected()), this, SLOT(disconnected()));
	
	connect(sess, SIGNAL(evWelcome()), this, SLOT(evWelcome()));
	connect(sess, SIGNAL(evMOTD(QString)), this, SLOT(evMOTD(QString)));
	connect(sess, SIGNAL(evNickInUse()), this, SLOT(evNickInUse()));
	
	connect(sess, SIGNAL(packPrivMsg(IrcParams)), this, SLOT(packPrivMsg(IrcParams)));
	connect(sess, SIGNAL(packNotice(IrcParams)), this, SLOT(packNotice(IrcParams)));
	connect(sess, SIGNAL(packNick(IrcParams)), this, SLOT(packNick(IrcParams)));
	connect(sess, SIGNAL(packJoin(IrcParams)), this, SLOT(packJoin(IrcParams)));
	connect(sess, SIGNAL(packPart(IrcParams)), this, SLOT(packPart(IrcParams)));
	connect(sess, SIGNAL(packQuit(IrcParams)), this, SLOT(packQuit(IrcParams)));
	connect(sess, SIGNAL(packMode(IrcParams)), this, SLOT(packMode(IrcParams)));
	connect(sess, SIGNAL(packKick(IrcParams)), this, SLOT(packKick(IrcParams)));
	connect(sess, SIGNAL(packError(QString)), this, SLOT(packError(QString)));
	connect(sess, SIGNAL(packOther(QString)), this, SLOT(packOther(QString)));
	
	// Read basic settings
	serwer = settings -> GetString("server");
	kanal = settings -> GetString("channel");
	port = settings -> GetString("port");
	orig_nick = nick = settings -> GetString("nick");
	imie = settings -> GetString("name");
	email = settings -> GetString("email");
	
	Log("********************* Start programu *********************");
	
	// Load plugins
	plugins.clear();
	plugins.push_back(new CCorePlugin(this, settings));
	plugins.push_back(new CPermissions(this, settings));
	plugins.push_back(new CUsers(this, settings));
	plugins.push_back(new CRoll(this, settings));
	plugins.push_back(new CRandomChat(this, settings));
	plugins.push_back(new CWzorce(this, settings));
	plugins.push_back(new CEightball(this, settings));
	plugins.push_back(new CRussian(this, settings));
	plugins.push_back(new Badwords(this, settings));
	plugins.push_back(new AntiFlood(this, settings));
	plugins.push_back(new CKickBan(this, settings));
	plugins.push_back(new CLeaveMsg(this, settings));
}

CBotCore::~CBotCore()
{
}

QString CBotCore::getNick()
{
	return nick;
}

QString CBotCore::getChannel()
{
	return kanal;
}

void CBotCore::botQuit()
{
	emit quit();
}


// Raw event (= signal from IrcSession) handler registration
void CBotCore::handleRawEvent(const char* event, const CBotPlugin* handler, const char* slot)
{
	connect(sess, event, handler, slot);
}

// Processed event handler registration
void CBotCore::handleEvent(const char* event, const CBotPlugin* handler, const char* slot)
{
	connect(this, event, handler, slot);
}

// Registration of a command
void CBotCore::registerCommand(QString command, CBotPlugin* handler)
{
	commands[command] = handler;
}

// Registration of a plugin to be read by CBotCore::getPlugin
void CBotCore::registerPluginId(CBotPlugin* plugin, QString id)
{
	idPlugins[id] = plugin;
}


// Plugin getter for plugin-plugin communication
CBotPlugin* CBotCore::getPlugin(QString id)
{
	if(idPlugins.find(id) != idPlugins.end())
		return idPlugins[id];
	else
		return NULL;
}

// Check if a user is a master (deprecated, permissions should be used instead)
bool CBotCore::master(QString nick)
{
	CUsers* users = (CUsers*)getPlugin("users");
	QString account = users -> getAccount(nick);
	if(account == "") return false;
	bool b = false;
	QFile f("masters.ini");
	f.open(QIODevice::ReadOnly | QIODevice::Text);

	QTextStream fin(&f);

	while(!fin.atEnd())
	{
		QString line = fin.readLine();
		if(line == account)
		{
			b = true;
			break;
		}
	}

	f.close();
	return b;
}

void CBotCore::sendMsg(QString addr, QString txt)
{
	QString add_s = "";
	if(addr == "") addr = kanal;
	if(addr != kanal) add_s = "|priv";
	Log("<" + nick + add_s + "> " + txt);
	sess -> PrivMsg(addr, txt);
}

void CBotCore::sendMsgChannel(QString txt)
{
	Log("<" + nick + "> " + txt);
	sess -> PrivMsg(kanal, txt);
}

void CBotCore::channelMode(QString mode, QString params)
{
	sess -> Mode(kanal, mode, params);
}

void CBotCore::kickUser(QString who, QString reason)
{
	if(who == nick) return;
	sess -> Kick(kanal, who, reason);
}

CIrcSession* CBotCore::session()
{
	return sess;
}

void CBotCore::botConnect()
{
	Log("*** Łączenie... " + serwer + ":" + port);
	sess->Connect(serwer, port.toInt(), nick, email, imie);
	Log("*** Koniec Connect.");
}

void CBotCore::botDisconnect()
{
	sess->disconnectFromHost();
}

void CBotCore::connected()
{
	Log("*** Połączono.");
}

void CBotCore::disconnected()
{
	Log("*** Rozłączono.");
}

void CBotCore::evWelcome()
{
	Log("*** Odebrano Welcome.");
	if(nick != orig_nick)
	{
		Log("Ghostuję wrednego człowieka.");
		sess -> PrivMsg("NickServ","ghost " + orig_nick + " " + settings->GetString("password"));
	}
	kanal = settings->GetString("channel");
	Log("*** Dołączanie do kanału " + kanal);
	sess -> Join(kanal);
}

void CBotCore::evMOTD(QString txt)
{
	Log("*** MOTD: " + txt, false);
}

void CBotCore::evNickInUse()
{
	Log("*** Nick " + nick + " już jest używany przez kogoś innego - próbuję " + nick + "_.");
	nick += "_";
	sess -> Nick(nick);

	QStringList s = email.split("@");
	if(s.size()<2) return;
	sess -> User(s[0], "\"" + s[1] + "\"", imie);
}

// Log the message and pass it to handlers
void CBotCore::packPrivMsg(IrcParams p)
{
	QString appendix = "";
	QString addr = kanal;
	if(p.params[1] == nick)
	{
		appendix = "|priv";
		addr = p.params[0];
	}
	Log("<" + p.params[0] + appendix + "> " + p.params[2]);
	
	if(p.params[2].mid(0,1) == settings->GetString("commandChar"))
	{
		int i = p.params[2].indexOf(" ");
		QString command = p.params[2].mid(1,i-1);
		if(commands.find(command) == commands.end())
			return;
		QStringList params;
		if(i >= 0)
			params = p.params[2].mid(i+1).split(" ",QString::SkipEmptyParts);

		CPermissions* perm = (CPermissions*)getPlugin("permissions");
		if(!perm -> checkPrivilege(p.params[0], command, params))
		{
			sendMsg(addr, "Nie masz uprawnień do tego polecenia.");
			return;
		}
		commands[command]->executeCommand(command, params, addr, p.params[0]);
	}
	else
		emit ircMessage(p.params[0], addr, p.params[2]);
}

// Log notice packets, also handles auto-ghosting
void CBotCore::packNotice(IrcParams p)
{
	Log("*** NOTICE " + p.params[1] + ": " + p.params[2]);
	if(p.params[2].contains("This nickname is registered."))
		sess->PrivMsg("NickServ","identify "+settings->GetString("password"));
	if(p.params[2].contains("ghosted"))
	{
		sess->Nick(orig_nick);
		nick = orig_nick;
	}
}

// Log error
void CBotCore::packError(QString txt)
{
	Log("*** ERROR: " + txt);
}

// Log join and user mask
void CBotCore::packJoin(IrcParams p)
{
	Log(p.params[0] + " (" + p.mask + tr(") dołączył do ") + p.params[1]);
}

// Log part message
void CBotCore::packPart(IrcParams p)
{
	QString temp = p.params[0];
	temp += tr(" opuścił ") + p.params[1];
	if(p.params.size()>2) temp += " (" + p.params[2] + ")";
	Log(temp);
}

// Log nick change
void CBotCore::packNick(IrcParams p)
{
	Log(p.params[0] + tr(" jest teraz znany jako ") + p.params[1]);
	emit ircNickChange(p.params[0], p.params[1]);
}

// Log quit message
void CBotCore::packQuit(IrcParams p)
{
	QString temp = p.params[0];
	temp += " (" + p.mask + tr(") wyszedł");
	if(p.params.size()>1) temp += " (" + p.params[1] + ")";
	Log(temp);
}

// Log mode change, also attempt to reverse a ban on the bot
void CBotCore::packMode(IrcParams p)
{
	if(p.params.size() > 3 && p.params[1] == kanal && p.params[2] == "+b")
	{
		CUsers* users = (CUsers*) getPlugin("users");
		int id = users -> Find(nick);
		if(id == -1) return;
		User u = (*users)[id];
		QRegExp rx(p.params[3]);
		rx.setPatternSyntax(QRegExp::Wildcard);
		if(rx.exactMatch(u.nick + "!" + u.name + "@" + u.mask))
		{
			sess -> Mode(p.params[1], "-b", p.params[3]);
			return;
		}
	}
	QString temp;
	
	temp = p.params[0];
	temp += tr(" ustawił tryb ") + p.params[1];
	
	if(p.params.size()>2) temp += " " + p.params[2];
	
	if(p.params.size()>3)
	{
		temp += " (";
		
		for(int i=3; i<p.params.size(); i++)
			temp += p.params[i] + " ";
		
		temp = temp.mid(0,temp.length()-1);
		
		temp += ")";
	}
	
	Log(temp);
}

// Handle kicks - autojoin if kicked
void CBotCore::packKick(IrcParams p)
{
	if(p.params[2] == nick)
	{
		sess -> Join(kanal);
		return;
	}
	QString temp = p.params[2];
	temp += tr(" został wyrzucony z ") + p.params[1];
	temp += " przez " + p.params[0];
	if(p.params.size()>3) temp += " (" + p.params[3] + ")";
	Log(temp);
}

// Log that something was received
void CBotCore::packOther(QString txt)
{
	Log("*** Other packet: " + txt);
}

// Log program end, clear plugins, delete objects no longer being necessary
void CBotCore::programEnd()
{
	Log("********************* Koniec programu ********************\n");
	
	while(plugins.size() > 0)
	{
		delete plugins.last();
		plugins.pop_back();
	}
	delete sess;
	delete settings;
	delete wnd;
}

// Logging function
void CBotCore::Log(QString txt, bool file)
{
	QDateTime time = QDateTime::currentDateTime();
	QString wndtxt, filetxt;
	
	wndtxt = time.toString("(hh:mm:ss)") + " " + txt;
	filetxt = time.toString("(dd.MM.yyyy, hh:mm:ss)") + " " + txt;
	
	wnd->Log(wndtxt);
	
	if(file)
	{
		QFile f("log.txt");
		f.open(QIODevice::Append | QIODevice::Text);
		
		QTextStream fout(&f);
		
		fout << filetxt << "\n";
		
		f.close();
	}
}

/****************************************************************************/

// Remember core and settings objects in properties
CBotPlugin::CBotPlugin(CBotCore* c, CBotSettings* s)
{
	core = c;
	settings = s;
}

CBotPlugin::~CBotPlugin()
{
}

/****************************************************************************/

// Core plugin - register "quit" and "rejoin"
CCorePlugin::CCorePlugin(CBotCore* c, CBotSettings* s)
	: CBotPlugin(c, s)
{
	core -> registerCommand("rejoin", this);
	core -> registerCommand("quit", this);
}

CCorePlugin::~CCorePlugin()
{
}

// Handle registered commands
void CCorePlugin::executeCommand(QString command, QStringList, QString, QString sender)
{
	if(command == "rejoin")
	{
		core -> session() -> Join(core -> getChannel());
		return;
	}
	if(command == "quit" && core -> master(sender))
	{
		core -> botQuit();
		return;
	}
}
