#ifndef CLIENT_H
#define CLIENT_H

#include <QWidget>
#include <QThread>
#include <QFile>
#include <QTextStream>
#include <QTime>
#include "clienthelper.h"
#include <QScrollBar>
#include <QToolBar>
#include "receivethread.h"

namespace Ui {
class Client;
}

class Client : public QWidget
{
    Q_OBJECT

public:
    explicit Client(QWidget *parent = 0);
    ~Client();
    QString getServerIP();
    QString getServerPort();
    QString getUsername();
    void exportChatToText();

public slots:
    void updateChat(QString username, QString message, QString type);
    void updateUsers(QVector<QString> userList);

private:
    Ui::Client *ui;
    int connect_sd;
    bool connected;
    void updateStatusMessage(QString message);
    void toggleInput(bool state);
    QThread* receiveThread;
    ReceiveThread* receiveWorker;

private slots:
    void on_bConnect_clicked();
    void on_bSendMessage_clicked();
    void on_bExport_clicked();
    void on_bDisconnect_clicked();

signals:
    void returnPressed();
    void finished();
};

#endif // CLIENT_H
