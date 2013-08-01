#include "randomchat.h"
#include <QDateTime>
#include <QFile>
#include <stdlib.h>

Slowo::Slowo(QString s)
{
	slowo=s;
	wagi.clear();
	slowa.clear();
}

Slowo::~Slowo()
{
}

void Slowo::DodajLink(qint32 waga, qint32 s)
{
	wagi.push_back(waga);
	slowa.push_back(s);
}

qint32 Slowo::suma()
{
	qint32 i,suma;
	suma=0;
	for(i=0; i<wagi.size(); i++)
		suma += wagi[i];
	return suma;
}

QString Slowo::S()
{
	return slowo;
}

qint32 Slowo::LosujLink()
{
	qint32 x,i;
	x = rand()%suma();
	i = 0;
	do
	{
		x -= wagi[i];
		i++;
	}
	while(x > 0);
	i--;
	return slowa[i];
}

void Slowo::DodajWage(qint32 x)
{
	qint32 i;
	for(i=0; i<slowa.size(); i++)
		if(slowa[i]==x)
		{
			wagi[i]++;
			break;
		}
	if(i == slowa.size())
		DodajLink(1,x);
}

//----------------------------------------------------------

CRandomChat::CRandomChat(CBotCore* c, CBotSettings* s)
	: CBotPlugin(c, s)
{
	plik = "randomchat.dat";
	enabled = settings->GetBool("enabled_random");
	stfu_active = false;
	
	core->handleEvent(SIGNAL(ircMessage(QString, QString, QString)), this, SLOT(ircMessage(QString, QString, QString)));
	core->registerCommand("random", this);
	core->registerCommand("gadaj", this);
	core->registerCommand("stfu", this);
	
	srand(QDateTime::currentDateTime().toTime_t());
	
	wczytaj();
	modified = false;
	
	connect(&tAutosave, SIGNAL(timeout()), this, SLOT(autosave()));
	tAutosave.setSingleShot(false);
	tAutosave.start(settings->GetInt("random_autosaveDelay"));
}

void CRandomChat::wczytaj()
{
	QFile f(plik);
	
	qint32 i,j,n,n_wag,len,x,waga;
	quint16* pom;
	
	if(!f.open(QIODevice::ReadOnly))
	{
		slowa.push_back(new Slowo(""));
		return;
	}
	
	QDataStream fin(&f);
	fin.setByteOrder(QDataStream::LittleEndian);
	
	fin >> n;
	
	for(i=0; i<n; i++)
	{
		fin >> len;
		pom = new quint16[len];
		for(j=0; j<len; j++)
			fin >> pom[j];	//wczytaj słowo
		slowa.push_back(new Slowo(QString::fromUtf16(pom)));
		
		fin >> n_wag; 	//wczytaj ile polaczen ma to slowo
		for(j=0; j<n_wag; j++)
		{
			fin >> x;		//wczytaj polaczenia i ich wagi
			fin >> waga;
			slowa[i]->DodajLink(waga,x);
		}
		delete[] pom;
	}
	
	f.close();
}

void CRandomChat::zapisz()
{
	QFile f(plik);
	quint32 i,j,len,n_wag,n;
	
	f.open(QIODevice::WriteOnly);
	n = slowa.size();
	
	QDataStream fout(&f);
	fout.setByteOrder(QDataStream::LittleEndian);
	
	fout << n;
	
	for(i=0; i<n; i++)
	{
		len = slowa[i]->S().length()+1;
		fout << len;
		for(j=0; j<len; j++)
			fout << slowa[i]->S().utf16()[j];
		
		n_wag = slowa[i]->wagi.size();
		fout << n_wag;
		for(j=0; j<n_wag; j++)
		{
			fout << slowa[i]->slowa[j];
			fout << slowa[i]->wagi[j];
		}
	}
	f.close();
}

CRandomChat::~CRandomChat()
{
	tAutosave.stop();
	zapisz();
}

qint32 CRandomChat::Find(QString s)
{
	qint32 i;
	for(i=1; i<slowa.size(); i++)
		if(slowa[i]->S() == s) return i;
		return -1;
}

