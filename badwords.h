#ifndef __BADWORDS__
#define __BADWORDS__

#include "CBotCore.h"

struct Warning
{
	int user_id;
	int number;
};

class Badwords : public CBotPlugin
{
Q_OBJECT
	QVector<QString> badwords;
	QVector<Warning> warns;
public:
	Badwords(CBotCore*, CBotSettings*);
	~Badwords();

	void BadwordFound(int, QString, QString, QString);
	void executeCommand(QString command, QStringList params, QString addr, QString sender);

public slots:
	void ircMessage(QString sender, QString addr, QString msg);
};

#endif
