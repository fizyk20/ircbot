#ifndef __RANDOMCHAT_H__
#define __RANDOMCHAT_H__

#include "CBotCore.h"
#include <QTimer>

class CRandomChat;

class Slowo
{
	QString slowo;
	QVector<qint32> wagi;
	QVector<qint32> slowa;
	qint32 suma();
public:
	Slowo(QString);
	~Slowo();
	void DodajLink(qint32 waga,qint32 s);
	void DodajWage(qint32);
	QString S();
	qint32 LosujLink();
	friend class CRandomChat;
};

class CRandomChat : public CBotPlugin
{
Q_OBJECT
	QString plik;
	QVector<Slowo*> slowa;
	qint32 Find(QString s);
	
	void wczytaj();
	void zapisz();
	
	bool stfu_active;
	
	QTimer tAutosave;
	bool modified;
public:
	CRandomChat(CBotCore*, CBotSettings*);
	~CRandomChat();
	
	void executeCommand(QString command, QStringList params, QString addr, QString sender);
	
	QString Generuj();
	void Analizuj(QString s);
	void stfu();
	
	friend class Slowo;
	
public slots:
	void ircMessage(QString sender, QString addr, QString msg);
	void endSTFU();
	void autosave();
};

QString zlikwiduj_polskie(QString);
QString zlikwiduj_kontrolne(QString);

#endif
