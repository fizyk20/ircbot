#include "CWindow.h"
#include <QTextStream>
#include <stdio.h>

CWindow::CWindow()
{
}

CWindow::~CWindow()
{
}

void CWindow::Log(QString txt)
{
	QTextStream qout(stdout);
	qout << txt << endl;
}

void CWindow::clickedConnect()
{
	emit botConnect();
}

void CWindow::clickedDisconnect()
{
	emit botDisconnect();
}
