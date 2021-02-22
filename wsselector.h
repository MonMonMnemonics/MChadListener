#ifndef WSSELECTOR_H
#define WSSELECTOR_H

#include <QDialog>
#include "streamviewer.h"

namespace Ui {
class WSselector;
}

class WSselector : public QDialog
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
    explicit WSselector(QWidget *parent = nullptr, StreamViewer *SV = nullptr);
    ~WSselector();

private:
    Ui::WSselector *ui;
    StreamViewer *ParentSV;

    QNetworkAccessManager *manager;
    QNetworkRequest request;
    QList<Roomdt> RoomList;

    //  ROOM PAGE
    void RMgetroom();
    void RMReqParser(QString reqresult);


private slots:
    //  ROOM PAGE
    void RMClosebtn();
    void Populate();
    void SearchRoom();
    void ClickLinkText(const QString &link);
};

#endif // WSSELECTOR_H
