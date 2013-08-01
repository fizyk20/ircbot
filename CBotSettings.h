#ifndef __BOTSETTINGS__
#define __BOTSETTINGS__

#include <QMap>
#include <QString>
#include <QFile>
#include <QTextStream>

class CBotSettings : public QObject
{
Q_OBJECT
	QString nazwa_pliku;
	QMap<QString,QString> values;
public:
	CBotSettings(QString);
	~CBotSettings();

	void Save();

	double GetDouble(QString);
	int GetInt(QString);
	QString GetString(QString);
	bool GetBool(QString);

	void SetDouble(QString,double);
	void SetInt(QString,int);
	void SetString(QString, QString);
	void SetBool(QString, bool);
};

bool toBool(QString str);

#endif