#ifndef LOGIN_H
#define LOGIN_H

#include <QWidget>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QtWebSockets/QWebSocketServer>
#include <QtWebSockets/QWebSocket>
#include <QPainter>

#include "archivepicker.h"
#include "streamviewer.h"

namespace Ui {

class Login;
}

struct Roomdt
{
    QString Name;
    bool Empty;
    bool PP;
    QString Link;
};

struct Archivedt
{
    QString Name;
    QString Link;
    bool PP;
};

class Login : public QWidget
{
    Q_OBJECT

public:
    explicit Login(QWidget *parent = nullptr);
    ~Login();

    //  MULTI PAGE MANAGER
    void RemoveWindowList(StreamViewer* SV);
    void OpenReplay(QString UIDRoom, QString RoomPass, StreamViewer* SV);
    void CloseReplay(QString UIDRoom, StreamViewer* SV);

private:
    //  FRONT PAGE
    void FPgetversion();
    void FPReqParser(QString reqresult);

    //  ROOM PAGE
    void RMgetroom();
    void RMReqParser(QString reqresult);

    //  ARCHIVE LIST PAGE
    void ARLgetlist();
    void ARLReqParser(QString reqresult);

    //  ARCHIVE PAGE
    void ARgetlist(QString Roomnick);
    void ARReqParser(QString reqresult);

    Ui::Login *ui;
    Archivepicker *ArcPicker;
    StreamViewer *StreamView;
    QNetworkAccessManager *manager;
    QNetworkRequest request;
    QList<Roomdt> RoomList;

    QList<Archivedt> ArchiveList;
    QNetworkAccessManager *ARmanager;
    QNetworkRequest ARrequest;

    int mode;
    QString CurrentRoom;

private slots:
    //  FRONT PAGE
    void FPClosebtn();
    void FPSessionbtn();
    void FPArchivebtn();
    void ClickLinkFP(const QString &link);

    //  ROOM PAGE
    void RMEnterbtn();
    void RMClosebtn();
    void Populate();
    void SearchRoom();
    void ClickLinkText(const QString &link);

    //  ARCHIVE LIST PAGE
    void ARLSearchbtn();
    void ARLSearchbtnTag();
    void ARLClosebtn();
    void ARLClickLink(const QString &link);

    //  ARCHIVE PAGE
    void ARSearchbtn();
    void ARSearchbtnTag();
    void ARClosebtn();
    void ARpopulate();
    void ARsearch();
    void ARClickLink(const QString &link);
    void ArchiveParser(QNetworkReply *reply);

    void streamReceived();
    void Test();

};

#endif // LOGIN_H
