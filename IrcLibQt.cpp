#include "IrcLibQt.h"
#include <QRegExp>
#include <QStringList>

CIrcSession::CIrcSession()
{
	packet_buffer = "";
	connect(this, SIGNAL(connected()), this, SLOT(connectedSlot()));
	connect(this, SIGNAL(readyRead()), this, SLOT(processData()));
}

CIrcSession::~CIrcSession()
{
}

void CIrcSession::Connect(QString serv, short p, QString n, QString m, QString nm)
{
	server = serv;
	port = p;
	nick = n;
	mail = m;
	name = nm;
	
	connectToHost(serv, p);
}

void CIrcSession::connectedSlot()
{
	Nick(nick);
	
	QStringList s = mail.split("@");
	if(s.size()<2) return;
	User(s[0], "\"" + s[1] + "\"", name);
}

void CIrcSession::processData()
{
	int n = bytesAvailable();
	
	char* buffer = new char[n+1];
	readData(buffer, n);
	buffer[n] = 0;
	
	packet_buffer += QString::fromUtf8(buffer);
	
	delete[] buffer;
	
	QStringList packets = packet_buffer.split("\n");
	packet_buffer = packets[packets.size()-1];
	packets.pop_back();
	
	int i;
	for(i=0; i<packets.size(); i++)
	{
		IrcPacket p;
		p = PacketToStruct(packets[i]);
		ProcessPacket(p);
	}
}

int CIrcSession::ProcessPacket(IrcPacket p)
{
	int num,a;
	bool oth=true;
	
	IrcParams par;
	QString temp;
	QStringList temp_list;
	
	int i;
	
	num = p.command.toInt();
	
	par.params.clear();
	
	i = p.hdr.indexOf('!');
	
	if(i != -1)
	{
		par.mask = p.hdr.mid(i,p.hdr.length());
		par.params.push_back(p.hdr.mid(0,i));
	}
	else
	{
		par.mask = "";
		par.params.push_back(p.hdr);
	}
	
	for(i=0; i<p.params.size(); i++)
		par.params.push_back(p.params[i]);
	
	if(num==0)
	{
		if(p.command == "PRIVMSG")
		{
			emit packPrivMsg(par);
			oth=false;
		}
		if(p.command == "PING")
		{
			Pong(p.params[0]);
			oth=false;
		}
		if(p.command == "NOTICE")
		{
			emit packNotice(par);
			oth=false;
		}
		if(p.command == "ERROR")
		{
			emit packError(p.params[0]);
			oth=false;
		}
		if(p.command == "JOIN")
		{
			emit packJoin(par);
			oth=false;
		}
		if(p.command == "PART")
		{
			emit packPart(par);
			oth=false;
		}
		if(p.command == "QUIT")
		{
			emit packQuit(par);
			oth=false;
		}
		if(p.command == "NICK")
		{
			emit packNick(par);
			oth=false;
		}
		if(p.command == "MODE")
		{
			emit packMode(par);
			oth=false;
		}
		if(p.command == "KICK")
		{
			emit packKick(par);
			oth=false;
		}
		if(oth)
		{
			emit packOther(p.params[0]);
		}
	}
	else
	{
		switch(num)
		{
			case RPL_WELCOME:
				a = p.params[1].indexOf('@');
				myhost = p.params[1].mid(a+1,p.params[1].length());
				myhost = myhost.split(QRegExp("\\s+"))[0];
				emit evWelcome();
				break;
			case RPL_MOTD:
				emit evMOTD(p.params[1]);
				break;
			case RPL_NAMREPLY:
				temp_list = p.params[p.params.size()-1].split(QRegExp("\\s+"), QString::SkipEmptyParts);
				int j;
				par.params.clear();
				for(j=0; j<temp_list.size(); j++)
					par.params.push_back(temp_list[j]);
				
				emit evNameReply(par);
				break;
			case RPL_ENDOFNAMES:
				break;
			case RPL_BANLIST:
				emit evBanList(par);
				break;
			case RPL_ENDOFBANLIST:
				emit evEndBanList();
				break;
			case RPL_WHOISUSER:
				emit evWhoIsUser(par);
				break;
			case ERR_NICKNAMEINUSE:
				emit evNickInUse();
				break;
			default:
				break;
		}
	}
	return 0;
}

void CIrcSession::sendPacket(IrcPacket pack)
{
	QString a;
	a = StructToPacket(pack);

	QByteArray pom = a.toUtf8();
	writeData(pom.data(),pom.size());
}

