#ifndef __CWZORCE_H__
#define __CWZORCE_H__

#include "CBotCore.h"

class CWzorce : public CBotPlugin
{
Q_OBJECT
	QMap<QString,QString> wzorce;
	
public:
	CWzorce(CBotCore*, CBotSettings*);
	~CWzorce();
	
	void executeCommand(QString, QStringList, QString, QString);
	
public slots:
	void ircMessage(QString sender, QString addr, QString msg);
};

#endif
