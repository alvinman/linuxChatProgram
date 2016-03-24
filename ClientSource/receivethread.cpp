#include "receivethread.h"

// --- CONSTRUCTOR ---
ReceiveThread::ReceiveThread(int socket) {
    // you could copy data from constructor arguments to internal variables here.
    m_socket = socket;
}

// --- DECONSTRUCTOR ---
ReceiveThread::~ReceiveThread() {
    // free resources
}

// --- PROCESS ---
// Start processing data.
void ReceiveThread::process() {
    // allocate resources using new here
    qRegisterMetaType<QVector<QString>>("userVector");

    int n;
    int bytes_to_read;
    char *bp;
    char buf[BUFLEN];
    QVector<QString> userList;

    while(1){

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

            std::string username;

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

