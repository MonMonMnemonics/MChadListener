#include "streamviewer.h"
#include "ui_streamviewer.h"
#include "sharedvar.h"
#include <QWidget>
#include <QResizeEvent>
#include <QTimer>
#include <QtMath>
#include <QHBoxLayout>
#include <QScrollBar>
#include <QInputDialog>
#include <QColorDialog>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFontDialog>
#include "windowcontrol.h"

StreamViewer::StreamViewer(QWidget *parent, QString UIDRoom, QString RoomPass) :
    QWidget(parent),
    ui(new Ui::StreamViewer)
{
    ui->setupUi(this);

    this->setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
    this->setParent(nullptr); // Create TopLevel-Widget
    this->setAttribute(Qt::WA_NoSystemBackground, true);
    this->setAttribute(Qt::WA_TranslucentBackground, true);

    //      FLUSH STARTING CONDITION
    FilterList.clear();
    EntryDisplayList.clear();
    ui->scrollAreaWidgetContents->setLayout(ui->verticalLayout);
    ui->FontSizeBtn->setText("Font");
    ui->scrollArea->setStyleSheet("background-color:#" + BGSheetColour + ";");

    Mousechecker = new QTimer(this);

    //      CONNECTION CONFIGURATIONS
    connect(ui->BackBtn, SIGNAL(released()), this, SLOT(Closebtn()));
    connect(ui->SyncBtn, SIGNAL(released()), this, SLOT(SyncBtnClick()));
    connect(ui->FilterBtn, SIGNAL(released()), this, SLOT(FilterBtnClick()));
    connect(ui->FontSizeBtn, SIGNAL(released()), this, SLOT(FontSizeBtnClick()));
    connect(ui->BGSheetClr, SIGNAL(released()), this, SLOT(BGSheetClrBtnClick()));
    connect(ui->WinManager, SIGNAL(released()), this, SLOT(WinManagerClick()));
    connect(ui->NewWin, SIGNAL(released()), this, SLOT(NewWindowClick()));
    connect(ui->AddList, SIGNAL(released()), this, SLOT(AddListenerClick()));
    connect(ui->RemList, SIGNAL(released()), this, SLOT(RemoveListenerClick()));
    connect(ui->ExFilt, SIGNAL(released()), this, SLOT(ExtraFilterClick()));
    connect(Mousechecker, SIGNAL(timeout()), this, SLOT(MenuOpen()));
    connect(ui->TPcheckBox, SIGNAL(stateChanged(int)), this, SLOT(TPCheckclick(int)));
    connect(ui->BGSheet, SIGNAL(stateChanged(int)), this, SLOT(BGSheetClick(int)));
    connect(ui->scrollArea->verticalScrollBar(), SIGNAL(rangeChanged(int,int)), this, SLOT(ScrollToBottom(int, int)));
    connect(ui->scrollArea->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(ScrollValChange(int)));

    Mousechecker->start(FPS);

    if (UIDRoom == "MSync"){
        EntryDisplay({"-", 0, "CLICK SYNC ON TOP RIGHT CORNER TO SYNC WITH MSYNC", "00D921", "FFFFFF"});
    } else if (UIDRoom == "") {
        ui->FilterBtn->hide();
        ui->SyncBtn->hide();
        EntryDisplay({"-", 0, "ADD LISTENER TO START LISTENING TO LIVE SESSION", "00D921", "FFFFFF"});
    } else {
        Frontpage->OpenReplay(UIDRoom, RoomPass, this);
        //connect(nw, &QNetworkReply::readyRead, this, &StreamViewer::streamReceived);
    }
}

StreamViewer::~StreamViewer()
{
    delete ui;
}

void StreamViewer::WinManagerClick(){
    ui->stackedWidget->setCurrentIndex(2);
}

void StreamViewer::BGSheetClrBtnClick(){
    QColor DialogColor = QColorDialog::getColor(QColor("#" + BGSheetColour),
                                            this, "BGSHEET COLOUR", QColorDialog::ShowAlphaChannel);

    if (DialogColor.isValid())   {
       BGSheetColour = DialogColor.name(QColor::HexArgb).replace(QRegExp("#"), "");
       ui->scrollArea->setStyleSheet("background-color:#" + BGSheetColour + ";");
    }

}

