#ifndef __ANTIFLOOD__
#define __ANTIFLOOD__

#include "CBotCore.h"
#include <QDateTime>

struct Flood
{
	QString user;
	QDateTime last_message;
	int how_many;
	int kicks;
};

class AntiFlood : public CBotPlugin
{
Q_OBJECT
	QVector<Flood> floods;
public:
	AntiFlood(CBotCore*, CBotSettings*);
	~AntiFlood();

	int Find(QString user);

	void executeCommand(QString command, QStringList params, QString addr, QString sender);

public slots:
	void ircMessage(QString, QString, QString);
};

#endif
