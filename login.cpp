#include "login.h"
#include "ui_login.h"
#include "sharedvar.h"

#include<QColor>
#include<QStringListModel>
#include<QJsonObject>
#include<QtConcurrent/QtConcurrent>
#include<QFutureWatcher>
#include<QInputDialog>
#include<QCryptographicHash>
#include<QGraphicsDropShadowEffect>
#include<QDesktopServices>

QString Ver = "0.9.4.2";

//  WEBSOCKET HANDLER
QWebSocketServer *m_pWebSocketServer;
QList<QWebSocket *> m_clients;

/* mode int
 * 0 = front page
 * 1 = Active Session
 * 2 = Active Session Link Search
 * 3 = Archive
 * 4 = Archive Link Search
*/

Login::Login(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Login)
{
    ui->setupUi(this);
    mode = 0;
    Frontpage = this;

    manager = new QNetworkAccessManager();
    connect(manager, &QNetworkAccessManager::finished,
        this, [=](QNetworkReply *reply) {
            if (reply->error()) {
                return;
            }

            QString answer = reply->readAll();

            switch (ui->stackedWidget->currentIndex()) {
                case (0):
                    FPReqParser(answer);
                    break;
                case (1):
                    RMReqParser(answer);
                    break;
                case (2):
                    ARLReqParser(answer);
                    break;
                case (3):
                    ARReqParser(answer);
                    break;
            }
        }
    );

    //---------------- Connect FP buttons ----------------------
    connect(ui->FPExit, SIGNAL(released()), this, SLOT(FPClosebtn()));
    connect(ui->FPSession, SIGNAL(released()), this, SLOT(FPSessionbtn()));
    connect(ui->FPArchive, SIGNAL(released()), this, SLOT(FPArchivebtn()));
    connect(ui->FPVersion, SIGNAL(linkActivated(QString)), this, SLOT(ClickLinkFP(QString)));
    FPgetversion();

    //---------------- Connect session buttons ----------------------
    connect(ui->RMCloseBtn, SIGNAL(released()), this, SLOT(RMClosebtn()));
    connect(ui->RMEnterBtn, SIGNAL(released()), this, SLOT(RMEnterbtn()));
    connect(ui->RMcheckBox, SIGNAL(stateChanged(int)), this, SLOT(Populate()));
    connect(ui->RMlineEdit, SIGNAL(textChanged(const QString)), this, SLOT(SearchRoom()));
    connect(ui->RMListDisplay, SIGNAL(linkActivated(QString)), this, SLOT(ClickLinkText(QString)));

    //---------------- Connect archive list buttons ----------------------
    connect(ui->ARLCloseBtn, SIGNAL(released()), this, SLOT(ARLClosebtn()));
    connect(ui->ARLSearchBtn, SIGNAL(released()), this, SLOT(ARLSearchbtn()));
    connect(ui->ARLSearchTags, SIGNAL(released()), this, SLOT(ARLSearchbtnTag()));
    connect(ui->ARLListDisplay, SIGNAL(linkActivated(QString)), this, SLOT(ARLClickLink(QString)));

    //---------------- Connect archive buttons ----------------------
    connect(ui->ARCloseBtn, SIGNAL(released()), this, SLOT(ARClosebtn()));
    connect(ui->ARSearchBtn, SIGNAL(released()), this, SLOT(ARSearchbtn()));
    connect(ui->ARSearchTags, SIGNAL(released()), this, SLOT(ARSearchbtnTag()));
    connect(ui->ARlineEdit, SIGNAL(textChanged(const QString)), this, SLOT(ARsearch()));
    connect(ui->ARListDisplay, SIGNAL(linkActivated(QString)), this, SLOT(ARClickLink(QString)));

    ARmanager = new QNetworkAccessManager();
    connect(ARmanager, SIGNAL(finished(QNetworkReply*)),
        this, SLOT(ArchiveParser(QNetworkReply*)));

    ARrequest.setUrl(QUrl("http://157.230.241.238/Archive/"));
    ARrequest.setRawHeader("Content-Type", "application/json");
}