void StreamViewer::streamReceived()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    QString msg = QString(reply->readAll());

    if (msg.indexOf("ERROR") == 0){
        EntryDisplay({"-", 0, "UNABLE TO CONNECT TO SERVER (PASS DOES NOT MATCH)", "D90007", "FFFFFF"});
    } else if (msg.indexOf("\"flag\":\"Connect\"") != -1){
        EntryDisplay({"-", 0, "CONNECTED TO SERVER", "00D921", "FFFFFF"});
    } else if (msg.indexOf("\"flag\":\"insert\"") != -1){
        msg = "[" + msg.split("\"content\":")[1].replace("}\n\n", "") + "]";
        QJsonDocument jsonResponse = QJsonDocument::fromJson(msg.toUtf8());
        QList<QVariant> list =  jsonResponse.toVariant().toList();

        foreach(QVariant dt, list){
            EntryDisplay({dt.toMap()["_id"].toString(), dt.toMap()["Stime"].toDouble(), dt.toMap()["Stext"].toString(), dt.toMap()["CC"].toString(), dt.toMap()["OC"].toString()});
        }

    } else if (msg.indexOf("\"flag\":\"update\"") != -1){
        msg = "[" + msg.split("\"content\":")[1].replace("}\n\n", "") + "]";
        QJsonDocument jsonResponse = QJsonDocument::fromJson(msg.toUtf8());
        QList<QVariant> list =  jsonResponse.toVariant().toList();

        foreach(QVariant dt, list){
            for (int i = 0; i < EntryDisplayList.count(); i++){
                if (EntryDisplayList[i].ID == dt.toMap()["_id"].toString()){
                    RepaintEntry({dt.toMap()["_id"].toString(), dt.toMap()["Stime"].toDouble(), dt.toMap()["Stext"].toString(), dt.toMap()["CC"].toString(), dt.toMap()["OC"].toString()}, ui->scrollAreaWidgetContents->findChildren<QLabel*>()[i]);
                    break;
                }
            }
        }
    }
}

void StreamViewer::FilterBtnClick(){
    FSoption = new FilterandStyling(this);
    FSoption->exec();
}

void StreamViewer::Closebtn(){
    Mousechecker->stop();
    if ((SocketOn) && (ui->SyncBtn->isVisible())){
        m_pWebSocketServer->pauseAccepting();
        m_pWebSocketServer->~QWebSocketServer();
        m_clients.clear();
        SocketOn = false;
    }

    Frontpage->RemoveWindowList(this);
}

void StreamViewer::TPCheckclick(int mode){
    if (mode == 2){
        this->setAttribute(Qt::WA_NoSystemBackground, true);
        this->setAttribute(Qt::WA_TranslucentBackground, true);
        this->show();
    } else {
        this->setAttribute(Qt::WA_NoSystemBackground, false);
        this->setAttribute(Qt::WA_TranslucentBackground, false);
        this->show();
    }
}

void StreamViewer::resizeEvent(QResizeEvent* event){
    if (event->oldSize() != QSize(-1,-1)){
        ui->stackedWidget->resize(this->window()->width() - ui->stackedWidget->pos().x() , ui->stackedWidget->height());
        ui->scrollArea->resize(this->window()->width() - ui->scrollArea->pos().x() - 5, this->window()->height() - ui->scrollArea->pos().y());

        ui->SyncBtn->move(this->window()->width() - 110, ui->SyncBtn->pos().y());
        ui->FilterBtn->move(this->window()->width() - 190, ui->FilterBtn->pos().y());
        if (event->size().width() != event->oldSize().width()){
            EntryDisplayResize();
            Overwriteconfig();
        }
    }
}

void StreamViewer::mousePressEvent(QMouseEvent *event){
    mpos = event->pos();
}

