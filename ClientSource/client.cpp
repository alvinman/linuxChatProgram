#include "client.h"
#include "ui_client.h"
#include "receivethread.h"

Client::Client(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Client)
{
    ui->setupUi(this);
}

Client::~Client()
{
    ::close (connect_sd);
    delete ui;
}

void Client::on_bConnect_clicked(){

    // int connect_sd;
    int port;
    struct sockaddr_in server;
    struct hostent *hp;
    std::string hostnameStr;
    char* hostname;

    //get server ip
    hostnameStr = Client::getServerIP().toStdString();
    hostname = new char [hostnameStr.size()+1];
    strcpy(hostname, hostnameStr.c_str());

    //get server port
    port = Client::getServerPort().toInt();

    //create tcp socket
    if ((connect_sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        perror("Cannot Create Socket!");

    //setup address struct
    setupAddrStruct(server, hp, hostname, port);

    // Connecting to the server
    // need to call connect with the :: prefix, to prevent it getting mixed
    // up with the Qt version of connect
    if (::connect (connect_sd, (struct sockaddr *)&server, sizeof(server)) == -1)
    {
        fprintf(stderr, "Can't connect to server\n");
        perror("connect");
        exit(1);
    }

    ui->tvStatus->setText("Connected");

    //create thread to receive messages
    QThread* receiveThread = new QThread;
    ReceiveThread* receiveWorker = new ReceiveThread(connect_sd);
    receiveWorker->moveToThread(receiveThread);
    connect(receiveWorker, SIGNAL(updateChatBox(QString)), this, SLOT(updateChat(QString)));
//    connect(receiveWorker, SIGNAL(error(QString)), this, SLOT(errorString(QString)));
    connect(receiveThread, SIGNAL(started()), receiveWorker, SLOT(process()));
    connect(receiveWorker, SIGNAL(finished()), receiveThread, SLOT(quit()));
    connect(receiveWorker, SIGNAL(finished()), receiveWorker, SLOT(deleteLater()));
    connect(receiveThread, SIGNAL(finished()), receiveThread, SLOT(deleteLater()));
    receiveThread->start();

}

void Client::on_bSendMessage_clicked(){
    QString message = ui->etMessage->text();
    std::string messageStr = message.toStdString();
    char* messageChar = new char [messageStr.size()+1];
    strcpy(messageChar, messageStr.c_str());

    //append message to the chat window
    ui->dtMessageHistory->append(message);

    //send message to the server
    std::thread sendThread(sendMessage, std::ref(connect_sd), std::ref(messageChar));
    sendThread.join();
}

QString Client::getServerIP(){
    return ui->etIP->text();
}

QString Client::getServerPort(){
    return ui->etPort->text();
}

void Client::updateChat(QString message){
    ui->dtMessageHistory->append(message);
}