Login::~Login()
{
    delete ui;
}



//---------------------------------------------- Front Page ---------------------------------------------
void Login::FPClosebtn(){
    Frontpage->close();
}

void Login::FPSessionbtn(){
    ui->RMcheckBox->setVisible(true);
    ui->RMTitle->setText("<html><head/><body><p><span style=\" font-size:48pt; font-weight:600; color:#ffffff;\">ROOM</span></p><p><span style=\" font-size:16pt; font-weight:600; color:#ff0000;\">•</span><span style=\" font-size:16pt; font-weight:600; color:#ffffff;\"> = Empty </span><span style=\" font-size:16pt; font-weight:600; color:#4e9a06;\">•</span><span style=\" font-size:16pt; font-weight:600; color:#ffffff;\"> = Password Protected</span></p></body></html>");
    mode = 1;
    ui->stackedWidget->setCurrentIndex(1);
    ui->RMEnterBtn->setText("Search By Link");
    RMgetroom();
}

void Login::FPArchivebtn(){
    ui->RMcheckBox->setVisible(false);
    ui->RMTitle->setText("<html><head/><body><p><span style=\" font-size:48pt; font-weight:600; color:#ffffff;\">ROOM</span></p></html>");
    mode = 3;
    ui->stackedWidget->setCurrentIndex(1);
    ui->RMEnterBtn->setText("Search By Link/Tags");
    RMgetroom();
}

void Login::FPgetversion() {
    request.setUrl(QUrl("http://157.230.241.238/version"));
    manager->get(request);
}

void Login::FPReqParser(QString reqresult){
    QJsonDocument jsonResponse = QJsonDocument::fromJson(reqresult.toUtf8());
    QJsonObject jsonObject = jsonResponse.object();

    if (jsonObject["Ver"].toString() == Ver){
        ui->FPVersion->setText("Version : " + Ver);
    } else {
        ui->FPVersion->setText("Newer version is available ( v" + jsonObject["Ver"].toString() + " ) <a href=\"https://drive.google.com/drive/u/0/folders/1a32LOTm1iOmnBLO03fgtYFEKelY9LrrA\"><span style=\" text-decoration: underline; color:#007af4;\">&gt; Click Here &lt;</span></a>");
    }
}

void Login::ClickLinkFP(const QString &link){
    QDesktopServices::openUrl(QUrl(link));
}
//============================================== Front Page =============================================



//--------------------------------------------- Room Page --------------------------------------------
void Login::RMClosebtn(){
    mode = 0;
    ui->stackedWidget->setCurrentIndex(0);
    RoomList.clear();
    ui->RMlineEdit->setText("");
    ui->RMcheckBox->setChecked(false);
}

void Login::RMEnterbtn() {
    if (mode == 1){
        bool ok = false;
        QString DialogText = QInputDialog::getText(this, tr("Search Stream Link"),
                                             tr("Search Stream:"), QLineEdit::Normal,
                                             "", &ok);

        if (ok && !DialogText.isEmpty())   {
            mode = 2;
            request.setUrl(QUrl("http://157.230.241.238/Room/?link=" + DialogText));
            manager->get(request);
        }
    } else {
        mode = 4;
        ui->stackedWidget->setCurrentIndex(2);
        ARLgetlist();
    }
}

void Login::RMgetroom() {
    request.setUrl(QUrl("http://157.230.241.238/Room/"));
    manager->get(request);
}

