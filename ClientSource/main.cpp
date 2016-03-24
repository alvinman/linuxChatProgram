#include "client.h"
#include <QApplication>
#include <QFile>

/*
        SOURCE FILE:        main.cpp
        
        PROGRAM:            Linux Chat

        FUNCTIONS:          int main(int argc, char *argv[]);

        PROGRAMMER:         Alvin Man

        DESIGNER:           Alvin Man

        NOTES:              Main starting point of the Linux Chat Program.  Defines the client
        					window, sets the proper stylesheet, and shows the window.
*/

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QFile styleFile(":qdarkstyle/style.qss");
    styleFile.open(QFile::ReadOnly | QFile::Text);
    QString style(styleFile.readAll());
    a.setStyleSheet(style);

    Client w;
    w.show();

    return a.exec();
}
