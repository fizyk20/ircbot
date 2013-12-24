#include "antiflood.h"
#include "CUsers.h"

AntiFlood::AntiFlood(CBotCore* c, CBotSettings* s)
	: CBotPlugin(c, s)
{
	core -> registerCommand("antiflood", this);
	core -> handleEvent(SIGNAL(ircMessage(QString, QString, QString)), this, SLOT(ircMessage(QString, QString, QString)));
}

AntiFlood::~AntiFlood()
{
}

int AntiFlood::Find(QString user)
{
	int i;
	for(i = 0; i < floods.size(); i++)
		if(floods[i].user == user) return i;
	return -1;
}

void AntiFlood::executeCommand(QString, QStringList params, QString addr, QString sender)
{
	if(! core -> master(sender)) return;
	if(params.length() < 1) return;
	if(params[0] == "enable")
	{
		settings -> SetBool("enabled_antiflood", true);
		core -> sendMsg(addr, "Zabezpieczenie Anti-Flood włączone.");
	}
	if(params[0] == "disable")
	{
		settings -> SetBool("enabled_antiflood", false);
		core -> sendMsg(addr, "Zabezpieczenie Anti-Flood wyłączone.");
	}
	if(params[0] == "interval" && params.length() > 1)
	{
		settings -> SetInt("antiflood_interval", params[1].toInt());
		core -> sendMsg(addr, "Interwał Anti-Flood ustawiony na " + params[1] + " milisekund.");
	}
	if(params[0] == "number" && params.length() > 1)
	{
		settings -> SetInt("antiflood_number", params[1].toInt());
		core -> sendMsg(addr, "Liczba wiadomości Anti-Flood ustawiona na " + params[1] + ".");
	}
}

void AntiFlood::ircMessage(QString sender, QString, QString)
{
	CUsers* users = (CUsers*) core -> getPlugin("users");
	if(settings -> GetBool("enabled_antiflood"))
	{
		int id = Find(sender);
		if(id<0)
		{
			Flood f;
			f.user = sender;
			f.last_message = QDateTime::currentDateTime();
			f.how_many = 1;
			f.kicks = 0;
			floods.push_back(f);
		}
		else
		{
			if(floods[id].last_message.msecsTo(QDateTime::currentDateTime()) < settings -> GetInt("antiflood_interval"))
				floods[id].how_many++;
			else
				floods[id].how_many = 1;
			floods[id].last_message = QDateTime::currentDateTime();
			if(floods[id].how_many >= settings -> GetInt("antiflood_number"))
			{
				floods[id].kicks++;
				if(floods[id].kicks > settings -> GetInt("max_warnings"))
				{
					QString mask;
					int id2 = users -> Find(sender);
					if(id2>=0)
					{
						mask = (*users)[id2].mask;
						core -> channelMode("+b", mask);
					}
				}
				core -> kickUser(sender, "Przestań floodować!");
				floods[id].how_many = 1;
			}
		}
	}
}