void Login::ClickLinkText(const QString &link){
    if (mode == 1){
        if (link == "MSync"){
            StreamWindowsList.append({new StreamViewer(nullptr, link, ""), QList<QString>({ link }), QList<QString>({ })});
            StreamWindowsList.last().StreamView->show();
            this->hide();
        } else {
            foreach(const Roomdt& dt, RoomList){
                if (dt.Name == link) {
                    if (dt.PP){
                        bool ok = false;
                        QString DialogText = QInputDialog::getText(this, tr("Password Protected"),
                                                             tr("Password:"), QLineEdit::Password,
                                                             "", &ok);

                        if (ok && !DialogText.isEmpty())   {
                            StreamWindowsList.append({new StreamViewer(nullptr, link, QString("%1").arg(QString(QCryptographicHash::hash(DialogText.toUtf8(),QCryptographicHash::Sha256).toHex()))), QList<QString>({ link }), QList<QString>({ })});
                            StreamWindowsList.last().StreamView->show();
                            this->hide();
                        }
                    } else {
                        StreamWindowsList.append({new StreamViewer(nullptr, link, ""), QList<QString>({ link }), QList<QString>({ })});
                        StreamWindowsList.last().StreamView->show();
                        this->hide();
                    }
                    break;
                }
            }
        }
    } else {
        ui->stackedWidget->setCurrentIndex(3);
        ARgetlist(link);
        CurrentRoom = link;
    }
}

void Login::RMReqParser(QString reqresult){
    QJsonDocument jsonResponse = QJsonDocument::fromJson(reqresult.toUtf8());
    QList<QVariant> list =  jsonResponse.toVariant().toList();

    QString S = "";
    if (mode == 1){
        S = "<a href=\"MSync\" style=\" text-decoration: none; color:#000000;\">MSync</a><br>";
    }

    foreach(QVariant dt, list){
        if (mode != 2){
            RoomList.append({dt.toMap()["Nick"].toString(), dt.toMap()["Empty"].toBool(), dt.toMap()["EntryPass"].toBool(), dt.toMap()["StreamLink"].toString()});
        }
        if (mode == 3) {
            S += "<a href=\"" + dt.toMap()["Nick"].toString() + "\" style=\" text-decoration: none; color:#000000;\">" + dt.toMap()["Nick"].toString() + "</a><br>";
        } else if (mode == 2){
            if (dt.toMap()["EntryPass"].toBool()){
                S += "<span style=\"color:#4e9a06;\">•</span>";
            }

            if (dt.toMap()["Empty"].toBool()){
                S += "<span style=\"color:#ff0000;\">•</span>";
            }
            S += "<a href=\"" + dt.toMap()["Nick"].toString() + "\" style=\" text-decoration: none; color:#000000;\">" + dt.toMap()["Nick"].toString() + "</a><br>";
        } else if (!RoomList[RoomList.count() - 1].Empty){
            S += "<a href=\"" + dt.toMap()["Nick"].toString() + "\" style=\" text-decoration: none; color:#000000;\">" + dt.toMap()["Nick"].toString() + "</a><br>";
        }
    }

    ui->RMListDisplay->setText(S);
    ui->RMListDisplay->adjustSize();
    ui->scrollAreaWidgetContents->setMinimumSize(0, ui->RMListDisplay->height());
    if (mode == 2){
        mode = 1;
    }
}

