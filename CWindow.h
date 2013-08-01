#ifndef __CWINDOW__
#define __CWINDOW__

#include <QMainWindow>
#include <QTextEdit>
#include <QPushButton>

class CWindow : public QMainWindow
{
Q_OBJECT
	QTextEdit* mainEdit;
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
