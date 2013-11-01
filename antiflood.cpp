#include "antiflood.h"
#include "CUsers.h"

AntiFlood::AntiFlood(CBotCore* c, CBotSettings* s)
	: CBotPlugin(c, s)
{
	enabled = settings -> GetBool("enabled_antiflood");
	interval = settings -> GetInt("antiflood_interval");
	number = settings -> GetInt("antiflood_number");

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

void AntiFlood::executeCommand(QString command, QStringList params, QString addr, QString sender)
{
	if(! core -> master(sender)) return;
	if(params.length() < 1) return;
	if(params[0] == "enable")
	{
		enabled = true;
		core -> sendMsg(addr, "Zabezpieczenie Anti-Flood włączone.");
	}
	if(params[0] == "disable")
	{
		enabled = true;
		core -> sendMsg(addr, "Zabezpieczenie Anti-Flood wyłączone.");
	}
	if(params[0] == "interval" && params.length() > 1)
	{
		interval = params[1].toInt();
		core -> sendMsg(addr, "Interwał Anti-Flood ustawiony na " + params[1] + " milisekund.");
	}
	if(params[0] == "number" && params.length() > 1)
	{
		number = params[1].toInt();
		core -> sendMsg(addr, "Liczba wiadomości Anti-Flood ustawiona na " + params[1] + ".");
	}
}

void AntiFlood::ircMessage(QString sender, QString ret_addr, QString msg)
{
	CUsers* users = (CUsers*) core -> getPlugin("users");
	if(enabled)
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
			if(floods[id].last_message.msecsTo(QDateTime::currentDateTime()) < interval)
				floods[id].how_many++;
			else
				floods[id].how_many = 1;
			floods[id].last_message = QDateTime::currentDateTime();
			if(floods[id].how_many >= number)
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