void Login::Populate(){
    QString Stemp = "";
    QString s = "";
    if (ui->RMcheckBox->isChecked()){
        foreach(const Roomdt& dt, RoomList){
            s = "";
            if (dt.PP){
                s += "<span style=\"color:#4e9a06;\">•</span>";
            }

            if (dt.Empty){
                s += "<span style=\"color:#ff0000;\">•</span>";
            }

            s += "<a href=\"" + dt.Name + "\" style=\" text-decoration: none; color:#000000;\">" + dt.Name + "</a><br>";

            if (mode == 3){
                Stemp += s;
            } else if (dt.Empty){
                Stemp += s;
            } else {
                Stemp = s + Stemp;
            }
        }
        if (mode == 1){
            Stemp = "<a href=\"MSync\" style=\" text-decoration: none; color:#000000;\">MSync</a><br>" + Stemp;
        }

        ui->RMListDisplay->setText(Stemp);
        ui->RMListDisplay->adjustSize();
        ui->scrollAreaWidgetContents->setMinimumSize(0, ui->RMListDisplay->height());
    } else {
        if (mode == 1){
            Stemp = "<a href=\"MSync\" style=\" text-decoration: none; color:#000000;\">MSync</a><br>";
        }
        foreach(const Roomdt& dt, RoomList){
            s = "";
            if (mode == 3){
                s = "<a href=\"" + dt.Name + "\" style=\" text-decoration: none; color:#000000;\">" + dt.Name + "</a><br>";
            } else if (!dt.Empty){
                if (dt.PP){
                    s += "<span style=\"color:#73d216;\">•</span>";
                }

                s += "<a href=\"" + dt.Name + "\" style=\" text-decoration: none; color:#000000;\">" + dt.Name + "</a><br>";
            }
            Stemp += s;
        }
        ui->RMListDisplay->setText(Stemp);
        ui->RMListDisplay->adjustSize();
        ui->scrollAreaWidgetContents->setMinimumSize(0, ui->RMListDisplay->height());
    }
}

void Login::SearchRoom(){
    if (ui->RMlineEdit->text().trimmed().count() == 0 ){
        this->Populate();
    } else {
        QString Stemp = "";
        QString s;
        if (ui->RMcheckBox->isChecked()){
            foreach(const Roomdt& dt, RoomList){
                if (dt.Name.indexOf(ui->RMlineEdit->text(),0,Qt::CaseInsensitive) != -1){
                    s = "";
                    if (dt.PP){
                        s += "<span style=\"color:#4e9a06;\">•</span> ";
                    }

                    if (dt.Empty){
                        s += "<span style=\"color:#ff0000;\">•</span> ";
                    }

                    s += "<a href=\"" + dt.Name + "\" style=\" text-decoration: none; color:#000000;\">" + dt.Name + "</a><br>";

                    if (mode == 3){
                        Stemp += s;
                    } else if (dt.Empty){
                        Stemp += s;
                    } else {
                        Stemp = s + Stemp;
                    }
                }
            }
            if (mode == 1){
                Stemp = "<a href=\"MSync\" style=\" text-decoration: none; color:#000000;\">MSync</a><br>" + Stemp;
            }
            ui->RMListDisplay->setText(Stemp);
            ui->RMListDisplay->adjustSize();
            ui->scrollAreaWidgetContents->setMinimumSize(0, ui->RMListDisplay->height());
        } else {
            if (mode == 1){
                Stemp = "<a href=\"MSync\" style=\" text-decoration: none; color:#000000;\">MSync</a><br>";
            }

            foreach(const Roomdt& dt, RoomList){
                if (dt.Name.indexOf(ui->RMlineEdit->text(),0,Qt::CaseInsensitive) != - 1){
                    s = "";
                    if (mode == 3){
                        s = "<a href=\"" + dt.Name + "\" style=\" text-decoration: none; color:#000000;\">" + dt.Name + "</a><br>";
                    } else if (!dt.Empty){
                        if (dt.PP){
                            s += "<span style=\"color:#73d216;\">•</span>";
                        }

                        s += "<a href=\"" + dt.Name + "\" style=\" text-decoration: none; color:#000000;\">" + dt.Name + "</a><br>";
                    }
                    Stemp += s;
                }
            }
            ui->RMListDisplay->setText(Stemp);
            ui->RMListDisplay->adjustSize();
            ui->scrollAreaWidgetContents->setMinimumSize(0, ui->RMListDisplay->height());
        }
    }
}
//============================================= Session Page ============================================



//------------------------------------------- Archive List Page ------------------------------------------
void Login::ARLClosebtn(){
    mode = 3;
    ui->stackedWidget->setCurrentIndex(1);
}