//poszczególne pakiety

void CIrcSession::Nick(QString nick)
{
	IrcPacket pack;
	
	pack.hdr = "";
	pack.command = "NICK";
	pack.params.clear();
	pack.params.push_back(nick);
	
	sendPacket(pack);
}

void CIrcSession::User(QString uname,QString host,QString name)
{
	IrcPacket pack;
	
	pack.hdr = "";
	pack.command = "USER";
	pack.params.clear();
	pack.params.push_back(uname);
	pack.params.push_back(host);
	pack.params.push_back(QString("\"") + server + "\"");
	pack.params.push_back(name);
	
	sendPacket(pack);
}

void CIrcSession::Quit(QString reason)
{
	IrcPacket pack;
	
	pack.hdr = "";
	pack.command = "QUIT";
	pack.params.clear();
	pack.params.push_back(reason);
	
	sendPacket(pack);
	
	server = "";
	port=0;
	close();
}

void CIrcSession::Join(QString channel)
{
	IrcPacket pack;
	
	pack.hdr = "";
	pack.command = "JOIN";
	pack.params.clear();
	pack.params.push_back(channel);
	
	sendPacket(pack);
}

void CIrcSession::Part(QString channel)
{
	IrcPacket pack;
	
	pack.hdr = "";
	pack.command = "PART";
	pack.params.clear();
	pack.params.push_back(channel);
	
	sendPacket(pack);
}

void CIrcSession::PrivMsg(QString to,QString text)
{
	IrcPacket pack;
	
	pack.hdr = "";
	pack.command = "PRIVMSG";
	pack.params.clear();
	pack.params.push_back(to);
	pack.params.push_back(text);
	
	sendPacket(pack);
}

void CIrcSession::Kick(QString channel,QString nick,QString text)
{
	IrcPacket pack;
	
	pack.hdr = "";
	pack.command = "KICK";
	pack.params.clear();
	pack.params.push_back(channel);
	pack.params.push_back(nick);
	pack.params.push_back(text);
	
	sendPacket(pack);
}

void CIrcSession::Mode(QString who,QString mode,QString param)
{
	IrcPacket pack;
	
	pack.hdr = "";
	pack.command = "MODE";
	pack.params.clear();
	pack.params.push_back(who);
	pack.params.push_back(mode);
	if(param != "")
		pack.params.push_back(param);
	
	sendPacket(pack);
}

void CIrcSession::Pong(QString param)
{
	IrcPacket pack;
	
	pack.hdr = "";
	pack.command = "PONG";
	pack.params.clear();
	pack.params.push_back(param);
	
	sendPacket(pack);
}

void CIrcSession::WhoIs(QString nick)
{
	IrcPacket pack;

	pack.hdr = "";
	pack.command = "WHOIS";
	pack.params.clear();
	pack.params.push_back(nick);

	sendPacket(pack);
}

/******************************************************************/

//funkcje pomocnicze

QString StructToPacket(IrcPacket pack)
{
	QString p;
	p = "";
	if(pack.hdr != "") p += pack.hdr + " ";
	p += pack.command;
	
	for(int i=0; i<pack.params.size(); i++)
		if(pack.params[i].indexOf(' ') == -1 && (i<pack.params.size()-1))
		{
			p += " ";
			p += pack.params[i];
		}
		else
		{
			p += " :";
			p += pack.params[i];
		}
		
	p += "\r\n";
	return p;
}

IrcPacket PacketToStruct(QString pack)
{
	int i;
	IrcPacket pk;
	
	while(pack[pack.length()-1]== '\n' || pack[pack.length()-1] == '\r') pack = pack.mid(0,pack.length()-1);
	
	QStringList arr = pack.split(" ", QString::SkipEmptyParts);
	
	if(arr[0][0] == ':') 
	{
		pk.hdr = arr[0].mid(1,arr[0].length()-1);
		pk.command = arr[1];
		i=2;
	}
	else
	{
		pk.hdr = "";
		pk.command = arr[0];
		i=1;
	}
	
	pk.params.clear();
	
	bool b=false;
	
	for(; i<arr.size(); i++)
	{
		if(!b) pk.params.push_back(arr[i]);
		else pk.params[pk.params.size()-1] += QString(" ") + arr[i];
		if(arr[i][0] == ':') b = true;
	}
	
	if(b)
	{
		QString trail = pk.params[pk.params.size()-1];
		pk.params.pop_back();
		trail = trail.right(trail.length()-1);
		pk.params.push_back(trail);
	}
	
	return pk;
}
