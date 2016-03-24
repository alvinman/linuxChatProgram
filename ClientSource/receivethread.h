#ifndef RECEIVETHREAD_H
#define RECEIVETHREAD_H

#include <QWidget>
#include "clienthelper.h"

class ReceiveThread : public QObject
{
    Q_OBJECT

public:
    ReceiveThread(int socket);
    ~ReceiveThread();
    bool abort;

public slots:
    void process();

signals:
    void updateChatBox(QString username, QString message, QString type);
    void updateUserList(QVector<QString> userList);
    void finished();

private:
    int m_socket;
};

#endif // RECEIVETHREAD_H
