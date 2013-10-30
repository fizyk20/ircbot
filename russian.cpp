#include "russian.h"
#include <stdlib.h>
#include <QDateTime>
#include "CUsers.h"

CRussian::CRussian(CBotCore* c, CBotSettings* s)
	: CBotPlugin(c, s)
{
	naboj = -1;
	enabled = settings->GetBool("enabled_russian");
	srand(QDateTime::currentDateTime().toTime_t());
	
	thread = NULL;
	core->registerCommand("russian", this);
}

CRussian::~CRussian()
{
	if(thread)
	{
		thread->terminate();
		delete thread;
	}
}

void CRussian::executeCommand(QString command, QStringList params, QString addr, QString sender)
{
	if(params.length() > 1 && core->master(sender))
	{
		if(params[1] == "enable")
		{
			enabled = true;
			core -> sendMsgChannel("Włączono rosyjską ruletkę.");
			naboj = -1;
			return;
		}
		if(params[1] == "disable")
		{
			enabled = false;
			core -> sendMsgChannel("Wyłączono rosyjską ruletkę.");
			return;
		}
		core -> sendMsgChannel("Sam się " + params[1] + "!");
		return;
	}
	
	if(!enabled)
	{
		core -> sendMsgChannel("Rosyjska ruletka jest wyłączona.");
		return;
	}
	
	if(naboj == -1)
	{
		core -> sendMsgChannel("Rewolwer przeładowany.");
		naboj = rand() % 6;
	}
	
	thread = new RussianThread(this, core, sender, naboj == 0);
	naboj--;
	thread -> start();
}

void CRussian::addDead(Dead d)
{
	dead.push_back(d);
}

void CRussian::unban()
{
	Dead d = dead.takeFirst();
	//core -> Mode();
}

RussianThread::RussianThread(CRussian* r, CBotCore* c, QString p, bool s)
{
	russian = r;
	core = c;
	player = p;
	shoot = s;
}

RussianThread::~RussianThread()
{
}

void RussianThread::run()
{
	core -> sendMsgChannel(player + " wolno naciska spust...");
	sleep(5);
	if(shoot)
	{
		CUsers* users = (CUsers*) core -> getPlugin("users");
		int n = users -> Find(player);
		if(n < 0 || !(*users)[n].present)
		{
			core -> sendMsgChannel(player + " najwyraźniej nie istnieje, więc nie mógł się zabić.");
			return;
		}
		User u = (*users)[n];
		core -> sendMsgChannel("Jeb!");
		//core -> Mode()
		//core -> Kick()
		Dead d;
		d.mask = u.mask;
		d.nick = player;
		russian -> addDead(d);
		QTimer::singleShot(20000, russian, SLOT(unban()));
	}
	else
	{
		core -> sendMsgChannel("Klik! Uff, tym razem Ci się udało.");
	}
}

