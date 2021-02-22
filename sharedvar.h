#ifndef SHAREDVAR_H
#define SHAREDVAR_H
#include <QWidget>
#include "login.h"
#include "archivepicker.h"
#include "streamviewer.h"

struct EntryData
{
    double Stime;
    QString Stext;
    QString CC;
    QString OC;
};

extern Login *Frontpage;
extern Archivepicker *Arpicker;
extern QList<EntryData> Entries;
extern bool ResizeDrag;
extern QString WSTagList;
extern QString WSKeywords;
extern QString CCmod;
extern QString OCmod;
extern QString CCFilter;
extern QString OCFilter;
extern QString BGSheetColour;
extern QFont Displayfont;

//  WEBSOCKET HANDLER
extern QWebSocketServer *m_pWebSocketServer;
extern QList<QWebSocket *> m_clients;
extern QString UIDSocket;
extern bool SocketOn;

//  MULTIPAGE VIEW
extern QNetworkAccessManager *NetworkManager;

struct StreamWindow{
    StreamViewer *StreamView;
    QList<QString> Rooms;
    QList<QString> WSList;
};

struct NetworkReply{
    QNetworkReply *Reply;
    QString RoomName;
    int Listener;
};

extern QList<StreamWindow> StreamWindowsList;
extern QList<NetworkReply> ReplyList;

class SharedVar
{    

public:
    SharedVar();
    static void Test();

};

#endif // SHAREDVAR_H
