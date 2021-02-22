#ifndef WINDOWCONTROL_H
#define WINDOWCONTROL_H

#include <QDialog>
#include "streamviewer.h"

namespace Ui {
class WindowControl;
}

class WindowControl : public QDialog
{
    Q_OBJECT

struct Roomdt
    {
        QString Name;
        bool Empty;
        bool PP;
        QString Link;
    };


public:
    explicit WindowControl(QWidget *parent = nullptr, StreamViewer *SV = nullptr);
    ~WindowControl();

private:
    Ui::WindowControl *ui;
    StreamViewer *ParentSV;

    QNetworkAccessManager *manager;
    QNetworkRequest request;
    QList<Roomdt> RoomList;

    //  ROOM PAGE
    void RMgetroom();
    void RMReqParser(QString reqresult);


private slots:
    //  ROOM PAGE
    void RMEnterbtn();
    void RMClosebtn();
    void Populate();
    void SearchRoom();
    void ClickLinkText(const QString &link);
};

#endif // WINDOWCONTROL_H
