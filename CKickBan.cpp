#include "CKickBan.h"
#include "CUsers.h"
#include <QTimer>
#include "permissions.h"

Vote::Vote()
{
	vFor = vAgainst = vNone = 0;
}

Vote::~Vote()
{
}

bool Vote::voteFor(QString nick)
{
	if(voted.contains(nick)) return false;
	voted.push_back(nick);
	vFor ++;
	return true;
}

bool Vote::voteAgainst(QString nick)
{
	if(voted.contains(nick)) return false;
	voted.push_back(nick);
	vAgainst ++;
	return true;
}

bool Vote::voteNone(QString nick)
{
	if(voted.contains(nick)) return false;
	voted.push_back(nick);
	vNone ++;
	return true;
}

double Vote::resultFor()
{
	if(vFor + vAgainst == 0) return 0.;
	return double(vFor) / (vFor + vAgainst);
}

double Vote::resultAgainst()
{
	if(vFor + vAgainst == 0) return 0.;
	return double(vAgainst) / (vFor + vAgainst);
}

int Vote::numVotes()
{
	return vFor + vAgainst + vNone;
}

/*********************************************************************/

CKickBan::CKickBan(CBotCore* c, CBotSettings* s)
	: CBotPlugin(c, s)
{
	vote = NULL;
	state = Waiting;
	next_id = 0;

	core -> registerCommand("vote", this);

	core -> registerCommand("kick", this);
	core -> registerCommand("ban", this);
	core -> registerCommand("unban", this);
	core -> registerCommand("banlist", this);
	core -> registerCommand("votekick", this);
	core -> registerCommand("voteban", this);
	core -> registerCommand("voteunban", this);

	core -> registerCommand("yes", this);
	core -> registerCommand("no", this);

	core -> handleRawEvent(SIGNAL(evBanList(IrcParams)), this, SLOT(evBanList(IrcParams)));
	core -> handleRawEvent(SIGNAL(evEndBanList()), this, SLOT(evEndBanList()));
	core -> handleRawEvent(SIGNAL(packMode(IrcParams)), this, SLOT(packMode(IrcParams)));

	CPermissions* perm = (CPermissions*)core->getPlugin("permissions");
	perm -> registerCommand("vote", false);
	perm -> registerCommand("kick", false);
	perm -> registerCommand("ban", false);
	perm -> registerCommand("unban", false);
	perm -> registerCommand("banlist", true);
	perm -> registerCommand("votekick", true);
	perm -> registerCommand("voteban", true);
	perm -> registerCommand("voteunban", true);
	perm -> registerCommand("yes", true);
	perm -> registerCommand("no", true);
}

CKickBan::~CKickBan()
{
	if(vote != NULL)
		delete vote;
}

