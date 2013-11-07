#ifndef CKICKBAN_H_
#define CKICKBAN_H_

#include "CBotCore.h"

class Vote
{
	int vFor;
	int vAgainst;
	int vNone;
public:
	Vote();
	~Vote();

	void voteFor();
	void voteAgainst();
	void voteNone();

	double resultFor();
	double resultAgainst();
	double resultNone();
};

class CKickBan : public CBotPlugin
{
Q_OBJECT
	enum{Waiting, Voting} state;

	QMap<QString, QString> bans;
	Vote* vote;
	int next_id;
	QString list_reply;
public:
	CKickBan(CBotCore*, CBotSettings*);
	~CKickBan();

	void executeCommand(QString, QStringList, QString, QString);

public slots:
	void packMode(IrcParams);
	void evBanList(IrcParams);
	void evEndBanList();
};

#endif
