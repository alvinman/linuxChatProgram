#ifndef CLIENT_H
#define CLIENT_H

#include <QWidget>
#include <QThread>
#include "clienthelper.h"

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

public slots:
    void updateChat(QString message);

private:
    Ui::Client *ui;
    int connect_sd;

private slots:
    void on_bConnect_clicked();
    void on_bSendMessage_clicked();
};

#endif // CLIENT_H
