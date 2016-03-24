#include "client.h"
#include "ui_client.h"
#include "receivethread.h"

Client::Client(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Client)
{
    ui->setupUi(this);
    connect(ui->etMessage, SIGNAL(returnPressed()), ui->bSendMessage, SIGNAL(clicked()));
    connected = false;
    ui->bDisconnect->setEnabled(false);
    Client::updateStatusMessage("Connect to start chatting!");
}

Client::~Client()
{
    shutdown(connect_sd, SHUT_WR);
    delete ui;
}

void Client::on_bConnect_clicked(){

    // int connect_sd;
    int port;
    struct sockaddr_in server;
    struct hostent *hp;
    std::string hostnameStr;
    char* hostname;

    //clear the status message if any
    Client::updateStatusMessage("Connecting...");

    //check username before allowing any connections
    QString username = Client::getUsername();
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
    if ((connect_sd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        perror("Cannot Create Socket!");
        return;
    }

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
    } else {
        connected = true;
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

    ui->globalStatusMessages->setText("Connected");
    Client::toggleInput(false);

    //create thread to receive messages
    QThread* receiveThread = new QThread;
    ReceiveThread* receiveWorker = new ReceiveThread(connect_sd);
    receiveWorker->moveToThread(receiveThread);
    connect(receiveWorker, SIGNAL(updateChatBox(QString, QString, QString)), this, SLOT(updateChat(QString, QString, QString)));
    connect(receiveWorker, SIGNAL(updateUserList(QVector<QString>)), this, SLOT(updateUsers(QVector<QString>)));
    connect(receiveThread, SIGNAL(started()), receiveWorker, SLOT(process()));
    connect(receiveWorker, SIGNAL(finished()), receiveThread, SLOT(quit()));
    connect(receiveWorker, SIGNAL(finished()), receiveWorker, SLOT(deleteLater()));
    connect(receiveThread, SIGNAL(finished()), receiveThread, SLOT(deleteLater()));

    receiveThread->start();
}

void Client::on_bSendMessage_clicked(){

    if(connected != true){
        Client::updateStatusMessage("You are not connected");
        return;
    }

    //get message
    QString message = ui->etMessage->text();
    if(message.isEmpty()){
        return;
    }

    //get username
    QString username = Client::getUsername();

    //append message to the chat window
    Client::updateChat(username + ": ", message, "message");

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
    //only take the first word of username, if there are multiple words
    return ui->etUsername->text().split(" ").at(0);
}

void Client::updateChat(QString username, QString message, QString type){
    QString finalString;
    QTime time = QTime::currentTime();
    QString timeString = "[" + time.toString() + "]";
    QString timeStyle="<span style=\" font-size:9pt; color:#979797;\" > ";
    finalString.append(timeStyle).append(timeString).append("</span>");
    if(type == "disconnect"){
        QString italicStyle="<span style=\" font-size: 10pt; font-style: italic; color #FFFFFF;\" >";
        finalString.append(italicStyle);
    }
    QString messageStyle="<span style=\" font-size:12pt; font-weight:600; color:#FFA000;\" > ";
    finalString.append(messageStyle).append(username).append("</span>");
    finalString.append(message);
    if(type == "disconnect"){
        finalString.append("</span>");
    }

    ui->dtMessageHistory->insertHtml(finalString);
    ui->dtMessageHistory->append("\n");
    ui->dtMessageHistory->verticalScrollBar()->setValue(ui->dtMessageHistory->verticalScrollBar()->maximum());

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
        Client::updateStatusMessage("Chat log exported");
    } else {
        Client::updateStatusMessage("Export error");
        return;
    }
    file.close();
}

void Client::updateStatusMessage(QString message){
    ui->globalStatusMessages->setText(message);
}

void Client::toggleInput(bool state){
    ui->etUsername->setDisabled(!state);
    ui->etIP->setDisabled(!state);
    ui->etPort->setDisabled(!state);
    ui->bConnect->setEnabled(state);
    ui->bDisconnect->setEnabled(!state);
}

void Client::on_bDisconnect_clicked(){
    shutdown(connect_sd, SHUT_WR);
    Client::toggleInput(true);
    ui->dtUserList->clear();
    connected = false;
    Client::updateStatusMessage("Disconnected");
}
