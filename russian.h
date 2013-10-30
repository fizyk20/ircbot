#ifndef __RUSSIAN__
#define __RUSSIAN__

#include "CBotCore.h"
#include <QTimer>
#include <QThread>

class CRussian;

class RussianThread : public QThread
{
Q_OBJECT
	QString player;
	CRussian* russian;
	CBotCore* core;
	bool shoot;
public:
	RussianThread(CRussian* r, CBotCore* c, QString p, bool s);
	~RussianThread();
	
protected:
	void run();
};

struct Dead
{
	QString nick;
	QString mask;
};

class CRussian : public CBotPlugin
{
Q_OBJECT
	int naboj;
	QString player;
	bool enabled;
	
	RussianThread* thread;
	QList<Dead> dead;
public:
	CRussian(CBotCore*, CBotSettings*);
	~CRussian();
	
	void executeCommand(QString command, QStringList params, QString addr, QString sender);
	void addDead(Dead);
	
public slots:
	void unban();
};

#endif
