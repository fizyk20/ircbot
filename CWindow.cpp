#include "CWindow.h"

CWindow::CWindow()
{
	QPushButton *bConnect, *bDisconnect;
	
	mainEdit = new QTextEdit(this);
	bConnect = new QPushButton(tr("Połącz"), this);
	bDisconnect = new QPushButton(tr("Rozłącz"), this);
	
	resize(800,600);
	move(100,100);
	
	mainEdit -> move(10,10);
	mainEdit -> resize(640,540);
	mainEdit -> setReadOnly(true);
	
	bConnect -> move(660,10);
	bConnect -> resize(130,30);
	
	bDisconnect -> move(660,50);
	bDisconnect -> resize(130,30);
	
	connect(bConnect, SIGNAL(clicked()), this, SLOT(clickedConnect()));
	connect(bDisconnect, SIGNAL(clicked()), this, SLOT(clickedDisconnect()));
	
	show();
}

CWindow::~CWindow()
{
}

void CWindow::Log(QString txt)
{
	mainEdit -> append(txt);
}

void CWindow::clickedConnect()
{
	emit botConnect();
}

void CWindow::clickedDisconnect()
{
	emit botDisconnect();
}
