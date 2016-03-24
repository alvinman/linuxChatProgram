#include "client.h"
#include "ui_client.h"
#include "receivethread.h"

/*
        SOURCE FILE:        client.cpp
        
        PROGRAM:            Linux Chat

        FUNCTIONS:          Client::Client(QWidget *parent) :
                                QWidget(parent),
                                ui(new Ui::Client);
                            Client::~Client();
                            void Client::on_bConnect_clicked();
                            void Client::on_bSendMessage_clicked();
                            void Client::on_bExport_clicked();
                            QString Client::getServerIP();
                            QString Client::getServerPort();
                            QString Client::getUsername();
                            void Client::updateChat(QString username, QString message, QString type);
                            void Client::updateUsers(QVector<QString> userList);
                            void Client::exportChatToText();
                            void Client::updateStatusMessage(QString message);
                            void Client::toggleInput(bool state);
                            void Client::on_bDisconnect_clicked();

        PROGRAMMER:         Alvin Man

        DESIGNER:           Alvin Man

        NOTES:              Defines the main qT window of the client module. It provides all the logic
                            to handle user input requests, and creating the sending/receiving threads.
                            Handles the formatting of the message strings sent to the server.
*/

/*
        FUNCTION:           Client Constructor

        PROGRAMMER:         Alvin Man

        DESIGNER:           Alvin Man

        INTERFACE:          Client::Client(QWidget *parent) :
                                QWidget(parent),
                                ui(new Ui::Client);

        RETURNS:            N/A

        NOTES:              Constructor for a new client instance.
*/
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

/*
        FUNCTION:           Client Destructor

        PROGRAMMER:         Alvin Man

        DESIGNER:           Alvin Man

        INTERFACE:          Client::~Client();

        RETURNS:            N/A

        NOTES:              Destructor for client instance.
*/
Client::~Client()
{
    shutdown(connect_sd, SHUT_WR);
    delete ui;
}

/*
        FUNCTION:           on_bConnect_clicked

        PROGRAMMER:         Alvin Man

        DESIGNER:           Alvin Man

        INTERFACE:          void Client::on_bConnect_clicked()

        RETURNS:            void

        NOTES:              Defines the logic to connect to the server once the connect
                            button is clicked.  Grabs the connection parameters, creates
                            the socket, address struct, and connects to server.  Also
                            creates the listening thread for receiving broadcasted messages.
*/
void Client::on_bConnect_clicked(){

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
    receiveThread = new QThread;
    receiveWorker = new ReceiveThread(connect_sd);
    receiveWorker->moveToThread(receiveThread);
    connect(receiveWorker, SIGNAL(updateChatBox(QString, QString, QString)), this, SLOT(updateChat(QString, QString, QString)));
    connect(receiveWorker, SIGNAL(updateUserList(QVector<QString>)), this, SLOT(updateUsers(QVector<QString>)));
    connect(receiveThread, SIGNAL(started()), receiveWorker, SLOT(process()));
    connect(receiveWorker, SIGNAL(finished()), receiveThread, SLOT(quit()));
    connect(receiveWorker, SIGNAL(finished()), receiveWorker, SLOT(deleteLater()));
    connect(receiveThread, SIGNAL(finished()), receiveThread, SLOT(deleteLater()));

    receiveThread->start();

}

/*
        FUNCTION:           on_bSendMessage_clicked

        PROGRAMMER:         Alvin Man

        DESIGNER:           Alvin Man

        INTERFACE:          void Client::on_bSendMessage_clicked()

        RETURNS:            void

        NOTES:              Defines the logic send a user's chat message to the server. Formats
                            the message string appropriately and starts the sendThread.
*/
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

/*
        FUNCTION:           on_bExport_clicked

        PROGRAMMER:         Alvin Man

        DESIGNER:           Alvin Man

        INTERFACE:          void Client::on_bExport_clicked()

        RETURNS:            void

        NOTES:              Handler function for the export button, calls the method
                            to export the chat history to a text file.
*/
void Client::on_bExport_clicked(){
    Client::exportChatToText();
}

