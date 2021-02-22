#include "wsselector.h"
#include "ui_wsselector.h"
#include "sharedvar.h"
#include <QInputDialog>
#include <QJsonDocument>

WSselector::WSselector(QWidget *parent, StreamViewer *SV) :
    QDialog(parent),
    ui(new Ui::WSselector)
{
    this->setWindowFlags(Qt::WindowStaysOnTopHint);
    this->setParent(nullptr); // Create TopLevel-Widget

    ui->setupUi(this);
    ParentSV = SV;

    manager = new QNetworkAccessManager();
    connect(manager, &QNetworkAccessManager::finished,
        this, [=](QNetworkReply *reply) {
            if (reply->error()) {
                return;
            }

            QString answer = reply->readAll();
            RMReqParser(answer);
        }
    );
    //---------------- Connect session buttons ----------------------
    connect(ui->RMCloseBtn, SIGNAL(released()), this, SLOT(RMClosebtn()));
    connect(ui->RMlineEdit, SIGNAL(textChanged(const QString)), this, SLOT(SearchRoom()));
    connect(ui->RMListDisplay, SIGNAL(linkActivated(QString)), this, SLOT(ClickLinkText(QString)));
    RMgetroom();
}

WSselector::~WSselector()
{
    delete ui;
}

//--------------------------------------------- Room Page --------------------------------------------
void WSselector::RMClosebtn(){
    this->close();
}

void WSselector::RMgetroom() {
    request.setUrl(QUrl("http://157.230.241.238/Room/"));
    manager->get(request);
}

void WSselector::ClickLinkText(const QString &link){
    foreach(Roomdt dt, RoomList){
        if (dt.Name == link) {
            if (dt.PP){
                bool find = false;

                foreach(NetworkReply NR, ReplyList){
                    if (NR.RoomName == link){
                        find = true;
                        Frontpage->OpenReplay(link, "", ParentSV);
                        break;
                    }
                }


                if (!find){
                    bool ok = false;
                    QString DialogText = QInputDialog::getText(this, tr("Password Protected"),
                                                         tr("Password:"), QLineEdit::Password,
                                                         "", &ok);

                    if (ok && !DialogText.isEmpty())   {
                        Frontpage->OpenReplay(link, QString("%1").arg(QString(QCryptographicHash::hash(DialogText.toUtf8(),QCryptographicHash::Sha256).toHex())), ParentSV);
                        this->close();
                    }
                }
            } else {
                Frontpage->OpenReplay(link, "", ParentSV);
                this->close();
            }
            break;
        }
    }
}

void WSselector::RMReqParser(QString reqresult){
    QJsonDocument jsonResponse = QJsonDocument::fromJson(reqresult.toUtf8());
    QList<QVariant> list =  jsonResponse.toVariant().toList();

    QString S = "";
    QList<QString> Alreadyin;
    foreach(StreamWindow SW, StreamWindowsList){
        if (SW.StreamView == ParentSV){
            Alreadyin = SW.Rooms;
            break;
        }
    }

    foreach(QVariant dt, list){
        if (!Alreadyin.contains(dt.toMap()["Nick"].toString())){
            RoomList.append({dt.toMap()["Nick"].toString(), dt.toMap()["Empty"].toBool(), dt.toMap()["EntryPass"].toBool(), dt.toMap()["StreamLink"].toString()});

            if (!RoomList[RoomList.count() - 1].Empty){
                S += "<a href=\"" + dt.toMap()["Nick"].toString() + "\" style=\" text-decoration: none; color:#000000;\">" + dt.toMap()["Nick"].toString() + "</a><br>";
            }
        }
    }

    ui->RMListDisplay->setText(S);
    ui->RMListDisplay->adjustSize();
    ui->scrollAreaWidgetContents->setMinimumSize(0, ui->RMListDisplay->height());
}

void WSselector::Populate(){
    QString Stemp = "";
    QString s = "";
    /*
    if (ui->RMcheckBox->isChecked()){
        foreach(const Roomdt dt, RoomList){
            s = "";
            if (dt.PP){
                s += "<span style=\"color:#4e9a06;\">•</span>";
            }

            if (dt.Empty){
                s += "<span style=\"color:#ff0000;\">•</span>";
            }

            s += "<a href=\"" + dt.Name + "\" style=\" text-decoration: none; color:#000000;\">" + dt.Name + "</a><br>";

            if (dt.Empty){
                Stemp += s;
            } else {
                Stemp = s + Stemp;
            }
        }

        ui->RMListDisplay->setText(Stemp);
        ui->RMListDisplay->adjustSize();
        ui->scrollAreaWidgetContents->setMinimumSize(0, ui->RMListDisplay->height());
    } else {
        foreach(const Roomdt dt, RoomList){
            if (dt.Empty == false){
                s = "";
                if (dt.PP){
                    s += "<span style=\"color:#73d216;\">•</span>";
                }

                s += "<a href=\"" + dt.Name + "\" style=\" text-decoration: none; color:#000000;\">" + dt.Name + "</a><br>";
                Stemp += s;
            }
        }
        ui->RMListDisplay->setText(Stemp);
        ui->RMListDisplay->adjustSize();
        ui->scrollAreaWidgetContents->setMinimumSize(0, ui->RMListDisplay->height());
    }
    */
}

void WSselector::SearchRoom(){
    if (ui->RMlineEdit->text().trimmed().count() == 0 ){
        this->Populate();
    } else {
        /*
        QString Stemp = "";
        QString s;
        if (){
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

                    if (dt.Empty){
                        Stemp += s;
                    } else {
                        Stemp = s + Stemp;
                    }
                }
            }

            ui->RMListDisplay->setText(Stemp);
            ui->RMListDisplay->adjustSize();
            ui->scrollAreaWidgetContents->setMinimumSize(0, ui->RMListDisplay->height());
        } else {
            foreach(const Roomdt& dt, RoomList){
                if (dt.Name.indexOf(ui->RMlineEdit->text(),0,Qt::CaseInsensitive) != - 1){
                    if (!dt.Empty){
                        s = "";
                        if (dt.PP){
                            s += "<span style=\"color:#73d216;\">•</span>";
                        }

                        s += "<a href=\"" + dt.Name + "\" style=\" text-decoration: none; color:#000000;\">" + dt.Name + "</a><br>";
                        Stemp += s;
                    }
                }
            }
            ui->RMListDisplay->setText(Stemp);
            ui->RMListDisplay->adjustSize();
            ui->scrollAreaWidgetContents->setMinimumSize(0, ui->RMListDisplay->height());
        }
     */
    }
}
//============================================= Session Page ============================================