void StreamViewer::mouseMoveEvent(QMouseEvent *event){
    if ((event->buttons() == Qt::LeftButton) && (ResizeMode == 0)
            && (this->cursor().pos().y() > this->pos().y() + 10)
            && (this->cursor().pos().y() < this->pos().y() + this->window()->height() - 10)
            && (this->cursor().pos().x() > this->pos().x() + 10)
            && (this->cursor().pos().y() < this->pos().x() + this->window()->width() - 10)) {
        QPoint diff = event->pos() - mpos;
        QPoint newpos = this->pos() + diff;

        this->move(newpos);
    } else if ((ResizeMode == 1) && (event->buttons() == Qt::LeftButton) && (this->window()->pos().x() + this->window()->width() > this->cursor().pos().x())){
        this->window()->resize(this->window()->pos().x() + this->window()->width() - this->cursor().pos().x(), this->window()->height());
        this->window()->move(this->cursor().pos().x(), this->window()->y());
        ResizeDrag = true;
    } else if ((ResizeMode == 3) && (event->buttons() == Qt::LeftButton) && (this->cursor().pos().x() > this->window()->pos().x())){
        this->window()->resize(this->cursor().pos().x() - this->window()->pos().x(), this->window()->height());
        ResizeDrag = true;
    } else if ((ResizeMode == 4) && (event->buttons() == Qt::LeftButton) && (this->cursor().pos().y() > this->window()->pos().y() + ui->stackedWidget->height())){
        this->window()->resize(this->window()->width(), this->cursor().pos().y() - this->window()->pos().y());
        ResizeDrag = true;
    }
}

void StreamViewer::mouseReleaseEvent(QMouseEvent *event){
    Q_UNUSED(event);
    if (ResizeDrag){
        ResizeDrag = false;
    }
}

void StreamViewer::MenuOpen(){
    if ((this->cursor().pos().y() < ui->stackedWidget->height() + this->pos().y())
            && (this->cursor().pos().y() > this->pos().y())
            && (this->cursor().pos().x() < ui->stackedWidget->width() + this->pos().x())
            && (this->cursor().pos().x() > this->pos().x())){
        if (ui->stackedWidget->currentIndex() == 0){
            ui->stackedWidget->setCurrentIndex(1);
        }
    }  else {
        ui->stackedWidget->setCurrentIndex(0);
    }

    if ((this->cursor().pos().y() < ui->scrollArea->height() + this->pos().y())
            && (this->cursor().pos().y() > this->pos().y())
            && (this->cursor().pos().x() < ui->scrollArea->width() + this->pos().x())
            && (this->cursor().pos().x() > this->pos().x())){
        ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    } else {
        ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    }

    /*  Resize Mode
     *  0 = none
     *  1 = LEFT
     *  2 = TOP
     *  3 = RIGHT
     *  4 = BOTTOM
    */

    if (!ResizeDrag){
        if ((this->cursor().pos().y() < this->pos().y() + this->window()->height())
                && (this->cursor().pos().y() > this->pos().y())
                && (this->cursor().pos().x() < this->pos().x() + this->window()->width())
                && (this->cursor().pos().x() > this->pos().x() + this->window()->width() - 10)){
            this->setCursor(Qt::SizeHorCursor);
            ResizeMode = 3;
        } else if ((this->cursor().pos().y() < this->pos().y() + this->window()->height())
                && (this->cursor().pos().y() > this->pos().y())
                && (this->cursor().pos().x() < this->pos().x() + 10)
                && (this->cursor().pos().x() > this->pos().x())){
            this->setCursor(Qt::SizeHorCursor);
            ResizeMode = 1;
        } else if ((this->cursor().pos().y() < this->pos().y() + this->window()->height())
                && (this->cursor().pos().y() > this->pos().y() + this->window()->height() - 10)
                && (this->cursor().pos().x() < this->pos().x() + this->window()->width())
                && (this->cursor().pos().x() > this->pos().x())){
            this->setCursor(Qt::SizeVerCursor);
            ResizeMode = 4;
        } else {
            this->unsetCursor();
            ResizeMode = 0;
        }
    }
}

void StreamViewer::FontSizeBtnClick(){
    bool ok = false;
    QFont tempfont = QFontDialog::getFont(&ok, Displayfont, this);

    if (ok) {
        Displayfont = tempfont;
        EntryDisplayResize();
        Overwriteconfig();
    }
}

QString StreamViewer::StringifyTime(double timestamp){
    QString S = "";
    int temp;

    temp = int(floor(timestamp/3600000));
    if (temp < 10){
        S += "0" + QString::number(temp) + ":";
    } else {
        S += QString::number(temp) + ":";
    }
    timestamp -= uint(3600000*temp);

    temp = int(floor(timestamp/60000));
    if (temp < 10){
        S += "0" + QString::number(temp) + ":";
    } else {
        S += QString::number(temp) + ":";
    }
    timestamp -= uint(60000*temp);

    temp = int(floor(timestamp/1000));
    if (temp < 10){
        S += "0" + QString::number(temp);
    } else {
        S += QString::number(temp);
    }

    return (S);
}

