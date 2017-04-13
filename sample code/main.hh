#ifndef P2PAPP_MAIN_HH
#define P2PAPP_MAIN_HH

#include <QDialog>
#include <QTextEdit>
#include <QLineEdit>
#include <QUdpSocket>
#include <QTimer>


class NetSocket : public QUdpSocket
{
    Q_OBJECT
    
public:
    QString id, lastOrigin;
    quint32 lastIdx;
    int	lastPort;
    int	isRumoring;
    NetSocket();
    /* Bind this socket to a P2Papp-specific default port. */
    bool bind();
    
    
    int getPortMax();
    
    
    int getPortMin();
    
    
    void rumor( QString origin, quint32 idx, int rcvPort );
    
    
    void status( int rcvPort );
    
    
    int					myPortMin, myPortMax, myPort;
    QMap<QString, QVector<QString> >	messageDict;
    QMap<QString, QVariant>			seqDict;
};

class ChatDialog : public QDialog
{
    Q_OBJECT
    
public:
    ChatDialog();
    
    public slots:
    void gotReturnPressed();
    
    
    void receiveDatagram();
    
    
    void rumorTimeout();
    
    
    void entropyTimeout();
    
    
private:
    QTextEdit	*textview;
    QLineEdit	*textline;
    NetSocket	*socket;
    QTimer		*rumorTimer, *antiEntropyTimer;
};

#endif /* P2PAPP_MAIN_HH */