void Login::ARLSearchbtn() {
    bool ok = false;
    QString DialogText = QInputDialog::getText(this, tr("Search Stream Link"),
                                         tr("Link:"), QLineEdit::Normal,
                                         "", &ok);

    if (ok) {
        mode = 4;
        if (DialogText.isEmpty()){
            request.setUrl(QUrl("http://157.230.241.238/Archive/"));
        } else {
            request.setUrl(QUrl("http://157.230.241.238/Archive/?link=" + DialogText));
        }
        manager->get(request);
    }
}

void Login::ARLSearchbtnTag() {
    bool ok = false;
    QString DialogText = QInputDialog::getText(this, tr("Search By Tags (use comma separator, ex: Full, EN, 3D)"),
                                         tr("Tags (use comma separator, ex: Full, EN, 3D):"), QLineEdit::Normal,
                                         "", &ok);

    if (ok) {
        mode = 4;
        if (DialogText.isEmpty()){
            request.setUrl(QUrl("http://157.230.241.238/Archive/"));
        } else {
            request.setUrl(QUrl("http://157.230.241.238/Archive/?tags=" + DialogText.replace(", ", "_").replace(",", "_")));
        }
        manager->get(request);
    }
}

void Login::ARLgetlist() {
    request.setUrl(QUrl("http://157.230.241.238/Archive/"));
    manager->get(request);
}

void Login::ARLClickLink(const QString &link){
    foreach(const Archivedt& dt, ArchiveList){
        if (dt.Link == link) {
            if (dt.PP){
                bool ok = false;
                QString DialogText = QInputDialog::getText(this, tr("Password Protected"),
                                                     tr("Password:"), QLineEdit::Password,
                                                     "", &ok);

                if (ok && !DialogText.isEmpty())   {
                    QJsonObject obj;
                    obj["link"] = link;
                    obj["pass"] = QString("%1").arg(QString(QCryptographicHash::hash(DialogText.toUtf8(),QCryptographicHash::Sha256).toHex()));
                    QJsonDocument doc(obj);
                    QByteArray data = doc.toJson();
                    ARmanager->post(ARrequest, data);
                }
            } else {
                QJsonObject obj;
                obj["link"] = link;
                obj["pass"] = "";
                QJsonDocument doc(obj);
                QByteArray data = doc.toJson();
                ARmanager->post(ARrequest, data);
            }
            break;
        }
    }
}

void Login::ARLReqParser(QString reqresult){
    QJsonDocument jsonResponse = QJsonDocument::fromJson(reqresult.toUtf8());
    QList<QVariant> list =  jsonResponse.toVariant().toList();
    ArchiveList.clear();
    QString S = "";

    foreach(QVariant dt, list){
        ArchiveList.append({dt.toMap()["Nick"].toString(), dt.toMap()["Link"].toString(), dt.toMap()["Pass"].toBool()});

        if (dt.toMap()["Pass"].toBool()){
            S += "<span style=\"color:#4e9a06;\">•</span>";
        }
        S += "<a href=\"" + dt.toMap()["Link"].toString() + "\" style=\" text-decoration: none; color:#000000;\">" + dt.toMap()["Nick"].toString() + "</a><br>";
    }

    ui->ARLListDisplay->setText(S);
    ui->ARLListDisplay->adjustSize();
    ui->scrollAreaWidgetContents->setMinimumSize(0, ui->ARLListDisplay->height());
}
//=========================================== Archive List Page ==========================================



//--------------------------------------------- Archive Page --------------------------------------------
void Login::ARClosebtn(){
    mode = 3;
    ui->stackedWidget->setCurrentIndex(1);
    ui->ARlineEdit->setText("");
    ArchiveList.clear();
}

