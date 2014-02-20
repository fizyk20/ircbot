#include "CEightball.h"

CEightball::CEightball(CBotCore* c, CBotSettings* s)
	: CBotPlugin(c, s)
{
	load();
	core -> registerCommand("eightball", this);
}

CEightball::~CEightball()
{
}

void CEightball::load()
{
	QFile file("eightball.ini");
	file.open(QIODevice::ReadOnly | QIODevice::Text);
	
	QTextStream in(&file);
	while (!in.atEnd()) 
	{
		QString answer = in.readLine();
		
		answers.push_back(answer);
	}
	
	file.close();
}

void CEightball::executeCommand(QString command, QStringList params, QString addr, QString sender)
{
	if(params.length() < 1)
	{
		core -> sendMsg(addr, "A gdzie pytanie?");
		return;
	}
	
	if(params[0] == "reload")
	{
		answers.clear();
		load();
		return;
	}

	unsigned n = rand() % answers.size();
	QString ans = answers[n];
	ans.replace("%s", sender);
	
	core -> sendMsg(addr, ans);
}