/*
        FUNCTION:           getServerIP

        PROGRAMMER:         Alvin Man

        DESIGNER:           Alvin Man

        INTERFACE:          QString Client::getServerIP()

        RETURNS:            QString

        NOTES:              Returns the server ip typed into the ip input.
*/
QString Client::getServerIP(){
    return ui->etIP->text();
}

/*
        FUNCTION:           getServerPort

        PROGRAMMER:         Alvin Man

        DESIGNER:           Alvin Man

        INTERFACE:          QString Client::getServerPort()

        RETURNS:            QString

        NOTES:              Returns the server port typed into the port input.
*/
QString Client::getServerPort(){
    return ui->etPort->text();
}

/*
        FUNCTION:           getUsername

        PROGRAMMER:         Alvin Man

        DESIGNER:           Alvin Man

        INTERFACE:          QString Client::getUsername()

        RETURNS:            QString

        NOTES:              Returns the username defined in the username input.
*/
QString Client::getUsername(){
    //only take the first word of username, if there are multiple words
    return ui->etUsername->text().split(" ").at(0);
}

/*
        FUNCTION:           updateChat

        PROGRAMMER:         Alvin Man

        DESIGNER:           Alvin Man

        INTERFACE:          void Client::updateChat(QString username, QString message, QString type)
                                username - Username defined by user
                                message - the message user typed in chat input
                                type - user / message / disconnect

        RETURNS:            void

        NOTES:              Formats the received message string based on the message and type received,
                            and inserts it into the chat window.
*/
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

/*
        FUNCTION:           updateUsers

        PROGRAMMER:         Alvin Man

        DESIGNER:           Alvin Man

        INTERFACE:          void Client::updateUsers(QVector<QString> userList)
                                userList - vector containing all the users currently connected

        RETURNS:            void

        NOTES:              Updates the user panel with the currently connected list of users.
*/
void Client::updateUsers(QVector<QString> userList){
    ui->dtUserList->clear();
    for(auto& user : userList){
       ui->dtUserList->addItem(user);
    }
}

/*
        FUNCTION:           exportChatToText

        PROGRAMMER:         Alvin Man

        DESIGNER:           Alvin Man

        INTERFACE:          void Client::exportChatToText()

        RETURNS:            void

        NOTES:              Creates a text file and writes all existing chat messages into
                            the external file.
*/
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

/*
        FUNCTION:           updateStatusMessage

        PROGRAMMER:         Alvin Man

        DESIGNER:           Alvin Man

        INTERFACE:          void Client::updateStatusMessage(QString message)
                                message - status message to display

        RETURNS:            void

        NOTES:              Updates the status message with the given status string.
*/
void Client::updateStatusMessage(QString message){
    ui->globalStatusMessages->setText(message);
}

/*
        FUNCTION:           toggleInput

        PROGRAMMER:         Alvin Man

        DESIGNER:           Alvin Man

        INTERFACE:          void Client::toggleInput(bool state)
                                state - true / false, depending on the state of the connection

        RETURNS:            void

        NOTES:              Enables or disables the user inputs and connect/disconnect buttons
                            according to the state.
*/
void Client::toggleInput(bool state){
    ui->etUsername->setDisabled(!state);
    ui->etIP->setDisabled(!state);
    ui->etPort->setDisabled(!state);
    ui->bConnect->setEnabled(state);
    ui->bDisconnect->setEnabled(!state);
}

/*
        FUNCTION:           on_bDisconnect_clicked

        PROGRAMMER:         Alvin Man

        DESIGNER:           Alvin Man

        INTERFACE:          void Client::on_bDisconnect_clicked()

        RETURNS:            void

        NOTES:              Handles logic to disconnect a user session and reset the client
                            to be able to connect again.
*/
void Client::on_bDisconnect_clicked(){
    receiveWorker->abort = true;
//    receiveThread->terminate();
//    receiveWorker->finished();
    shutdown(connect_sd, SHUT_WR);
    Client::toggleInput(true);
    ui->dtUserList->clear();
    connected = false;
    Client::updateStatusMessage("Disconnected");
}