void Login::ARSearchbtn() {
    bool ok = false;
    QString DialogText = QInputDialog::getText(this, tr("Search Stream Link"),
                                         tr("Search Stream:"), QLineEdit::Normal,
                                         "", &ok);

    if (ok && !DialogText.isEmpty())   {
        mode = 4;
        request.setUrl(QUrl("http://157.230.241.238/Archive/?room=" + CurrentRoom + "&link=" + DialogText));
        manager->get(request);
    }

}

void Login::ARSearchbtnTag() {
    bool ok = false;
    QString DialogText = QInputDialog::getText(this, tr("Search By Tags"),
                                         tr("Tags (use comma separator, ex: Full, EN, 3D):"), QLineEdit::Normal,
                                         "", &ok);

    if (ok && !DialogText.isEmpty())   {
        mode = 4;
        request.setUrl(QUrl("http://157.230.241.238/Archive/?room=" + CurrentRoom + "&tags=" + DialogText.replace(", ", "_").replace(",", "_")));
        manager->get(request);
    }

}

void Login::ARgetlist(QString Roomnick) {
    request.setUrl(QUrl("http://157.230.241.238/Archive/?room=" + Roomnick));
    manager->get(request);
}

void Login::ARClickLink(const QString &link){
    foreach(const Archivedt& dt, ArchiveList){
        if (dt.Link == link) {
            if (dt.PP){
                bool ok = false;
                QString DialogText = QInputDialog::getText(this, tr("Password Protected"),
                                                     tr("Password:"), QLineEdit::Password,
                                                     "", &ok);

                if (ok && !DialogText.isEmpty())   {
                    QJsonObject obj;
                    obj["link"] = link;
                    obj["pass"] = QString("%1").arg(QString(QCryptographicHash::hash(DialogText.toUtf8(),QCryptographicHash::Sha256).toHex()));
                    QJsonDocument doc(obj);
                    QByteArray data = doc.toJson();
                    ARmanager->post(ARrequest, data);
                }
            } else {
                QJsonObject obj;
                obj["link"] = link;
                obj["pass"] = "";
                QJsonDocument doc(obj);
                QByteArray data = doc.toJson();
                ARmanager->post(ARrequest, data);
            }
            break;
        }
    }
}

void Login::ARReqParser(QString reqresult){
    QJsonDocument jsonResponse = QJsonDocument::fromJson(reqresult.toUtf8());
    QList<QVariant> list =  jsonResponse.toVariant().toList();

    QString S = "";

    foreach(QVariant dt, list){
        if (mode != 4){
            ArchiveList.append({dt.toMap()["Nick"].toString(), dt.toMap()["Link"].toString(), dt.toMap()["Pass"].toBool()});
        }

        if (dt.toMap()["Pass"].toBool()){
            S += "<span style=\"color:#4e9a06;\">•</span>";
        }
        S += "<a href=\"" + dt.toMap()["Link"].toString() + "\" style=\" text-decoration: none; color:#000000;\">" + dt.toMap()["Nick"].toString() + "</a><br>";
    }

    ui->ARListDisplay->setText(S);
    ui->ARListDisplay->adjustSize();
    ui->scrollAreaWidgetContents->setMinimumSize(0, ui->ARListDisplay->height());

    if (mode == 4){
        mode = 3;
    }
}

void Login::ARpopulate(){
    QString Stemp = "";
    QString s = "";

    foreach(const Archivedt& dt, ArchiveList){
        s = "";
        if (dt.PP){
            s += "<span style=\"color:#4e9a06;\">•</span>";
        }

        s += "<a href=\"" + dt.Link + "\" style=\" text-decoration: none; color:#000000;\">" + dt.Name + "</a><br>";

        Stemp += s;
    }

    ui->ARListDisplay->setText(Stemp);
    ui->ARListDisplay->adjustSize();
    ui->scrollAreaWidgetContents->setMinimumSize(0, ui->ARListDisplay->height());
}

