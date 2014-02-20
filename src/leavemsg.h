/*
 * leavemsg.h
 *
 *  Created on: 24 gru 2013
 *      Author: bartek
 */

#ifndef LEAVEMSG_H_
#define LEAVEMSG_H_

#include "CBotCore.h"
#include <QMap>

class CLeaveMsg : public CBotPlugin
{
Q_OBJECT
	QMap<QString, QString> msgs;

	void load();
	void save();
public:
	CLeaveMsg(CBotCore*, CBotSettings*);
	~CLeaveMsg();

	void executeCommand(QString, QStringList, QString, QString);

public slots:
	void userAuthed(QString, QString);
};

#endif /* LEAVEMSG_H_ */
