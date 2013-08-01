#include "CBotSettings.h"

CBotSettings::CBotSettings(QString arg)
{
	nazwa_pliku = arg;

	QFile file(nazwa_pliku);
	if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return;
	
	QTextStream fin(&file);

	while(!fin.atEnd())
	{
		QString line = fin.readLine();

		int pos1 = line.indexOf('"');
		int pos2 = line.indexOf('"',pos1+1);
		int pos3 = line.indexOf('"',pos2+1);
		int pos4 = line.indexOf('"',pos3+1);

		if(pos1 == -1 || pos2 == -1 || pos3 == -1 || pos4 == -1)
			continue;

		QString name = line.mid(pos1+1,pos2-pos1-1);
		QString value = line.mid(pos3+1,pos4-pos3-1);

		values[name] = value;
	}

	file.close();
}

CBotSettings::~CBotSettings()
{
	Save();
}

void CBotSettings::Save()
{
	QFile file(nazwa_pliku);
	if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
		return;
	
	QTextStream fout(&file);

	QMap<QString,QString>::iterator it;

	for(it=values.begin(); it!=values.end(); it++)
	{
		fout << "\"" << it.key() << "\" \"" << it.value() << "\"" << endl;
	}

	file.close();
}

QString CBotSettings::GetString(QString name)
{
	QMap<QString,QString>::iterator it = values.find(name);

	if(it == values.end())
	{
		values[name] = tr("");
		return tr("");
	}
	return it.value();
}

double CBotSettings::GetDouble(QString name)
{
	QMap<QString,QString>::iterator it = values.find(name);

	if(it == values.end())
	{
		values[name] = tr("0.0");
		return 0.0;
	}
	return it.value().toDouble();
}

int CBotSettings::GetInt(QString name)
{
	QMap<QString,QString>::iterator it = values.find(name);

	if(it == values.end())
	{
		values[name] = tr("0");
		return 0;
	}
	return it.value().toInt();
}

bool CBotSettings::GetBool(QString name)
{	
	QMap<QString,QString>::iterator it = values.find(name);

	if(it == values.end())
	{
		values[name] = tr("false");
		return false;
	}
	return toBool(it.value());
}

void CBotSettings::SetString(QString name, QString val)
{
	values[name] = val;
}

void CBotSettings::SetDouble(QString name, double val)
{
	values[name] = QString::number(val);
}

void CBotSettings::SetInt(QString name, int val)
{
	values[name] = QString::number(val);
}

void CBotSettings::SetBool(QString name, bool val)
{
	values[name] = (val?tr("true"):tr("false"));
}

/***********************************************************************************/


bool toBool(QString str)
{
	if(str=="true") return true;
	return false;
}