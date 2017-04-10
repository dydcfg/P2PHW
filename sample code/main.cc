
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
        rumorTimer = new QTimer(this);
        connect(rumorTimer, SIGNAL(timeout()), this, SLOT(rumorTimeout()));
        antiEntropyTimer = new QTimer(this);
        connect(antiEntropyTimer, SIGNAL(timeout()), this, SLOT(entropyTimeout()));
        antiEntropyTimer->start(3000);
}

void ChatDialog::entropyTimeout(){
    qDebug()<<"Anti-Entropy Timeout";
    antiEntropyTimer->start(3000);
    int coin = qrand()%2;
    if (socket->myPortMin == socket->myPort)
        coin = 1;
    if (socket->myPortMax == socket->myPort)
        coin = 0;

    if (coin == 0)
        socket->status(socket->myPort - 1);
    else
        socket->status(socket->myPort + 1);
}


void ChatDialog::rumorTimeout(){
    qDebug()<<"Rumor Timeout";
    /*
    rumorTimer->start(1000);
    int coin = qrand()%2;
    if (socket->myPortMin == socket->myPort)
        coin = 1;
    if (socket->myPortMax == socket->myPort)
        coin = 0;

    if (coin == 0)
        socket->rumor(socket->lastOrigin, socket->lastIdx, socket->myPort - 1);
    else
        socket->rumor(socket->lastOrigin, socket->lastIdx, socket->myPort + 1);
    */
    return;
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

        qDebug() << msgMap;
        //Status
        if (msgMap.contains("Want")){
            qDebug() << "Status Received!";
            qDebug() << "Message from: " << sender.toString();
	    qDebug() << "Message port: " << senderPort;
            QMap<QString, QVariant> senderSeqDict = msgMap["Want"].toMap();
                rumorTimer->stop();
                for(QVariantMap::const_iterator iter = socket->seqDict.begin(); iter != socket->seqDict.end(); ++iter) {
                    if (!senderSeqDict.contains(iter.key()) || senderSeqDict[iter.key()].toUInt()<iter.value().toUInt() ){
                        socket->lastOrigin = iter.key();
                        socket->lastPort = 1;
                        if (!senderSeqDict.contains(iter.key())){
                            socket->lastIdx = 0;
                        }
                        else{
                            socket->lastIdx = senderSeqDict[iter.key()].toUInt();                       
                        }
                        rumorTimer->start(1000);
                        socket->rumor(iter.key(), socket->lastIdx, senderPort);
                        return;
                    }
                }
                
                for(QVariantMap::const_iterator iter = senderSeqDict.begin(); iter != senderSeqDict.end(); ++iter) {
                    if (!socket->seqDict.contains(iter.key()) || socket->seqDict[iter.key()].toUInt()<iter.value().toUInt() ){
                        socket->status(senderPort);
                        return;
                    }
                }
                
                if (socket->isRumoring == 0)
                    return;
                //Nothing to be updated. Filp a coin.
                //New rumor
                if (qrand()%2 == 0){
                    if (senderPort == socket->myPort+1)
                        socket->rumor(socket->lastOrigin, socket->lastIdx, socket->myPort - 1);
                    else
                        socket->rumor(socket->lastOrigin, socket->lastIdx, socket->myPort + 1);
                }
                //Stop
                else{
                    socket->isRumoring = 0;
                    return;
                }
        }
        //Rumor
        else{
            antiEntropyTimer->stop();
            antiEntropyTimer->start(3000);
            qDebug() << "Rumor Received!";
	    qDebug() << msgMap["ChatText"].toString();
	    QString txt = msgMap["ChatText"].toString();
	    QString origin = msgMap["Origin"].toString();
	    quint32 seqNum = msgMap["SeqNo"].toUInt();
	    qDebug() << "Message from: " << sender.toString();
	    qDebug() << "Seq: " << seqNum;
	    qDebug() << "Message port: " << senderPort;
            if (!socket->seqDict.contains(origin)){
                socket->seqDict.insert(origin, 0);
                QVector<QString> t;
    	        socket->messageDict.insert(origin, t);
            }
            if (socket->seqDict[origin].toUInt() == seqNum){
                textview->append(txt);
                socket->messageDict[origin].append(txt);
                socket->seqDict[origin] = seqNum+1;
                int coin = qrand()%2;
                if (socket->myPortMin == socket->myPort)
                    coin = 1;
                if (socket->myPortMax == socket->myPort)
                    coin = 0;
                if (coin == 1){
                    rumorTimer->start(1000);
                    socket->rumor(origin, seqNum, socket->myPort+1);
                }
                else{
                    rumorTimer->start(1000);
                    socket->rumor(origin, seqNum, socket->myPort-1);
                }
            }

            socket->status(senderPort);
	}
       
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

        if (!socket->seqDict.contains(socket->id)){
    	    socket->seqDict.insert(socket->id, 0);
            QVector<QString> t;
    	    socket->messageDict.insert(socket->id, t);
        }

        socket->seqDict[socket->id] = socket->seqDict[socket->id].toUInt() + 1;
        socket->messageDict[socket->id].append(textline->text());
        
        //QVariantMap msgMap;
        //QByteArray datagram;

        //msgMap.insert("ChatText", textline->text());
        
        //QDataStream out(&datagram, QIODevice::WriteOnly);
        //out << msgMap;

        //for (int p = socket->getPortMin(); p <= socket->getPortMax(); p++) {
        //	socket->writeDatagram(datagram, QHostAddress::LocalHost, p);
        //}
        
	// Clear the textline to get ready for the next input message.
        int coin = qrand()%2;
        if (socket->myPortMin == socket->myPort)
            coin = 1;
        if (socket->myPortMax == socket->myPort)
            coin = 0;
        if (coin == 1){
            rumorTimer->start(1000);
            socket->rumor(socket->id, socket->seqDict[socket->id].toUInt()-1, socket->myPort+1);
        }
        else{
            rumorTimer->start(1000);
            socket->rumor(socket->id, socket->seqDict[socket->id].toUInt()-1, socket->myPort-1);
        }
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
	myPortMax = myPortMin + 2;
        lastPort = 0;
        isRumoring = 0;
}

