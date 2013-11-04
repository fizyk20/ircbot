#ifndef __CEIGHTBALL_H__
#define __CEIGHTBALL_H__

#include "CBotCore.h"

class CEightball : public CBotPlugin
{
Q_OBJECT
	QVector<QString> answers;
	void load();
public:
	CEightball(CBotCore*, CBotSettings*);
	~CEightball();
	
	void executeCommand(QString command, QStringList params, QString addr, QString sender);
};

#endif