void Login::ARsearch(){
    if (ui->ARlineEdit->text().trimmed().count() == 0 ){
        this->ARpopulate();
    } else {
        QString Stemp = "";
        QString s;

        foreach(const Archivedt& dt, ArchiveList){
            if (dt.Name.indexOf(ui->ARlineEdit->text(),0,Qt::CaseInsensitive) != -1){
                s = "";
                if (dt.PP){
                    s += "<span style=\"color:#4e9a06;\">•</span>";
                }

                s += "<a href=\"" + dt.Link + "\" style=\" text-decoration: none; color:#000000;\">" + dt.Name + "</a><br>";

                Stemp += s;
            }
        }

        ui->ARListDisplay->setText(Stemp);
        ui->ARListDisplay->adjustSize();
        ui->scrollAreaWidgetContents->setMinimumSize(0, ui->ARListDisplay->height());

    }
}

void Login::ArchiveParser(QNetworkReply *reply){
    QString result = reply->readAll();
    if (result.indexOf("ERROR") != 0){
        Entries.clear();
        QJsonDocument jsonResponse = QJsonDocument::fromJson(result.toUtf8());
        QList<QVariant> list =  jsonResponse.toVariant().toList();

        foreach(QVariant dt, list){
            Entries.append({dt.toMap()["Stime"].toDouble(), dt.toMap()["Stext"].toString(), dt.toMap()["CC"].toString(), dt.toMap()["OC"].toString()});
        }

        ArcPicker = new Archivepicker(nullptr);
        ArcPicker->show();
        this->hide();
    }
}

//============================================= Archive Page ============================================



//--------------------------------------------- MESSAGE NEXUS --------------------------------------------
void Login::RemoveWindowList(StreamViewer *SV){
    if (StreamWindowsList.count() == 1){
        this->show();
    }

    for(int i = 0; i < StreamWindowsList.count(); i++){
        if (StreamWindowsList[i].StreamView == SV){
            foreach (QString Roomname, StreamWindowsList[i].Rooms){
                CloseReplay(Roomname, SV);
            }

            SV->close();
            StreamWindowsList.removeAt(i);
            break;
        }
    }
}

void Login::OpenReplay(QString UIDRoom, QString RoomPass, StreamViewer* SV){
    bool find = false;

    for (int i = 0; i < StreamWindowsList.count(); i++){
        if (StreamWindowsList[i].StreamView == SV){
            StreamWindowsList[i].Rooms.append(UIDRoom);
        }
    }

    for(int i = 0; i < ReplyList.count(); i++){
        if (ReplyList[i].RoomName == UIDRoom){
            find = true;
            ReplyList[i].Listener += 1;
            foreach(const StreamWindow& sw, StreamWindowsList){
                if (sw.StreamView == SV){
                    sw.StreamView->EntryDisplay({"-", 0, "CONNECTED TO SERVER", "00D921", "FFFFFF"});
                }
            }
        }
    }

    if (!find){
        QString Conn;
        if (RoomPass == ""){
            Conn = "http://157.230.241.238/Listener/?room=" + UIDRoom;
        } else {
            Conn = "http://157.230.241.238/Listener/?room=" + UIDRoom + "&pass=" + RoomPass;
        }

        QNetworkRequest req(Conn);
        req.setRawHeader(QByteArray("Accept"), "text/event-stream");
        req.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
        req.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork); // Events shouldn't be cached
        ReplyList.append({NetworkManager->get(req), UIDRoom, 1});
        connect(ReplyList.last().Reply, SIGNAL(readyRead()), this, SLOT(streamReceived()));
        foreach(const StreamWindow& sw, StreamWindowsList){
            if (sw.StreamView == SV){
                sw.StreamView->EntryDisplay({"-", 0, "CONNECTED TO SERVER", "00D921", "FFFFFF"});
            }
        }
    }
}

