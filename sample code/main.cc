
#include <unistd.h>

#include <QVBoxLayout>
#include <QApplication>
#include <QDebug>
#include <QUdpSocket>
#include <string>

#include "main.hh"

ChatDialog::ChatDialog()
{
	setWindowTitle("P2Papp");

	// Read-only text box where we display messages from everyone.
	// This widget expands both horizontally and vertically.
	textview = new QTextEdit(this);
	textview->setReadOnly(true);

	// Small text-entry box the user can enter messages.
	// This widget normally expands only horizontally,
	// leaving extra vertical space for the textview widget.
	//
	// You might change this into a read/write QTextEdit,
	// so that the user can easily enter multi-line messages.
	textline = new QLineEdit(this);

	// Lay out the widgets to appear in the main window.
	// For Qt widget and layout concepts see:
	// http://doc.qt.nokia.com/4.7-snapshot/widgets-and-layouts.html
	QVBoxLayout *layout = new QVBoxLayout();
	layout->addWidget(textview);
	layout->addWidget(textline);
	setLayout(layout);
	
	socket = new NetSocket();
	socket->bind();
	// Register a callback on the textline's returnPressed signal
	// so that we can send the message entered by the user.
	connect(textline, SIGNAL(returnPressed()),
		this, SLOT(gotReturnPressed()));
	
	connect(socket, SIGNAL(readyRead()), this, SLOT(receiveDatagram()));
}

void ChatDialog::receiveDatagram()
{
	QByteArray datagram;
	datagram.resize(socket->pendingDatagramSize());

	QHostAddress sender;
	quint16 senderPort;
	
	socket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
	QDataStream in(&datagram, QIODevice::ReadOnly);

        QVariantMap msgMap;
	in >> msgMap;
	qDebug() << msgMap["ChatText"];
	QString txt = msgMap["ChatText"].toString();

	qDebug() << "Message from: " << sender.toString();
	qDebug() << "Message port: " << senderPort;
	
	textview->append(txt);
}

void ChatDialog::gotReturnPressed()
{
        // NetSocket *socket;
	// socket = new NetSocket();
	// socket->bind();
	
	// Initially, just echo the string locally.
	// Insert some networking code here...
	qDebug() << "FIX: send message to other peers: " << textline->text();
	textview->append(textline->text());
        
        QVariantMap msgMap;
        QByteArray datagram;

        msgMap.insert("ChatText", textline->text());
        
        QDataStream out(&datagram, QIODevice::WriteOnly);
        out << msgMap;

        for (int p = socket->getPortMin(); p <= socket->getPortMax(); p++) {
        	socket->writeDatagram(datagram, QHostAddress::LocalHost, p);
        }
        
	// Clear the textline to get ready for the next input message.
	textline->clear();
}

NetSocket::NetSocket()
{
	// Pick a range of four UDP ports to try to allocate by default,
	// computed based on my Unix user ID.
	// This makes it trivial for up to four P2Papp instances per user
	// to find each other on the same host,
	// barring UDP port conflicts with other applications
	// (which are quite possible).
	// We use the range from 32768 to 49151 for this purpose.
	myPortMin = 32768 + (getuid() % 4096)*4;
	myPortMax = myPortMin + 3;
}

bool NetSocket::bind()
{
	// Try to bind to each of the range myPortMin..myPortMax in turn.
	for (int p = myPortMin; p <= myPortMax; p++) {
		if (QUdpSocket::bind(p)) {
			qDebug() << "bound to UDP port " << p;
			myPort = p;
			return true;
		}
	}

	qDebug() << "Oops, no ports in my default range " << myPortMin
		<< "-" << myPortMax << " available";
	return false;
}

int NetSocket::getPortMax()
{
	return myPortMax;
}

int NetSocket::getPortMin()
{
	return myPortMin;
}

void NetSocket::rumor(QString origin, int rcvPort){
	if (rcvPort == -1){
		int dice = qrand()%2;
		rcvPort = myPort;
		if (dice == 0){
			rcvPort --;
		}
		else{
			rcvPort ++;
		}
	}

	QVariantMap msgMap;
    QByteArray datagram;

    msgMap.insert("ChatText", textline->text());
        
    QDataStream out(&datagram, QIODevice::WriteOnly);
    out << msgMap;

    for (int p = socket->getPortMin(); p <= socket->getPortMax(); p++) {
        socket->writeDatagram(datagram, QHostAddress::LocalHost, p);
    }


}

void NetSocket::status(int rcvPort){

}


int main(int argc, char **argv)
{
	// Initialize Qt toolkit
	QApplication app(argc, argv);

	// Create an initial chat dialog window
	ChatDialog dialog;
	dialog.show();

	// Create a UDP network socket
	NetSocket sock;
	if (!sock.bind())
		exit(1);

	// Enter the Qt main loop; everything else is event driven
	return app.exec();
}

