#include "CBotCore.h"
#include <QDateTime>
#include "CUsers.h"
#include "roll.h"
#include "randomchat.h"
#include "CWzorce.h"
#include "CEightball.h"
#include "russian.h"

CBotCore::CBotCore(QApplication* app)
{
	settings = new CBotSettings("settings.ini");
	sess = new CIrcSession;
	
	wnd = new CWindow;
	
	connect(app, SIGNAL(aboutToQuit()), this, SLOT(programEnd()));
	
	connect(wnd, SIGNAL(botConnect()), this, SLOT(botConnect()));
	connect(wnd, SIGNAL(botDisconnect()), this, SLOT(botDisconnect()));
	
	connect(sess, SIGNAL(connected()), this, SLOT(connected()));
	connect(sess, SIGNAL(disconnected()), this, SLOT(disconnected()));
	
	connect(sess, SIGNAL(evWelcome()), this, SLOT(evWelcome()));
	connect(sess, SIGNAL(evMOTD(QString)), this, SLOT(evMOTD(QString)));
	
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
	
	serwer = settings->GetString("server");
	kanal = settings->GetString("channel");
	port = settings->GetString("port");
	nick = settings->GetString("nick");
	imie = settings->GetString("name");
	email = settings->GetString("email");
	
	Log("********************* Start programu *********************");
	
	//load "plugins"
	plugins.clear();
	plugins.push_back(new CUsers(this, settings));
	plugins.push_back(new CRoll(this, settings));
	plugins.push_back(new CRandomChat(this, settings));
	plugins.push_back(new CWzorce(this, settings));
	plugins.push_back(new CEightball(this, settings));
	plugins.push_back(new CRussian(this, settings));
}

CBotCore::~CBotCore()
{
}

void CBotCore::handleRawEvent(const char* event, const CBotPlugin* handler, const char* slot)
{
	connect(sess, event, handler, slot);
}

void CBotCore::handleEvent(const char* event, const CBotPlugin* handler, const char* slot)
{
	connect(this, event, handler, slot);
}

void CBotCore::registerCommand(QString command, CBotPlugin* handler)
{
	commands[command] = handler;
}

void CBotCore::registerPluginId(CBotPlugin* plugin, QString id)
{
	idPlugins[id] = plugin;
}

CBotPlugin* CBotCore::getPlugin(QString id)
{
	if(idPlugins.find(id) != idPlugins.end())
		return idPlugins[id];
	else
		return NULL;
}

bool CBotCore::master(QString nick)
{
	bool b = false;
	QFile f("masters.ini");
	f.open(QIODevice::ReadOnly | QIODevice::Text);

	QTextStream fin(&f);

	while(!fin.atEnd())
	{
		QString line = fin.readLine();
		if(line == nick) 
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
	Log(nick + " ustawił tryb " + mode + " " + params);
	sess -> Mode(kanal, mode, params);
}

void CBotCore::kickUser(QString who, QString reason)
{
	Log(who + " został wyrzucony z kanału przez " + nick + " (" + reason + ")");
	sess -> Kick(kanal, who, reason);
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
	kanal = settings->GetString("channel");
	Log("*** Dołączanie do kanału " + kanal);
	sess -> Join(kanal);
}

void CBotCore::evMOTD(QString txt)
{
	Log("*** MOTD: " + txt, false);
}

void CBotCore::packPrivMsg(IrcParams p)
{
	QString appendix = "";
	QString addr = "";
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
		QStringList params = p.params[2].mid(i+1).split(" ",QString::SkipEmptyParts);
		commands[command]->executeCommand(command, params, addr, p.params[0]);
	}
	else
		emit ircMessage(p.params[0], p.params[1], p.params[2]);
}

void CBotCore::packNotice(IrcParams p)
{
	Log("*** NOTICE " + p.params[1] + ": " + p.params[2]);
	if(p.params[2].contains("This nickname is registered."))
		sess->PrivMsg("NickServ","identify "+settings->GetString("password"));
}

void CBotCore::packError(QString txt)
{
	Log("*** ERROR: " + txt);
}

void CBotCore::packJoin(IrcParams p)
{
	Log(p.params[0] + tr(" dołączył do ") + p.params[1]);
}

void CBotCore::packPart(IrcParams p)
{
	QString temp = p.params[0];
	temp += tr(" opuścił ") + p.params[1];
	if(p.params.size()>2) temp += " (" + p.params[2] + ")";
	Log(temp);
}

void CBotCore::packNick(IrcParams p)
{
	Log(p.params[0] + tr(" jest teraz znany jako ") + p.params[1]);
	emit ircNickChange(p.params[0], p.params[1]);
}

void CBotCore::packQuit(IrcParams p)
{
	QString temp = p.params[0];
	temp += tr(" wyszedł");
	if(p.params.size()>1) temp += " (" + p.params[1] + ")";
	Log(temp);
}

void CBotCore::packMode(IrcParams p)
{
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

void CBotCore::packKick(IrcParams p)
{
	QString temp = p.params[2];
	temp += tr(" został wyrzucony z ") + p.params[1];
	temp += " przez " + p.params[0];
	if(p.params.size()>3) temp += " (" + p.params[3] + ")";
	Log(temp);
}

void CBotCore::packOther(QString txt)
{
	Log("*** Other packet: " + txt);
}

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

CBotPlugin::CBotPlugin(CBotCore* c, CBotSettings* s)
{
	core = c;
	settings = s;
}

CBotPlugin::~CBotPlugin()
{
}
