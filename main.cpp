#include <QtCore/QCoreApplication>
#include <QTextCodec>
#include "CBotCore.h"

int main(int argc,char** argv)
{
	QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
	QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
	QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
	
	QCoreApplication app(argc,argv);
	CBotCore *o = new CBotCore(&app);
	return app.exec();
} 
