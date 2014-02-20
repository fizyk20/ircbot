#include "roll.h"
#include <stdlib.h>
#include <QDateTime>

CRoll::CRoll(CBotCore* c, CBotSettings* s)
	: CBotPlugin(c, s)
{
	srand(QDateTime::currentDateTime().toTime_t());
	core->registerCommand("roll", this);
}

CRoll::~CRoll()
{
}

void CRoll::executeCommand(QString command, QStringList params, QString addr, QString)
{
	if(params.length()<1)
	{
		core->sendMsg(addr, "Brak parametru.");
		return;
	}
	if(params[0] == "potato")
	{
		core->sendMsg(addr, "Wylosowano: firetruck");
		return;
	}
	if(params[0] == "sieczewerset")
	{
		core->sendMsg(addr, "Wylosowano: bazylion");
		return;
	}
	
	bool ok_n = true, ok_r = true;
	unsigned n = 1, range;
	
	QStringList p = params[0].split("d", QString::SkipEmptyParts);
	
	if(p.size() == 1)
	{
		range = p[0].toUInt(&ok_r);
	}
	else
	{
		n = p[0].toUInt(&ok_n);
		range = p[1].toUInt(&ok_r);
	}
	
	if(!ok_n || !ok_r)
	{
		core->sendMsg(addr, "Na pewno podajesz liczbę?");
		return;
	}
	
	if(range < 2)
	{
		core->sendMsg(addr, "To by było trochę nudne losowanie...");
		return;
	}
	
	if(n > 50)
	{
		n = 50;
		core -> sendMsg(addr, "UWAGA: Obcięto liczbę rzutów do 50.");
	}
	
	unsigned d;
	QString msg = "Wylosowano: ";
	
	for(unsigned i = 0; i < n; i++)
	{
		d = (unsigned)(((double)rand()/RAND_MAX)*range)+1;
		if(i != n-1)
			msg += QString::number(d) + ", ";
		else
			msg += QString::number(d);
	}
	
	core->sendMsg(addr, msg);
}
