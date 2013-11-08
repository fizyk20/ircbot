#include "badwords.h"
#include "CUsers.h"

Badwords::Badwords(CBotCore* c, CBotSettings* s)
	: CBotPlugin(c, s)
{
	load();

	core -> handleEvent(SIGNAL(ircMessage(QString, QString, QString)), this, SLOT(ircMessage(QString, QString, QString)));
	core -> registerCommand("badwords", this);
}

Badwords::~Badwords()
{
}

void Badwords::load()
{
	QFile file("badwords.ini");
	file.open(QIODevice::ReadOnly | QIODevice::Text);

	QTextStream in(&file);

	while (!in.atEnd())
	{
		QString badword = in.readLine();
		badwords.push_back(badword);
	}

	file.close();
}

void Badwords::executeCommand(QString command, QStringList params, QString addr, QString sender)
{
	if(params.length() < 1) return;
	if(!core -> master(sender)) return;

	if(params[0] == "enable")
	{
		core -> sendMsgChannel("Filtr Badwords włączony.");
		settings -> SetBool("enabled_badwords", true);
	}
	if(params[0] == "disable")
	{
		core -> sendMsgChannel("Filtr Badwords wyłączony.");
		settings -> SetBool("enabled_badwords", false);
	}
	if(params[0] == "kick_ops" && params.length() > 1)
	{
		if(params[1] == "on")
		{
			settings -> SetBool("enabled_kick_ops", true);
			core -> sendMsgChannel("Kopanie operatorów włączone.");
		}
		if(params[1] == "off")
		{
			settings -> SetBool("enabled_kick_ops", false);
			core -> sendMsgChannel("Kopanie operatorów wyłączone.");
		}
	}
	if(params[0] == "reload")
	{
		badwords.clear();
		load();
	}
}

void Badwords::ircMessage(QString sender, QString addr, QString msg)
{
	CUsers* users = (CUsers*) core -> getPlugin("users");
	if(users == NULL) return;
	int id = users -> Find(sender);
	if(id < 0) return;
	User u = (*users)[id];

	if(settings -> GetBool("enabled_badwords") && ((!(u.status & ST_HOP) && !(u.status & ST_OP)) || settings -> GetBool("enabled_kick_ops")))
	{
		int i;
		for(i = 0; i < badwords.size(); i++)
		{
			QRegExp reg = QRegExp("(" + badwords[i] + ")", Qt::CaseInsensitive);
			if(msg.contains(reg))
			{
				BadwordFound(id, sender, u.mask, reg.cap(1));
				return;
			}
		}
	}
}

void Badwords::BadwordFound(int id, QString nick, QString mask, QString badword)
{
	int i;
	for(i=0; i<warns.size(); i++)
		if(warns[i].user_id == id) break;
	if(i < warns.size())
	{
		warns[i].number++;
		if(warns[i].number >= settings->GetInt("max_warnings"))
		{
			core -> channelMode("+b", "*!*@" + mask);
			warns.erase(warns.begin()+i);
		}
	}
	else
	{
		if(1 >= settings->GetInt("max_warnings"))
		{
			core -> channelMode("+b", "*!*@" + mask);
		}
		else
		{
			Warning w;
			w.user_id = id;
			w.number = 1;
			warns.push_back(w);
		}
	}
	core -> kickUser(nick, "Uważaj co piszesz! (" + badword + ")");
}