void StreamViewer::BGSheetClick(int mode){
    if (mode == 2){
        ui->scrollArea->setStyleSheet("background-color:#" + BGSheetColour + ";");
    } else {
        ui->scrollArea->setStyleSheet("background-color:transparent;");
    }
}

void StreamViewer::SyncBtnClick(){
    if (!SocketOn){
        m_pWebSocketServer = new QWebSocketServer(QStringLiteral("MSync Arc Mode"), QWebSocketServer::NonSecureMode, this);
        quint16 port = 20083;

        if (m_pWebSocketServer->listen(QHostAddress::Any, port)) {
            connect(m_pWebSocketServer, SIGNAL(newConnection()),this, SLOT(onNewConnection()));
            SocketOn = true;
            ui->SyncBtn->setText("Sync (READY)");
        }
    } else {
        SocketOn = false;
        m_pWebSocketServer->pauseAccepting();
        m_pWebSocketServer->~QWebSocketServer();
        m_clients.clear();
        ui->SyncBtn->setText("Sync (OFF)");
    }
}

void StreamViewer::Overwriteconfig(){
    QFile file("AppConfig.ini");

    if(file.open(QIODevice::WriteOnly)){
        QTextStream stream(&file);
        stream << "Font=" + Displayfont.toString() << endl;
        stream <<  "BG Sheet Colour=" << BGSheetColour << endl;
        file.close();
    }
}

void StreamViewer::ExtraFilterClick(){
    bool ok = false;
    QString stemp = "";
    foreach(QString s, FilterList){
        stemp += s + ",";
    }

    if (stemp.length() > 0){
        stemp = stemp.left(stemp.length() - 1);
    }

    QString DialogText = QInputDialog::getText(this, tr("Extra Filter"),
                                         tr("Extra filter (use comma as separator ex:\"[Mio],[Korone]\")"), QLineEdit::Normal,
                                         stemp, &ok);

    if (ok) {
        FilterList.clear();
        foreach(QString s, DialogText.split(",")){
            FilterList.append(s.trimmed());
        }
    }
}

void StreamViewer::NewWindowClick(){
    StreamWindowsList.append({new StreamViewer(nullptr, "", ""), QList<QString>({}), QList<QString>({})});
    StreamWindowsList.last().StreamView->show();

    ui->stackedWidget->setCurrentIndex(1);
}

void StreamViewer::AddListenerClick(){
    ui->stackedWidget->setCurrentIndex(1);
    WindowControl *WC = new WindowControl(this, this);
    WC->exec();
}

void StreamViewer::RemoveListenerClick(){
    ui->stackedWidget->setCurrentIndex(1);

    QStringList items;
    foreach(StreamWindow SW, StreamWindowsList){
        if (SW.StreamView == this){
            items = SW.Rooms;

            if (items.count() != 0){
                bool ok;
                QString item = QInputDialog::getItem(this, tr("REMOVE LISTENER"),
                                                     tr("Listener List"), items, 0, false, &ok);
                if (ok && !item.isEmpty()){
                    Frontpage->CloseReplay(item, this);
                }
            }
        }
    }
}



//----------------------------------------------- WEB SOCKET HANDLER -------------------------------------------------
void StreamViewer::onNewConnection()
{
    QWebSocket *pSocket = m_pWebSocketServer->nextPendingConnection();

    connect(pSocket, SIGNAL(textMessageReceived(QString)), this, SLOT(processTextMessage(QString)));
    connect(pSocket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));
    ui->SyncBtn->setText("Sync (ON)");

    m_clients << pSocket;
}


