#ifndef P2PAPP_MAIN_HH
#define P2PAPP_MAIN_HH

#include <QDialog>
#include <QTextEdit>
#include <QLineEdit>
#include <QUdpSocket>


class NetSocket : public QUdpSocket
{
	Q_OBJECT

public:
	NetSocket();

	// Bind this socket to a P2Papp-specific default port.
	bool bind();
	
	int getPortMax();
	int getPortMin();

	void rumor(QString origin, int rcvPort);
	void status(int rcvPort);

private:
	int myPortMin, myPortMax, myPort;
	QMap<QString, QVector<QString> > messageDict;
	QMap<QString, int> seqDict;
};

class ChatDialog : public QDialog
{
	Q_OBJECT

public:
	ChatDialog();

public slots:
	void gotReturnPressed();
	void receiveDatagram();

private:
	QTextEdit *textview;
	QLineEdit *textline;
	NetSocket *socket;
};

#endif // P2PAPP_MAIN_HH