void Login::CloseReplay(QString UIDRoom, StreamViewer* SV){
    for (int i = 0; i < StreamWindowsList.count(); i++){
        if (StreamWindowsList[i].StreamView == SV){
            StreamWindowsList[i].Rooms.removeAll(UIDRoom);
        }
    }

    for(int i = 0; i < ReplyList.count(); i++){
        if (ReplyList[i].RoomName == UIDRoom){
            ReplyList[i].Listener -= 1;
            if (ReplyList[i].Listener <= 0){
                ReplyList[i].Reply->abort();
                ReplyList[i].Reply->deleteLater();
                ReplyList.removeAt(i);
                i--;
            }
        }
    }

}

void Login::streamReceived()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    QString Roomname = "";
    foreach(const NetworkReply& nw, ReplyList){
        if (nw.Reply == reply){
            Roomname = nw.RoomName;

            QString msg = QString(reply->readAll());

            if (msg.indexOf("ERROR") == 0){
                foreach(const StreamWindow& sw, StreamWindowsList){
                    if (sw.Rooms.contains(Roomname)){
                        sw.StreamView->EntryDisplay({"-", 0, "UNABLE TO CONNECT TO SERVER (PASS DOES NOT MATCH)", "D90007", "FFFFFF"});

                        reply->abort();
                        for(int i = 0; i < ReplyList.count(); i++){
                            if (ReplyList[i].Reply == reply){
                                for (int i = 0; i < StreamWindowsList.count() - 1; i++){
                                    StreamWindowsList[i].Rooms.removeAll(Roomname);
                                }
                                reply->deleteLater();
                                break;
                            }
                        }
                    }
                }
            } else if (msg.indexOf("\"flag\":\"Connect\"") != -1){
                foreach(const StreamWindow& sw, StreamWindowsList){
                    if (sw.Rooms.contains(Roomname)){
                        sw.StreamView->EntryDisplay({"-", 0, "CONNECTED TO SERVER", "00D921", "FFFFFF"});
                    }
                }
            } else if (msg.indexOf("\"flag\":\"insert\"") != -1){
                msg = "[" + msg.split("\"content\":")[1].replace("}\n\n", "") + "]";
                QJsonDocument jsonResponse = QJsonDocument::fromJson(msg.toUtf8());
                QList<QVariant> list =  jsonResponse.toVariant().toList();

                foreach(QVariant dt, list){
                    foreach(const StreamWindow& sw, StreamWindowsList){
                        if (sw.Rooms.contains(Roomname)){
                            sw.StreamView->EntryDisplay({dt.toMap()["_id"].toString(), dt.toMap()["Stime"].toDouble(), dt.toMap()["Stext"].toString(), dt.toMap()["CC"].toString(), dt.toMap()["OC"].toString()});
                        }
                    }
                }

            } else if (msg.indexOf("\"flag\":\"update\"") != -1){
                msg = "[" + msg.split("\"content\":")[1].replace("}\n\n", "") + "]";
                QJsonDocument jsonResponse = QJsonDocument::fromJson(msg.toUtf8());
                QList<QVariant> list =  jsonResponse.toVariant().toList();

                foreach(QVariant dt, list){
                    foreach(const StreamWindow& sw, StreamWindowsList){
                        if (sw.Rooms.contains(Roomname)){
                            for (int i = 0; i < sw.StreamView->EntryDisplayList.count(); i++){
                                if (sw.StreamView->EntryDisplayList[i].ID == dt.toMap()["_id"].toString()){
                                    sw.StreamView->RepaintEntry({dt.toMap()["_id"].toString(), dt.toMap()["Stime"].toDouble(), dt.toMap()["Stext"].toString(), dt.toMap()["CC"].toString(), dt.toMap()["OC"].toString()}, ui->scrollAreaWidgetContents->findChildren<QLabel*>()[i]);
                                    break;
                                }
                            }
                        }
                    }
                }
            }

            break;
        }
    }
}

//============================================= MESSAGE NEXUS ============================================



void Login::Test() {

}
