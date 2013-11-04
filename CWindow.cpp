#include "CWindow.h"
#include <iostream>
using namespace std;

CWindow::CWindow()
{
}

CWindow::~CWindow()
{
}

void CWindow::Log(QString txt)
{
	cout << txt;
}

void CWindow::clickedConnect()
{
	emit botConnect();
}

void CWindow::clickedDisconnect()
{
	emit botDisconnect();
}