void CRandomChat::Analizuj(QString s)
{
	int i;
	int x;
	QStringList wyr;
	
	s = zlikwiduj_polskie(s);
	s = zlikwiduj_kontrolne(s);
	//s=zamien_na_spacje(s);
	s = s.toLower();
	wyr = s.split(QRegExp("\\s+"), QString::SkipEmptyParts);
	
	for(i=0; i<wyr.length(); i++)	//dodaj slowa, ktorych jeszcze nie ma
	{
		x = Find(wyr[i]);
		if(x == -1)
		{
			slowa.push_back(new Slowo(wyr[i]));
		}
	}
	
	if(wyr.length()>0)
		slowa[0]->DodajWage(Find(wyr[0]));
	
	for(i=0; i<wyr.length()-1; i++)
	{
		slowa[Find(wyr[i])]->DodajWage(Find(wyr[i+1]));
	}
	
	if(wyr.length()>0)
	{
		x = Find(wyr[wyr.length()-1]);
		slowa[x]->DodajWage(-1);
	}
	
	modified = true;
}

QString CRandomChat::Generuj()
{
	qint32 i = slowa[0]->LosujLink();
	QString ret = "";
	int j=0;
	
	do {
		ret += slowa[i]->S() + " ";
		i = slowa[i]->LosujLink();
		j++;
	} while((i!=-1)&&(j<100));
	
	return ret;
}

void CRandomChat::stfu()
{
	if(stfu_active) return;
	
	stfu_active = true;
	QTimer::singleShot(300000, this, SLOT(endSTFU()));
	
	core -> sendMsgChannel("Ok, zamknę się na 5 minut, " + settings->GetString("wyzwisko") + " jeden.");
}

void CRandomChat::endSTFU()
{
	stfu_active = false;
}

void CRandomChat::autosave()
{
	if(modified)
		zapisz();
	modified = false;
}

void CRandomChat::executeCommand(QString command, QStringList params, QString addr, QString sender)
{
	if(command == "stfu")
		stfu();
	if(command == "random" && core->master(sender))
	{
		if(params[0] == "enable")
		{
			core -> sendMsg(addr,"Bredzenie włączone.");
			enabled = true;
		}
		if(params[0] == "disable")
		{
			core -> sendMsg(addr,"Bredzenie wyłączone.");
			enabled = false;
		}
	}
	if(command == "gadaj")
		core -> sendMsg(addr, Generuj());
}

void CRandomChat::ircMessage(QString, QString addr, QString msg)
{
	Analizuj(msg);
	
	if(enabled && !stfu_active)
	{
		int x = rand()%100+1;
		if(x > settings->GetInt("answer_probability")) return;
		core -> sendMsg(addr, Generuj());
	}
}

//---------------------------------------------------------------------------------

QString zlikwiduj_polskie(QString wiadomosc)
{
	int i;
	for(i=0; i<wiadomosc.length(); i++) //zmień polskie litery
	{
		if(wiadomosc[i] == L'Ą') wiadomosc[i]='A';
		if(wiadomosc[i] == L'ą') wiadomosc[i]='a';
		if(wiadomosc[i] == L'Ć') wiadomosc[i]='C';
		if(wiadomosc[i] == L'ć') wiadomosc[i]='c';
		if(wiadomosc[i] == L'Ę') wiadomosc[i]='E';
		if(wiadomosc[i] == L'ę') wiadomosc[i]='e';
		if(wiadomosc[i] == L'Ł') wiadomosc[i]='L';
		if(wiadomosc[i] == L'ł') wiadomosc[i]='l';
		if(wiadomosc[i] == L'Ń') wiadomosc[i]='N';
		if(wiadomosc[i] == L'ń') wiadomosc[i]='n';
		if(wiadomosc[i] == L'Ó') wiadomosc[i]='O';
		if(wiadomosc[i] == L'ó') wiadomosc[i]='o';
		if(wiadomosc[i] == L'Ś') wiadomosc[i]='S';
		if(wiadomosc[i] == L'ś') wiadomosc[i]='s';
		if(wiadomosc[i] == L'Ź') wiadomosc[i]='Z';
		if(wiadomosc[i] == L'ź') wiadomosc[i]='z';
		if(wiadomosc[i] == L'Ż') wiadomosc[i]='Z';
		if(wiadomosc[i] == L'ż') wiadomosc[i]='z';
	}
	return wiadomosc;
}

QString zlikwiduj_kontrolne(QString wiadomosc)
{
	QChar kontrolne[3]={QChar(2),QChar(3),QChar(255)};
	int n=3;
	
	int i;
	int a;
	for(i=0; i<n; i++)
	{
		a = wiadomosc.indexOf(kontrolne[i]);
		while(a != -1)
		{
			wiadomosc.remove(a,1);
			a = wiadomosc.indexOf(kontrolne[i]);
		}
	}
	return wiadomosc;
}