void StreamViewer::processTextMessage(QString message)
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());

    if (pClient) {
        foreach(QWebSocket *socket, m_clients){
            if (socket != pClient){
                socket->close(QWebSocketProtocol::CloseCodeNormal);
            }
        }

        QList<QString> ChoppedMsg = message.split("\",\"");
        if (ChoppedMsg[0].indexOf(":\"MChad-Reg") != -1){
            if (ChoppedMsg.length() != 3){
                return;
            }
            UIDSocket = ChoppedMsg[1].split("\":\"")[1].replace(QRegExp("\""),"");

            pClient->sendTextMessage("{\"Act\":\"MChad-RegListener\"}");
            WSSendTag(WSTagList, pClient);
            WSSendKeyWord(WSKeywords, pClient);
        } else if (ChoppedMsg[0].indexOf(":\"MChad-Entry") != -1) {
            if (ChoppedMsg.length() != 7){
                return;
            }

            ProcessCustomEntry(ChoppedMsg[3].split("\":\"")[1].toDouble(),
                          ChoppedMsg[4].split("\":\"")[1],
                          ChoppedMsg[5].split("\":\"")[1],
                          ChoppedMsg[6].split("\":\"")[1].replace("\"}",""),
                          ChoppedMsg[2].split("\":\"")[1]);
        }
    }
}

void StreamViewer::socketDisconnected()
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());

    if (pClient) {
        m_clients.removeAll(pClient);
        pClient->deleteLater();
    }
}

void StreamViewer::BroadcastMessage(QString msg){
    foreach(QWebSocket *socket, m_clients){
        socket->sendTextMessage(msg);
    }
}

void StreamViewer::WSSendTag(QString TextSend, QWebSocket *client){
    if(SocketOn){
        client->sendTextMessage("{\"Act\":\"MChad-FilterApp\",\"Atr\":\"TAG\",\"Val\":\"" + TextSend + "\"}");
    };
}

void StreamViewer::WSSendKeyWord(QString TextSend, QWebSocket *client){
    if(SocketOn){
        client->sendTextMessage("{\"Act\":\"MChad-FilterApp\",\"Atr\":\"KEYWORD\",\"Val\":\"" + TextSend + "\"}");
    };
}

//=============================================== WEB SOCKET HANDLER =================================================



//--------------------------------------------- ENTRY DISPLAY HANDLER -----------------------------------------------
void StreamViewer::ScrollToBottom(int min, int max)
{
    Q_UNUSED(min);
    if (AutoScroll){
        ui->scrollArea->verticalScrollBar()->setValue(max);
    }
}

void StreamViewer::ScrollValChange(int val){
    if (val > ui->scrollArea->verticalScrollBar()->maximum()*0.8){
        AutoScroll = true;
    } else {
        AutoScroll = false;
    }
}

void StreamViewer::EntryDisplayResize(){
    QList<QLabel*> labellist = ui->scrollAreaWidgetContents->findChildren<QLabel*>();

    for(int i = 0; i < labellist.count(); i++){
        RepaintEntry(EntryDisplayList[i], labellist[i]);
    }
}

void StreamViewer::EntryDisplay(EntryData dt){
    bool check = false;

    if ((dt.ID != "-") && (FilterList.count() > 0)){
        foreach(QString s, FilterList){
            if (dt.Stext.indexOf(s, 0, Qt::CaseInsensitive) != - 1){
                check = true;
                break;
            }
        }
    } else {
        check = true;
    }

    if (!check){
        return;
    }

    QList<QString> TextList;
    QList<QString> ChopppedS = dt.Stext.split(" ");
    QList<int> HeightList;
    int TotalHeight = 0;

    int outlinewidth = 1;

    QFont font = Displayfont;
    if(font.pixelSize() > 30){
        outlinewidth = 2;
    }

    QPainterPath path1;
    QFontMetrics fontmetric(font);

    while (ChopppedS.count() != 0) {
        QString S = "";
        int i;
        for(i = 0; fontmetric.boundingRect(S + ChopppedS[i]).width() < ui->scrollAreaWidgetContents->width() - 40; i++){
            if (i == ChopppedS.count() - 1){
                if (S != ""){
                    S += " " + ChopppedS[i];
                } else {
                    S += ChopppedS[i];
                }
                ChopppedS.clear();
                i = -1;
                break;
            } else {
                if (S != ""){
                    S += " " + ChopppedS[i];
                } else {
                    S += ChopppedS[i];
                }
            }
        }

        if (i == 0){
            S = ChopppedS[i];
            ChopppedS.removeFirst();
        }
        //qDebug() << S << " " << i;
        TextList.append(S);
        HeightList.append(fontmetric.boundingRect(S).height());
        TotalHeight += HeightList[HeightList.count() - 1];

        while (i > 0){
            ChopppedS.removeFirst();
            i--;
        }
    }

    //qDebug() << fontmetric.boundingRect().width();

    QImage image(ui->scrollAreaWidgetContents->width() - 10, TotalHeight + 10, QImage::Format_ARGB32);
    image.fill(Qt::transparent);

    /*
    if (BGSheetBool){
        image.fill(QColor(0, 0, 0, 120));
    } else {
        image.fill(Qt::transparent);
    }
    */

    QPoint p(20, 1);

    for (int i = 0; i < TextList.count(); i++){
        p += QPoint(0, HeightList[i]);
        path1.addText(p, font, TextList[i]);
    }

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);

    QPen pen(Qt::black, outlinewidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    if (dt.OC != ""){
        pen.setBrush(QColor("#" + dt.OC));
    }
    painter.setPen(pen);

    painter.setBrush(Qt::white);
    if (dt.CC != ""){
        painter.setBrush(QColor("#" + dt.CC));
    }
    painter.drawPath(path1);

    painter.end();

    QLabel *label = new QLabel(ui->scrollAreaWidgetContents);
    label->setPixmap(QPixmap::fromImage(image));

    if (ui->verticalLayout->count() > 9){
        QWidget *wid = ui->verticalLayout->takeAt(0)->widget();
        wid->setParent(nullptr);
        delete (wid);
        EntryDisplayList.removeFirst();
    }

    ui->verticalLayout->addWidget(label);

    EntryDisplayList.append(dt);
}

