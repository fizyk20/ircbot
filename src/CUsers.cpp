#include "CUsers.h"
#include <QDateTime>
#include <QTcpSocket>
#include <QStringList>
#include <QRegExp>

CUsersSocket::CUsersSocket()
{
}

CUsersSocket::~CUsersSocket()
{
}

void CUsersSocket::updateList(QString host, QString packet)
{
	connectToHost(host, 80);
	if(!waitForConnected(10000)) return;
	writeData(packet.toUtf8().constData(), packet.length());
	disconnectFromHost();
	waitForDisconnected(10000);
}

CUsers::CUsers(CBotCore* c, CBotSettings* s)
	: CBotPlugin(c, s)
{	
	core->handleRawEvent(SIGNAL(packMode(IrcParams)), this, SLOT(packMode(IrcParams)));
	core->handleRawEvent(SIGNAL(packNick(IrcParams)), this, SLOT(packNick(IrcParams)));
	core->handleRawEvent(SIGNAL(packJoin(IrcParams)), this, SLOT(packJoin(IrcParams)));
	core->handleRawEvent(SIGNAL(packPart(IrcParams)), this, SLOT(packPart(IrcParams)));
	core->handleRawEvent(SIGNAL(packQuit(IrcParams)), this, SLOT(packQuit(IrcParams)));
	core->handleRawEvent(SIGNAL(packKick(IrcParams)), this, SLOT(packKick(IrcParams)));
	core->handleRawEvent(SIGNAL(packPrivMsg(IrcParams)), this, SLOT(packPrivMsg(IrcParams)));
	core->handleRawEvent(SIGNAL(packNotice(IrcParams)), this, SLOT(packNotice(IrcParams)));
	core->handleRawEvent(SIGNAL(evNameReply(IrcParams)), this, SLOT(evNameReply(IrcParams)));
	core->handleRawEvent(SIGNAL(evWhoIsUser(IrcParams)), this, SLOT(evWhoIsUser(IrcParams)));
	core->handleRawEvent(SIGNAL(disconnected()), this, SLOT(botDisconnected()));
	core->registerCommand("seen", this);
	core->registerCommand("wakeup", this);
	core->registerCommand("auth", this);
	
	core->registerPluginId(this, "users");
	
	Load();

	nickBeingChecked = "";
}

CUsers::~CUsers()
{
	botDisconnected();
}

void CUsers::executeCommand(QString command, QStringList params, QString addr, QString sender)
{
	if(command == "seen")
	{
		if(params.length()<1)
		{
			core->sendMsg(addr, "Brak parametru.");
			return;
		}
		
		int i = Find(params[0]);
		if(i == -1)
			core -> sendMsg(addr, params[0] + "ostatnio widziany/a: nigdy");
		else
			core -> sendMsg(addr, params[0] + " ostatnio widziany/a: " + users[i].seen);
	}
	
	if(command == "wakeup")
	{
		QString msg = "Pobudka ";
		int i;
		for(i=0; i<users.size(); i++)
		{
			if(users[i].present) msg += users[i].nick + ", ";
		}
		
		msg = msg.left(msg.length()-2) + "!";
		
		core -> sendMsgChannel(msg);
	}

	if(command == "auth")
	{
		nickBeingChecked = sender;
		core -> sendMsg(settings -> GetString("users_authserv"), "info =" + sender);
	}
}

QString CUsers::date()
{
	return QDateTime::currentDateTime().toString("dd.MM.yyyy, hh:mm:ss");
}

QString CUsers::remove_prefix(QString nick)
{
	if(status(nick)!=0)
		return nick.right(nick.length()-1);
	return nick;
}

QString CUsers::add_prefix(QString nick)
{
	int n;
	n = Find(nick);
	if(n==-1) return "";
	
	QString a;
	switch(users[n].status)
	{
		case ST_OP:
			a = "@";
			break;
		case ST_HOP:
			a = "%";
			break;
		case ST_VOICE:
			a = "+";
			break;
		default:
			a = "";
			break;
	}
	a += nick;
	return a;
}

int CUsers::status(QString nick)
{
	switch(nick[0].toAscii())
	{
		case '@':
			return ST_OP;
		case '%':
			return ST_HOP;
		case '+':
			return ST_VOICE;
		default:
			return ST_USER;
	}
}

