#include "CWzorce.h"

CWzorce::CWzorce(CBotCore* c, CBotSettings* s)
	: CBotPlugin(c, s)
{
	load();
	
	core -> handleEvent(SIGNAL(ircMessage(QString, QString, QString)), this, SLOT(ircMessage(QString, QString, QString)));
	core -> registerCommand("wzorce", this);
}

CWzorce::~CWzorce()
{
}

void CWzorce::load()
{
	QFile file("wzorce.ini");
	file.open(QIODevice::ReadOnly | QIODevice::Text);

	QTextStream in(&file);
	while (!in.atEnd())
	{
		QString regexp = in.readLine();
		QString answer = in.readLine();

 		wzorce[regexp] = answer;
	}

	file.close();
}

void CWzorce::executeCommand(QString command, QStringList params, QString, QString)
{
	if(params.length() < 1) return;

	if(params[0] == "reload")
	{
		wzorce.clear();
		load();
		return;
	}
}

void CWzorce::ircMessage(QString sender, QString addr, QString msg)
{
	QMap<QString, QString>::iterator it;
	
	for(it = wzorce.begin(); it != wzorce.end(); it++)
	{
		QString reg = it.key();
		QRegExp rx;
		if(reg.left(6) == "[case]")
		{
			reg = reg.mid(6);
			rx = QRegExp(reg);
		}
		else
			rx = QRegExp(reg, Qt::CaseInsensitive);
			
		QString answer = it.value();
		
		if(msg.contains(rx))
		{
			core -> sendMsg(addr, answer);
		}
	}
}