bool NetSocket::bind()
{
	// Try to bind to each of the range myPortMin..myPortMax in turn.
	for (int p = myPortMin; p <= myPortMax; p++) {
		if (QUdpSocket::bind(p)) {
			qDebug() << "bound to UDP port " << p;
			myPort = p;
			id = QString::number(qrand());
	        id.append(QString::number(myPort));
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

void NetSocket::rumor(QString origin, quint32 idx, int rcvPort){
    qDebug() << rcvPort;
    isRumoring = 1;
    QVariantMap msgMap;
    QByteArray datagram;

    msgMap.insert("ChatText", messageDict[origin][idx]);
    msgMap.insert("Origin", origin);
    msgMap.insert("SeqNo", idx);
    QDataStream out(&datagram, QIODevice::WriteOnly);
    out << msgMap;

    lastPort = 1;
    lastOrigin = origin;
    lastIdx = idx;    
    writeDatagram(datagram, QHostAddress::LocalHost, rcvPort);

    //for (int p = socket->getPortMin(); p <= socket->getPortMax(); p++) {
    //    socket->writeDatagram(datagram, QHostAddress::LocalHost, p);
    //}

}

void NetSocket::status(int rcvPort){
    QVariantMap msgMap;
    QByteArray datagram;
    msgMap.insert("Want", seqDict);
    QDataStream out(&datagram, QIODevice::WriteOnly);
    out << msgMap;
    writeDatagram(datagram, QHostAddress::LocalHost, rcvPort);
    qDebug() << "status sent!";
}


int main(int argc, char **argv)
{
	// Initialize Qt toolkit
	QApplication app(argc, argv);
	// Create an initial chat dialog window
	ChatDialog dialog;
	dialog.show();
	// Create a UDP network socket
	//NetSocket sock;
	//if (!sock.bind())
	//	exit(1);

	// Enter the Qt main loop; everything else is event driven
	return app.exec();
}