void CUsers::aktualizuj()
{
	if(settings -> GetBool("user_update_file"))
	{
		QString fileName = settings -> GetString("users_file");
		QFile file(fileName);
		file.open(QIODevice::WriteOnly | QIODevice::Text);

		QTextStream out(&file);
		out << presentUsers() + 1 << "\n"; // take the bot into account
		int i;
		for(i=0; i<users.size(); i++)
			if(users[i].present) out << users[i].nick + "\n";

		file.close();
	}
	else
	{
		QString u_list = "";
		QString packet = "";
		QString host = settings->GetString("host");
		QString path = settings->GetString("host_path");

		int i;
		for(i=0; i<users.size(); i++)
			if(users[i].present) u_list += users[i].nick + "%20";

		u_list = u_list.mid(0,u_list.length()-3);

		packet = "POST " + path + "/userlist.php HTTP/1.1\r\n";
		packet += "Host: " + host + "\r\n";

		QString len = QString::number(u_list.length()+6);
	
		packet += "Content-Length: " + len + "\r\n";
		packet += "Content-Type: application/x-www-form-urlencoded\r\n\r\n";
		packet += "ulist=" + u_list + "\r\n\r\n";
		
		CUsersSocket sock;
		sock.updateList(host, packet);
	}
}

void CUsers::Save()
{
	QFile file("users.ini");
	file.open(QIODevice::WriteOnly | QIODevice::Text);
	
	QTextStream fout(&file);
	
	int i;
	
	for(i=0; i<users.size(); i++)
	{
		if(users[i].nick.length()>0)
			fout << users[i].nick << "\n" << users[i].seen << "\n";
	}
	
	file.close();
}

void CUsers::Load()
{
	QFile file("users.ini");
	file.open(QIODevice::ReadOnly | QIODevice::Text);

	QTextStream fin(&file);
	
	QString nick,data;
	
	while(!fin.atEnd())
	{
		nick = fin.readLine();
		data = fin.readLine();
		User u;
		u.nick = nick;
		u.seen = data;
		u.present = false;
		u.status = 0;
		u.name = "";
		u.mask = "";
		if(u.nick.length()>0)
			users.push_back(u);
	}
	
	file.close();
}

int CUsers::Find(QString nick)
{
	int i;
	for(i=0; i<users.size(); i++)
		if(users[i].nick.toLower() == nick.toLower()) return i;
	return -1;
}

User CUsers::operator[](int i)
{
	return users[i];
}

User CUsers::operator[](QString nick)
{
	User result_null;
	result_null.nick = "";
	int i = Find(nick);
	if(i<0) return result_null;
	return users[i];
}

int CUsers::presentUsers()
{
	int i, result;
	result = 0;
	for(i=0; i<users.size(); i++)
		if(users[i].present) result++;
	result--;	// take the bot into account
	return result;
}

QString CUsers::getAccount(QString nick)
{
	int i = Find(nick);
	if(i<0) return "";
	return users[i].account;
}

void CUsers::packMode(IrcParams par)
{
	int i,j;
	bool add=false;
	
	QString mode = par.params[2];
	QString params = "";
	for(i=3; i<par.params.size(); i++)
		params += par.params[i] + " ";
	params = params.left(params.length()-1);
	
	QStringList p = params.split(QRegExp("\\s+"));
	
	j=0;
	for(i=0; i<mode.length(); i++)
	{
		if(mode[i] == '+')
		{
			add = true;
			continue;
		}
		if(mode[i] == '-')
		{
			add = false;
			continue;
		}
		if(j >= p.size()) break; //zabezpieczenie

		int id = Find(p[j]);
		if(id < 0)
		{
			j++;
			continue;
		}
		if(mode[i] == 'v')
		{
			if(add)
				users[id].status |= ST_VOICE;
			else
				users[id].status &= ~ST_VOICE;
		}
		if(mode[i] == 'h')
		{
			if(add)
				users[id].status |= ST_HOP;
			else
				users[id].status &= ~ST_HOP;
		}
		if(mode[i] == 'o')
		{
			if(add)
				users[id].status |= ST_OP;
			else
				users[id].status &= ~ST_OP;
		}
		j++;
	}
}

void CUsers::packNick(IrcParams p)
{
	QString nick = p.params[0];
	QString nick2 = p.params[1];
	
	int n = Find(nick);
	
	if(n==-1)
	{
		Join(nick2, "");
		return;
	}
	int m = Find(nick2);
	if(m!=-1)
	{
		users[m].seen = date();
		users[m].present = true;
		users[n].present = false;
	}
	else
	{
		users[n].nick = nick2;
		users[n].seen = date();
	}
	aktualizuj();
}

