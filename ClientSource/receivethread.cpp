#include "receivethread.h"

/*
        SOURCE FILE:        receivethread.cpp
        
        PROGRAM:            Linux Chat

        FUNCTIONS:          ReceiveThread::ReceiveThread(int socket);
                            ReceiveThread::~ReceiveThread();
                            void ReceiveThread::process();

        PROGRAMMER:         Alvin Man

        DESIGNER:           Alvin Man

        NOTES:              Defines the logic to handle listening for incoming message
                            broadcasts from the server.  Checks to see what kind of messages
                            are sent and handles them accordingly.
*/

/*
        FUNCTION:           ReceiveThread Constructor

        PROGRAMMER:         Alvin Man

        DESIGNER:           Alvin Man

        INTERFACE:          ReceiveThread::ReceiveThread(int socket);
                                socket - socket to receive data from

        RETURNS:            N/A

        NOTES:              Constructor for a new receive thread instance.
*/
ReceiveThread::ReceiveThread(int socket) {
    m_socket = socket;
    abort = false;
}

/*
        FUNCTION:           Client Destructor

        PROGRAMMER:         Alvin Man

        DESIGNER:           Alvin Man

        INTERFACE:          ReceiveThread::~ReceiveThread();

        RETURNS:            N/A

        NOTES:              Destructor for receive thread instance.
*/
ReceiveThread::~ReceiveThread() {
    // free resources
}

/*
        FUNCTION:           process

        PROGRAMMER:         Alvin Man

        DESIGNER:           Alvin Man

        INTERFACE:          void ReceiveThread::process();

        RETURNS:            void

        NOTES:              Handles the reading of data from the socket, and determines
                            the type of message being read, and handles them accordingly.
                            Messages with a USER tag are parsed into usernames to update
                            the host list.  Messages with a MESSAGE tag are parsed into
                            messages sent by other clients.  Messages with a DISCONNECT tag
                            are parsed into disconnect status strings.
*/
void ReceiveThread::process() {
    // allocate resources using new here
    qRegisterMetaType<QVector<QString>>("userVector");

    int n;
    int bytes_to_read;
    char *bp;
    char buf[BUFLEN];
    QVector<QString> userList;

    while(abort != true){

        bp = buf;
        bytes_to_read = BUFLEN;
        if ((n = recv(m_socket, bp, bytes_to_read, 0)) > 0)
        {
            bytes_to_read -= n;
        }

        //check if it is a USER msg or MESSAGE msg
        std::string s(bp);
        std::string msg(bp);

        std::string userDelimiter = "USER: ";
        std::string msgDelimiter = "MESSAGE: ";
        std::string disconnectDelimiter = "DISCONNECT: ";
        size_t pos = 0;

        //USER message, update client list box
        if(s.find(userDelimiter) != std::string::npos){

            //clear old userlist vector
            userList.clear();

            //delete all the "USERS: " in the string
            while(s.find(userDelimiter) != std::string::npos){
                //delete the user string
                pos = s.find(userDelimiter);
                s.erase(pos, userDelimiter.length());
            }

            //parse the resulting tokens into an array
            std::stringstream ss(s);
            std::string token;
            while(std::getline(ss, token, ' ')){
                QString tokenString = QString::fromUtf8(token.c_str());
                userList.push_back(tokenString);
            }

            emit updateUserList(userList);
        }

        //regular MESSAGE tag, update chat history box
        if(msg.find(msgDelimiter) != std::string::npos){

            std::string username;

            //delete the "MESSAGE: " tag
            msg.erase(0, msgDelimiter.length());

            //grab the username token
            pos = msg.find(':');
            username = msg.substr(0, pos+1);
            msg.erase(0, pos+1);

            //convert string to QString
            QString qUsername = QString::fromUtf8(username.c_str());
            QString qMsg = QString::fromUtf8(msg.c_str());

            //emit the updateChatBox signal with username and message
            emit updateChatBox(qUsername, qMsg, "message");
        }

        //disconnect msg, notify clients that a user has disconnected
        if(msg.find(disconnectDelimiter) != std::string::npos){

            //delete the "DISCONNECT: " tag
            msg.erase(0, disconnectDelimiter.length());

            //convert string to QString
            QString qUsername = QString::fromUtf8(msg.c_str());

            //emit the updateChatBox signal with username
            emit updateChatBox(qUsername, " has disconnected from the server.", "disconnect");
        }
    }
    emit finished();
}

