#ifndef STREAMVIEWER_H
#define STREAMVIEWER_H

#include <QWidget>
#include <QLabel>
#include <QtWebSockets/QWebSocketServer>
#include <QtWebSockets/QWebSocket>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include "filterandstyling.h"

namespace Ui {
class StreamViewer;
}

class StreamViewer : public QWidget
{
    Q_OBJECT

struct EntryData
{
    QString ID;
    double Stime;
    QString Stext;
    QString CC;
    QString OC;
};

protected:
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

public:
    explicit StreamViewer(QWidget *parent = nullptr, QString UIDRoom = "", QString UIDPass = "");
    ~StreamViewer() override;

    //   ENTRY DISPLAY HANDLER
    void EntryDisplay(EntryData dt);
    void EntryDisplayResize();
    void RepaintEntry(EntryData dt, QLabel *label);
    QList<EntryData> EntryDisplayList;
    bool AutoScroll = true;

private:
    Ui::StreamViewer *ui;
    FilterandStyling *FSoption;
    QString StringifyTime(double timestamp);
    QPoint mpos;
    void Overwriteconfig();
    QList<QString> FilterList;

    int FPS = 1000/10;
    QTimer *Mousechecker;

    //  FRAMELESS RESIZEABLE
    int ResizeMode = 0;

    //  WEBSOCKET HANDLER
    void BroadcastMessage(QString msg);
    void ProcessCustomEntry(double stime, QString stext, QString OC, QString CC, QString STags);
    void WSSendTag(QString TextSend, QWebSocket *client);
    void WSSendKeyWord(QString TextSend, QWebSocket *client);

private slots:
    void Closebtn();
    void MenuOpen();
    void SyncBtnClick();
    void FontSizeBtnClick();
    void FilterBtnClick();
    void WinManagerClick();
    void NewWindowClick();
    void AddListenerClick();
    void RemoveListenerClick();
    void BGSheetClrBtnClick();
    void ExtraFilterClick();
    void TPCheckclick(int mode);
    void BGSheetClick(int mode);
    void ScrollToBottom(int min, int max);
    void ScrollValChange(int val);

    void streamReceived();

    //  WEBSOCKET HANDLER
    void onNewConnection();
    void processTextMessage(QString message);
    void socketDisconnected();

};

#endif // STREAMVIEWER_H
