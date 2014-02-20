#ifndef __CWINDOW__
#define __CWINDOW__

#include <QtCore>

class CWindow : public QObject
{
Q_OBJECT
public:
	CWindow();
	~CWindow();
	
public slots:
	void Log(QString);
	void clickedConnect();
	void clickedDisconnect();
	
signals:
	void botConnect();
	void botDisconnect();
};

#endif
