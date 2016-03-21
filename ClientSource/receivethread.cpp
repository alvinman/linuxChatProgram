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

    qDebug("Hello World! - inside thread processing");

    int n;
    int bytes_to_read;
    char *bp;
    char buf[BUFLEN];
//    std::vector<std::string> userList;
    QVector<QString> userList;

    while(1){

        bp = buf;
        bytes_to_read = BUFLEN;
        if ((n = recv(m_socket, bp, bytes_to_read, 0)) > 0)
        {
            bytes_to_read -= n;
        }

        //check if it is a USER msg or MESSAGE msg
        //CHANGE IT BACK TO THIS STRING WHEN MARTIN CHANGES IT
        std::string s(bp);

        //debug strings only
//        std::string s("USER: x3 USER: cxpson USER: gelox6");
        std::string msg(bp);

        std::cout << s << std::endl;
        std::string userDelimiter = "USER: ";
        std::string msgDelimiter = "MESSAGE: ";
        size_t pos = 0;

        //USER message, update client list box
        if(s.find(userDelimiter) != std::string::npos){

            //clear old userlist vector
            userList.clear();

            //delete all the "USERS: " in the string
            while(s.find(userDelimiter) != std::string::npos){
                //delete the user string
                pos = s.find(userDelimiter);
                std::cout << "pos: " << pos << std::endl;
                s.erase(pos, userDelimiter.length());
                std::cout << "after delete: " << s << std::endl;
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

            std::cout << "username: " << username << std::endl;
            std::cout << "msg: " << msg << std::endl;

            //convert string to QString
            QString qUsername = QString::fromUtf8(username.c_str());
            QString qMsg = QString::fromUtf8(msg.c_str());

            //emit the updateChatBox signal with username and message
            emit updateChatBox(qUsername, qMsg);
        }
    }

    emit finished();
}