void StreamViewer::RepaintEntry(EntryData dt, QLabel *label){
    QList<QString> TextList;
    QList<QString> ChopppedS = dt.Stext.split(" ");
    QList<int> HeightList;
    int TotalHeight = 0;

    int outlinewidth = 1;

    QFont font = Displayfont;
    if(font.pixelSize() > 30){
        outlinewidth = 2;
    }

    QPainterPath path1;
    QFontMetrics fontmetric(font);

    while (ChopppedS.count() != 0) {
        QString S = "";
        int i;
        for(i = 0; fontmetric.boundingRect(S + ChopppedS[i]).width() < ui->scrollAreaWidgetContents->width() - 40; i++){
            if (i == ChopppedS.count() - 1){
                if (S != ""){
                    S += " " + ChopppedS[i];
                } else {
                    S += ChopppedS[i];
                }
                ChopppedS.clear();
                i = -1;
                break;
            } else {
                if (S != ""){
                    S += " " + ChopppedS[i];
                } else {
                    S += ChopppedS[i];
                }
            }
        }

        if (i == 0){
            S = ChopppedS[i];
            ChopppedS.removeFirst();
        }
        TextList.append(S);
        HeightList.append(fontmetric.boundingRect(S).height());
        TotalHeight += HeightList[HeightList.count() - 1];

        while (i > 0){
            ChopppedS.removeFirst();
            i--;
        }
    }

    QImage image(ui->scrollAreaWidgetContents->width() - 10, TotalHeight + 10, QImage::Format_ARGB32);
    image.fill(Qt::transparent);

    /*
    if (BGSheetBool){
        image.fill(QColor(0, 0, 0, 120));
    } else {
        image.fill(Qt::transparent);
    }
    */

    QPoint p(20, 1);

    for (int i = 0; i < TextList.count(); i++){
        p += QPoint(0, HeightList[i]);
        path1.addText(p, font, TextList[i]);
    }

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);

    QPen pen(Qt::black, outlinewidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    if (dt.OC != ""){
        pen.setBrush(QColor("#" + dt.OC));
    }
    painter.setPen(pen);

    painter.setBrush(Qt::white);
    if (dt.CC != ""){
        painter.setBrush(QColor("#" + dt.CC));
    }
    painter.drawPath(path1);

    painter.end();

    label->setPixmap(QPixmap::fromImage(image));
}

void StreamViewer::ProcessCustomEntry(double stime, QString stext, QString OC, QString CC, QString STags){
    if ((STags.indexOf("mod") != -1) || (STags.indexOf("owner") != -1)){
        EntryDisplay({"LIVECHAT", stime, stext, CCmod, OCmod});
    } else {
        if (OC == "") {
            OC = OCFilter;
        }

        if (CC == "") {
            CC = CCFilter;
        }

        EntryDisplay({"LIVECHAT", stime, stext, CC, OC});
    }
}
//============================================= ENTRY DISPLAY HANDLER ===============================================
