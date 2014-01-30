#ifndef CKICKBAN_H_
#define CKICKBAN_H_

#include "CBotCore.h"

// Settings:
// vote_pass_min - min. percent of votes for to pass
// vote_timeout - number of milliseconds for vote timeout
// vote_min_votes - min. number of votes to consider the result

class Vote
{
	int vFor;
	int vAgainst;
	int vNone;
	QVector<QString> voted;
public:
	Vote();
	~Vote();

	bool voteFor(QString);
	bool voteAgainst(QString);
	bool voteNone(QString);

	double resultFor();
	double resultAgainst();
	int numVotes();
};

class CKickBan : public CBotPlugin
{
Q_OBJECT
	enum{Waiting, Voting} state;

	QMap<QString, QString> bans;

	Vote* vote;
	enum{Kick, Ban, Unban} voteType;
	QString voteTarget, voteMask;

	int next_id;
	QString list_reply;
public:
	CKickBan(CBotCore*, CBotSettings*);
	~CKickBan();

	void executeCommand(QString, QStringList, QString, QString);

	void ban(QString, QString);
	void unban(QString);

public slots:
	void packMode(IrcParams);
	void evBanList(IrcParams);
	void evEndBanList();

	void endVote();
};

#endif