void CKickBan::executeCommand(QString command, QStringList params, QString addr, QString sender)
{
	CUsers* users = (CUsers*) core -> getPlugin("users");
	if(command == "vote")
	{
		if(params.length() < 1) { core -> sendMsg(addr, "Za mało parametrów!"); return; }
		if(params[0] == "pass_min")
		{
			if(params.length() < 2) { core -> sendMsg(addr, "Za mało parametrów!"); return; }
			settings -> SetDouble("vote_pass_min", params[1].toDouble());
			return;
		}
		if(params[0] == "timeout")
		{
			if(params.length() < 2) { core -> sendMsg(addr, "Za mało parametrów!"); return; }
			settings -> SetInt("vote_timeout", params[1].toInt());
			return;
		}
		if(params[0] == "min_votes")
		{
			if(params.length() < 2) { core -> sendMsg(addr, "Za mało parametrów!"); return; }
			settings -> SetInt("vote_min_votes", params[1].toInt());
			return;
		}
		return;
	}
	if(state == Waiting)
	{
		if(command == "kick")
		{
			if(params.length() < 1) { core -> sendMsg(addr, "Za mało parametrów!"); return; }
			if(params[0] == core -> getNick()) { core -> sendMsg(addr, "Nie będę kopał samego siebie ;)"); return; }
			if(users -> Find(params[0]) == -1) { core -> sendMsg(addr, "Nie ma takiego użytkownika!"); return; }

			core -> kickUser(params[0], (params.length() > 1) ? params[1] : "");
		}
		else if(command == "ban")
		{
			if(params.length() < 1) { core -> sendMsg(addr, "Za mało parametrów!"); return; }
			if(params[0] == core -> getNick()) { core -> sendMsg(addr, "Nie będę banował samego siebie ;)"); return; }
			int id = users -> Find(params[0]);
			if(id == -1) { core -> sendMsg(addr, "Nie ma takiego użytkownika."); return; }

			User u = (*users)[id];
			ban(u.nick, "*!*@" + u.mask);
			core -> kickUser(params[0], (params.length() > 1) ? params[1] : "");
		}
		else if(command == "unban")
		{
			if(params.length() < 1) { core -> sendMsg(addr, "Za mało parametrów!"); return; }
			if(! bans.contains(params[0])) { core -> sendMsg(addr, "Nie ma bana o takiej nazwie!"); return; }

			unban(params[0]);
		}
		else if(command == "banlist")
		{
			list_reply = addr;
			core -> channelMode("b", "");
		}

		/**** Votes ****/

		else if(command == "votekick")
		{
			if(params.length() < 1) { core -> sendMsg(addr, "Za mało parametrów!"); return; }
			if(params[0] == core -> getNick()) { core -> sendMsg(addr, "Nie będę kopał samego siebie ;)"); return; }
			if(users -> Find(params[0]) == -1) { core -> sendMsg(addr, "Nie ma takiego użytkownika!"); return; }

			voteTarget = params[0];
			int timeout = settings -> GetInt("vote_timeout");
			state = Voting;
			voteType = Kick;
			vote = new Vote;
			core -> sendMsgChannel("Głosowanie nad kickiem dla " + params[0] + ".");
			core -> sendMsgChannel("W ciągu następnych " + QString::number(timeout/1000) + " sekund wpisz \"!yes\" (głos za), \"!no\" (głos przeciw) albo \"!whatever\" (wstrzymanie się).");
			QTimer::singleShot(timeout, this, SLOT(endVote()));
		}
		else if(command == "voteban")
		{
			if(params.length() < 1) { core -> sendMsg(addr, "Za mało parametrów!"); return; }
			if(params[0] == core -> getNick()) { core -> sendMsg(addr, "Nie będę banował samego siebie ;)"); return; }
			int id = users -> Find(params[0]);
			if(id == -1) { core -> sendMsg(addr, "Nie ma takiego użytkownika!"); return; }

			voteTarget = params[0];
			voteMask = "*!*@" + (*users)[id].mask;
			int timeout = settings -> GetInt("vote_timeout");
			state = Voting;
			voteType = Ban;
			vote = new Vote;
			core -> sendMsgChannel("Głosowanie nad banem dla " + params[0] + ".");
			core -> sendMsgChannel("W ciągu następnych " + QString::number(timeout/1000) + " sekund wpisz \"!yes\" (głos za), \"!no\" (głos przeciw) albo \"!whatever\" (wstrzymanie się).");
			QTimer::singleShot(timeout, this, SLOT(endVote()));
		}
		else if(command == "voteunban")
		{
			if(params.length() < 1) { core -> sendMsg(addr, "Za mało parametrów!"); return; }
			if(! bans.contains(params[0])) { core -> sendMsg(addr, "Nie ma takiego bana!"); return; }

			voteTarget = params[0];
			int timeout = settings -> GetInt("vote_timeout");
			state = Voting;
			voteType = Unban;
			vote = new Vote;
			core -> sendMsgChannel("Głosowanie nad zdjęciem bana dla " + params[0] + ".");
			core -> sendMsgChannel("W ciągu następnych " + QString::number(timeout/1000) + " sekund wpisz \"!yes\" (głos za), \"!no\" (głos przeciw) albo \"!whatever\" (wstrzymanie się).");
			QTimer::singleShot(timeout, this, SLOT(endVote()));
		}
		else
		{
			core -> sendMsg(addr, "Ta komenda jest teraz nieaktywna.");
			return;
		}
	}
	else if(state == Voting)
	{
		if(command == "yes")
		{
			if(! vote -> voteFor(sender))
			{
				core -> sendMsg(addr, "Już głosowałeś/aś w tym głosowaniu!");
				return;
			}
			if(vote -> numVotes() == users -> presentUsers())
			{
				endVote();
				return;
			}
		}
		else if(command == "no")
		{
			if(! vote -> voteAgainst(sender))
			{
				core -> sendMsg(addr, "Już głosowałeś/aś w tym głosowaniu!");
				return;
			}
			if(vote -> numVotes() == users -> presentUsers())
			{
				endVote();
				return;
			}
		}
		else if(command == "whatever")
		{
			if(! vote -> voteNone(sender))
			{
				core -> sendMsg(addr, "Już głosowałeś/aś w tym głosowaniu!");
				return;
			}
			if(vote -> numVotes() == users -> presentUsers())
			{
				endVote();
				return;
			}
		}
		else
		{
			core -> sendMsg(addr, "Ta komenda jest teraz nieaktywna.");
			return;
		}
	}
}

void CKickBan::ban(QString nick, QString mask)
{
	bans[nick] = mask;
	core -> channelMode("+b", mask);
}

void CKickBan::unban(QString nick)
{
	core -> channelMode("-b", bans[nick]);
	bans.remove(nick);
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

void CKickBan::endVote()
{
	if(vote == NULL) return;

	if(vote -> numVotes() < settings -> GetInt("vote_min_votes"))
		core -> sendMsgChannel("Zbyt niska frekwencja.");
	else
	{
		if(vote -> resultFor() < settings -> GetDouble("vote_pass_min"))
			core -> sendMsgChannel("Wynik głosowania: przeciw.");
		else
		{
			core -> sendMsgChannel("Wynik głosowania: za.");

			switch(voteType)
			{
			case Kick:
				core -> kickUser(voteTarget, "Lud przemówił.");
				break;
			case Ban:
				ban(voteTarget, voteMask);
				core -> kickUser(voteTarget, "Lud przemówił.");
				break;
			case Unban:
				unban(voteTarget);
				break;
			}
		}
	}

	state = Waiting;
	delete vote;
	vote = NULL;
}
