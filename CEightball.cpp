#include "CEightball.h"

CEightball::CEightball(CBotCore* c, CBotSettings* s)
	: CBotPlugin(c, s)
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
	
	core -> registerCommand("eightball", this);
}

CEightball::~CEightball()
{
}

void CEightball::executeCommand(QString command, QStringList params, QString addr, QString sender)
{
	if(params.length() < 1)
	{
		core -> sendMsg(addr, "A gdzie pytanie?");
		return;
	}
	
	unsigned n = rand() % answers.size();
	QString ans = answers[n];
	ans.replace("%s", sender);
	
	core -> sendMsg(addr, ans);
}
