#ifndef __ROLL_H__
#define __ROLL_H__

#include "CBotCore.h"

class CRoll : public CBotPlugin
{
Q_OBJECT
public:
	CRoll(CBotCore*, CBotSettings*);
	~CRoll();
	
	void executeCommand(QString command, QStringList params, QString addr, QString);
};

#endif
