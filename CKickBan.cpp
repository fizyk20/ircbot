#include "CKickBan.h"
#include "CUsers.h"

Vote::Vote()
{
	vFor = vAgainst = vNone = 0;
}

Vote::~Vote()
{
}

void Vote::voteFor()
{
	vFor ++;
}

void Vote::voteAgainst()
{
	vAgainst ++;
}

void Vote::voteNone()
{
	vNone ++;
}

double Vote::resultFor()
{
	if(vFor + vAgainst + vNone == 0) return 0.;
	return double(vFor) / (vFor + vAgainst + vNone);
}

double Vote::resultAgainst()
{
	if(vFor + vAgainst + vNone == 0) return 0.;
	return double(vAgainst) / (vFor + vAgainst + vNone);
}

double Vote::resultNone()
{
	if(vFor + vAgainst + vNone == 0) return 0.;
	return double(vNone) / (vFor + vAgainst + vNone);
}

/*********************************************************************/

CKickBan::CKickBan(CBotCore* c, CBotSettings* s)
	: CBotPlugin(c, s)
{
	vote = NULL;
	state = Waiting;
	next_id = 0;

	core -> registerCommand("kick", this);
	core -> registerCommand("ban", this);
	core -> registerCommand("unban", this);
	core -> registerCommand("banlist", this);
	core -> registerCommand("votekick", this);
	core -> registerCommand("voteban", this);
	core -> registerCommand("voteunban", this);

	core -> handleRawEvent(SIGNAL(evBanList(IrcParams)), this, SLOT(evBanList(IrcParams)));
	core -> handleRawEvent(SIGNAL(evEndBanList()), this, SLOT(evEndBanList()));
	core -> handleRawEvent(SIGNAL(packMode(IrcParams)), this, SLOT(packMode(IrcParams)));
}

CKickBan::~CKickBan()
{
	if(vote != NULL)
		delete vote;
}

void CKickBan::executeCommand(QString command, QStringList params, QString addr, QString sender)
{
	CUsers* users = (CUsers*) core -> getPlugin("users");
	if(command == "kick")
	{
		if(! core -> master(sender))
		{
			core -> sendMsg(addr, "Nie masz uprawnień do wydania tej komendy.");
			return;
		}
		if(params.length() < 1)
		{
			core -> sendMsg(addr, "Za mało parametrów!");
			return;
		}
		core -> kickUser(params[0], (params.length() > 1) ? params[1] : "");
	}
	else if(command == "ban")
	{
		if(! core -> master(sender))
		{
			core -> sendMsg(addr, "Nie masz uprawnień do wydania tej komendy.");
			return;
		}
		if(params.length() < 1)
		{
			core -> sendMsg(addr, "Za mało parametrów!");
			return;
		}
		int id = users -> Find(params[0]);
		if(id == -1)
		{
			core -> sendMsg(addr, "Nie ma takiego użytkownika.");
			return;
		}
		User u = (*users)[id];
		bans[u.nick] = "*!*@" + u.mask;
		core -> channelMode("+b", "*!*@" + u.mask);
		core -> kickUser(params[0], (params.length() > 1) ? params[1] : "");
	}
	else if(command == "unban")
	{
		if(! core -> master(sender))
		{
			core -> sendMsg(addr, "Nie masz uprawnień do wydania tej komendy.");
			return;
		}
		if(params.length() < 1)
		{
			core -> sendMsg(addr, "Za mało parametrów!");
			return;
		}
		if(! bans.contains(params[0]))
		{
			core -> sendMsg(addr, "Nie ma bana o takiej nazwie!");
			return;
		}
		core -> channelMode("-b", bans[params[0]]);
		bans.remove(params[0]);
	}
	else if(command == "banlist")
	{
		list_reply = addr;
		core -> channelMode("b", "");
	}
	else if(command == "votekick")
	{

	}
	else if(command == "voteban")
	{

	}
	else if(command == "voteunban")
	{

	}
}

void CKickBan::evBanList(IrcParams p)
{
	if(bans.key(p.params[3]) == "")
	{
		bans["ban" + QString::number(next_id)] = p.params[3];
		next_id++;
	}
}

void CKickBan::evEndBanList()
{
	core -> sendMsg(list_reply, "Lista banów:");
	QMap<QString, QString>::const_iterator it;

	for(it = bans.begin(); it != bans.end(); it++)
		core -> sendMsg(list_reply, it.key() + " - " + it.value());
}

void CKickBan::packMode(IrcParams p)
{

}