void CUsers::Join(QString nick, QString mask)
{
	int n = Find(remove_prefix(nick));
	if(n!=-1)
	{
		users[n].seen = date();
		users[n].present = true;
		users[n].status = status(nick);
		aktualizuj();
		return;
	}

	User u;

	u.nick = remove_prefix(nick);
	int iAt = mask.indexOf('@');
	if(iAt == -1)
	{
		u.name = "";
		u.mask = mask;
	}
	else
	{
		u.name = mask.mid(0, iAt);
		u.mask = mask.mid(iAt+1);
	}
	u.status = status(nick);
	u.seen = date();
	u.present = true;

	users.push_back(u);
}

void CUsers::packJoin(IrcParams p)
{	
	QString nick = p.params[0];
	QString mask = p.mask;
	
	Join(nick, mask);
	not_registered(nick);
	
	aktualizuj();
}

void CUsers::packPart(IrcParams p)
{
	int n = Find(p.params[0]);
	if(n==-1) return;
	users[n].present = false;
	users[n].seen = date();
	aktualizuj();
}

void CUsers::packQuit(IrcParams p)
{
	int n = Find(p.params[0]);
	if(n==-1) return;
	users[n].present = false;
	users[n].seen = date();
	aktualizuj();
}

void CUsers::packKick(IrcParams p)
{
	int n = Find(p.params[2]);
	if(n==-1) return;
	users[n].present = false;
	users[n].seen = date();
	aktualizuj();
}

void CUsers::packPrivMsg(IrcParams p)
{
	int n = Find(p.params[0]);
	if(n==-1) return;
	users[n].seen = date();
	int iExcl = p.mask.indexOf('!');
	int iAt = p.mask.indexOf('@');
	if(iExcl == -1 || iAt == -1)
	{
		users[n].name = "";
		users[n].mask = p.mask;
	}
	else
	{
		users[n].name = p.mask.mid(iExcl+1, iAt-iExcl-1);
		users[n].mask = p.mask.mid(iAt+1);
	}
	users[n].present = true;
}

void CUsers::registered(QString nick, QString account)
{
	int i = Find(nick);
	if(i < 0) return;
	users[i].account = account;
}

void CUsers::not_registered(QString nick)
{
	int i = Find(nick);
	if(i < 0) return;
	users[i].account = "";
}

void CUsers::packNotice(IrcParams p)
{
	if(p.params[0].toLower() != settings -> GetString("users_authserv").toLower()) return;

	QRegExp registered(settings -> GetString("users_regex_registered"));
	QRegExp unregistered(settings -> GetString("users_regex_unregistered"));

	if(p.params[2].contains(registered))
	{
		//nick is registered
		QString nick = nickBeingChecked;
		QString account = registered.cap(2);
		this->registered(nick, account);
		core -> sendMsg(nick, "OK, zarejestrowany jako " + account + ".");
		emit userAuthed(nick, account);
	}
	if(p.params[2].contains(unregistered))
	{
		//nick is not registered
		QString nick = nickBeingChecked;
		this->not_registered(nick);
		core -> sendMsg(nick, "BŁĄD - niezarejestrowany.");
	}
}

void CUsers::evNameReply(IrcParams p)
{
	int i;
	for(i=0; i<p.params.size(); i++)
	{
		Join(p.params[i], "");
		core -> session() -> WhoIs(remove_prefix(p.params[i]));
	}
	
	aktualizuj();
}

void CUsers::evWhoIsUser(IrcParams p)
{
	QString nick = p.params[2];
	QString name = p.params[3];
	QString mask = p.params[4];
	int i = Find(nick);
	if(i == -1) return;
	users[i].name = name;
	users[i].mask = mask;
}

void CUsers::botDisconnected()
{
	int i;
	for(i=0; i<users.size(); i++)
	{
		if(users[i].present)
			users[i].seen = date();
	}
	Save();

	if(settings -> GetBool("user_update_file"))
	{
		QString fileName = settings -> GetString("users_file");
		QFile file(fileName);
		file.open(QIODevice::WriteOnly | QIODevice::Text);

		QTextStream out(&file);
		out << "none";

		file.close();
	}
	else
	{
		QString host = settings->GetString("host");
		QString path = settings->GetString("host_path");

		QString packet = "";
		packet = "POST " + path + "/userlist.php HTTP/1.1\r\n";
		packet += "Host: " + host + "\r\n";
		packet += "Content-Length: 12\r\n";
		packet += "Content-Type: application/x-www-form-urlencoded\r\n\r\n";
		packet += "ulist=no_bot\r\n\r\n";

		CUsersSocket sock;
		sock.updateList(host, packet);
	}
}
