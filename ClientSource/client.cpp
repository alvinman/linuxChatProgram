#include "client.h"
#include "ui_client.h"
#include "receivethread.h"

Client::Client(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Client)
{
    ui->setupUi(this);
    connect(ui->etMessage, SIGNAL(returnPressed()), ui->bSendMessage, SIGNAL(clicked()));
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

    //check username before allowing any connections
    QString username = ui->etUsername->text();
    if(username.isEmpty()){
        Client::updateStatusMessage("Please enter username");
        return;
    } else {
        Client::updateStatusMessage("");
    }

    //get server ip
    hostnameStr = Client::getServerIP().toStdString();
    hostname = new char [hostnameStr.size()+1];
    strcpy(hostname, hostnameStr.c_str());

    //get server port
    port = Client::getServerPort().toInt();
    if(port == 0){
        port = 7000;
    }

    //create tcp socket
    if ((connect_sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        perror("Cannot Create Socket!");

    //setup address struct
    if(setupAddrStruct(server, hp, hostname, port) == -1){
        Client::updateStatusMessage("Invalid IP");
        return;
    } else {
        Client::updateStatusMessage("");
    }

    // Connecting to the server
    // need to call connect with the :: prefix, to prevent it getting mixed
    // up with the Qt version of connect
    if (::connect (connect_sd, (struct sockaddr *)&server, sizeof(server)) == -1)
    {
        fprintf(stderr, "Can't connect to server\n");
        perror("connect");
        Client::updateStatusMessage("Can't connect to server");
        return;
    }

    // Send the username to the server on initial connect
    QString usernameMessage = "USERNAME: " + username;

    // convert qstring message into char * message for sending
    std::string messageStr = usernameMessage.toStdString();
    char* messageChar = new char [messageStr.size()+1];
    strcpy(messageChar, messageStr.c_str());

    // Send a message indicating username
    std::thread sendThread(sendMessage, std::ref(connect_sd), std::ref(messageChar));
    sendThread.join();

    ui->tvStatus->setText("Connected");

    //create thread to receive messages
    QThread* receiveThread = new QThread;
    ReceiveThread* receiveWorker = new ReceiveThread(connect_sd);
    receiveWorker->moveToThread(receiveThread);
    connect(receiveWorker, SIGNAL(updateChatBox(QString, QString)), this, SLOT(updateChat(QString, QString)));
    connect(receiveWorker, SIGNAL(updateUserList(QVector<QString>)), this, SLOT(updateUsers(QVector<QString>)));
    connect(receiveThread, SIGNAL(started()), receiveWorker, SLOT(process()));
    connect(receiveWorker, SIGNAL(finished()), receiveThread, SLOT(quit()));
    connect(receiveWorker, SIGNAL(finished()), receiveWorker, SLOT(deleteLater()));
    connect(receiveThread, SIGNAL(finished()), receiveThread, SLOT(deleteLater()));
    receiveThread->start();
}

void Client::on_bSendMessage_clicked(){
    //get message
    QString message = ui->etMessage->text();

    //get username
    QString username = ui->etUsername->text();

    //append message to the chat window
    Client::updateChat(username + ": ", message);

    //prepend username to message
    message = username + ": " + message;

    //prepend message tag to message
    message = "MESSAGE: " + message;

    // convert qstring message into char * message for sending
    std::string messageStr = message.toStdString();
    char* messageChar = new char [messageStr.size()+1];
    strcpy(messageChar, messageStr.c_str());

    // clear the message box
    ui->etMessage->clear();

    //send message to the server
    std::thread sendThread(sendMessage, std::ref(connect_sd), std::ref(messageChar));
    sendThread.join();
}

void Client::on_bExport_clicked(){
    Client::exportChatToText();
}

QString Client::getServerIP(){
    return ui->etIP->text();
}

QString Client::getServerPort(){
    return ui->etPort->text();
}

QString Client::getUsername(){
    return ui->etUsername->text();
}

void Client::updateChat(QString username, QString message){
    QString styledString="<span style=\" font-size:12pt; font-weight:600; color:#FF0c32;\" > ";
    styledString.append(username);
    styledString.append("</span>");
    styledString.append(message);
    ui->dtMessageHistory->insertHtml(styledString);
    ui->dtMessageHistory->append("\n");
}

void Client::updateUsers(QVector<QString> userList){
    ui->dtUserList->clear();
    for(auto& user : userList){
       ui->dtUserList->addItem(user);
    }
}

void Client::exportChatToText(){
    QString chatHistory = ui->dtMessageHistory->toPlainText();
    QFile file("chat_log.txt");
    if(file.open(QIODevice::WriteOnly)){
        QTextStream stream(&file);
        stream << chatHistory << endl;
        Client::updateExportMessage("Chat log exported");
    } else {
        Client::updateExportMessage("Export error");
        return;
    }
    file.close();
}

void Client::updateStatusMessage(QString message){
    ui->statusMessage->setText(message);
}

void Client::updateExportMessage(QString message){
    ui->exportMessage->setText(message);
}
