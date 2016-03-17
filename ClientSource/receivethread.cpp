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
    qDebug("Hello World! - inside thread processing");

    int n;
    int bytes_to_read;
    char *bp;
    char buf[BUFLEN];

    while(1){

        bp = buf;
        bytes_to_read = BUFLEN;
        if ((n = recv(m_socket, bp, bytes_to_read, 0)) > 0)
        {
            bytes_to_read -= n;
        }

        qDebug("got this back?");
        qDebug(bp);
        //send this to the gui thread to print
        emit updateChatBox(bp);

    }

    emit finished();
}

