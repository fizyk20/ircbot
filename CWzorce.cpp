#include "CWzorce.h"

CWzorce::CWzorce(CBotCore* c, CBotSettings* s)
	: CBotPlugin(c, s)
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
	
	core -> handleEvent(SIGNAL(ircMessage(QString, QString, QString)), this, SLOT(ircMessage(QString, QString, QString)));
}

CWzorce::~CWzorce()
{
}

void CWzorce::executeCommand(QString, QStringList, QString, QString)
{
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
