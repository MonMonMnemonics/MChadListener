#include "archiveviewer.h"
#include "ui_archiveviewer.h"
#include "sharedvar.h"
#include <QWidget>
#include <QResizeEvent>
#include <QTimer>
#include <QtMath>
#include <QHBoxLayout>
#include <QScrollBar>
#include <QInputDialog>
#include <QFontDialog>
#include <QJsonObject>
#include <QJsonDocument>
#include <QColorDialog>

ArchiveViewer::ArchiveViewer(QWidget *parent, int startingpoint) :
    QWidget(parent),
    ui(new Ui::ArchiveViewer)
{
    this->setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
    this->setParent(nullptr); // Create TopLevel-Widget
    this->setAttribute(Qt::WA_NoSystemBackground, true);
    this->setAttribute(Qt::WA_TranslucentBackground, true);

    //      FLUSH STARTING CONDITION
    ui->setupUi(this);
    startindex = startingpoint;
    CurrentIdx = startindex;
    EntryDisplayIndex.clear();
    ui->scrollAreaWidgetContents->setLayout(ui->verticalLayout);
    ui->FontSizeBtn->setText("Font");
    ui->ForwardBtn->setText("Forward (" + QString::number(forwardmod) + ")");
    ui->SeekSlider->setMinimum(0);
    ui->SeekSlider->setMaximum(int(floor((Entries[Entries.count() - 1].Stime - Entries[startingpoint].Stime)/1000)));
    ui->scrollArea->setStyleSheet("background-color:#" + BGSheetColour + ";");

    Mousechecker = new QTimer(this);
    EntryTimer = new QTimer(this);

    //      CONNECTION CONFIGURATIONS
    connect(ui->BackBtn, SIGNAL(released()), this, SLOT(Closebtn()));
    connect(ui->SyncBtn, SIGNAL(released()), this, SLOT(SyncBtnClick()));
    connect(ui->PlayBtn, SIGNAL(released()), this, SLOT(PlayBtnClick()));
    connect(ui->StopBtn, SIGNAL(released()), this, SLOT(StopBtnClick()));
    connect(ui->BGSheetClr, SIGNAL(released()), this, SLOT(BGSheetClrBtnClick()));
    connect(ui->ForwardBtn, SIGNAL(released()), this, SLOT(ForwardModClick()));
    connect(ui->FontSizeBtn, SIGNAL(released()), this, SLOT(FontSizeBtnClick()));
    connect(Mousechecker, SIGNAL(timeout()), this, SLOT(MenuOpen()));
    connect(EntryTimer, SIGNAL(timeout()), this, SLOT(TimerPlay()));
    connect(ui->TPcheckBox, SIGNAL(stateChanged(int)), this, SLOT(TPCheckclick(int)));
    connect(ui->BGSheet, SIGNAL(stateChanged(int)), this, SLOT(BGSheetClick(int)));
    connect(ui->scrollArea->verticalScrollBar(), SIGNAL(rangeChanged(int,int)), this, SLOT(ScrollToBottom(int, int)));
    connect(ui->scrollArea->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(ScrollValChange(int)));
    connect(ui->SeekSlider, SIGNAL(sliderMoved(int)), this, SLOT(SeekSliderMove(int)));
    connect(ui->SeekSlider, SIGNAL(sliderReleased()), this, SLOT(SeekSliderRelease()));
    connect(ui->SeekSlider, SIGNAL(sliderPressed()), this, SLOT(SeekSliderPressed()));

    Mousechecker->start(FPS);
    EntryTimer->start(int(FPS));

    EntryDisplay(CurrentIdx);
}

ArchiveViewer::~ArchiveViewer()
{
    delete ui;
}

void ArchiveViewer::BGSheetClrBtnClick(){
    QColor DialogColor = QColorDialog::getColor(QColor("#" + BGSheetColour),
                                            this, "BGSHEET COLOUR", QColorDialog::ShowAlphaChannel);

    if (DialogColor.isValid())   {
       BGSheetColour = DialogColor.name(QColor::HexArgb).replace(QRegExp("#"), "");
       ui->scrollArea->setStyleSheet("background-color:#" + BGSheetColour + ";");
       Overwriteconfig();
    }

}

void ArchiveViewer::Closebtn(){
    EntryTimer->stop();
    Mousechecker->stop();
    Arpicker->show();
    if (SocketOn){
        m_pWebSocketServer->pauseAccepting();
        m_pWebSocketServer->~QWebSocketServer();
        m_clients.clear();
        SocketOn = false;
    }
    this->close();
}

void ArchiveViewer::TPCheckclick(int mode){
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

void ArchiveViewer::resizeEvent(QResizeEvent* event){
    if (event->oldSize() != QSize(-1,-1)){
        ui->stackedWidget->resize(this->window()->width() - ui->stackedWidget->pos().x(), ui->stackedWidget->height());
        ui->SeekSlider->resize(this->window()->width() - ui->SeekSlider->pos().x() - 10, ui->SeekSlider->height());
        ui->scrollArea->resize(this->window()->width() - ui->scrollArea->pos().x() - 5, this->window()->height() - ui->scrollArea->pos().y());

        ui->SyncBtn->move(this->window()->width() - 110, ui->SyncBtn->pos().y());
        ui->PlayBtn->move(this->window()->width() - 160, ui->PlayBtn->pos().y());
        ui->TimeLabel->move(this->window()->width() - 250, ui->TimeLabel->pos().y());
        ui->StopBtn->move(this->window()->width() - 300, ui->StopBtn->pos().y());
        ui->TimeLabelQuick->move(this->window()->width() - 110, ui->TimeLabelQuick->y());
        if (event->size().width() != event->oldSize().width()){
            EntryDisplayResize();
        }
    }
}

void ArchiveViewer::mousePressEvent(QMouseEvent *event){
    mpos = event->pos();
}

void ArchiveViewer::mouseMoveEvent(QMouseEvent *event){
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

void ArchiveViewer::mouseReleaseEvent(QMouseEvent *event){
    Q_UNUSED(event);
    if (ResizeDrag){
        ResizeDrag = false;
    }
}

void ArchiveViewer::keyReleaseEvent(QKeyEvent *event){
    if (event->key() == 16777234){
        quickview = 20;
        if (CurrentTime < 3000){
            SeekTime(0, true);
        } else {
            SeekTime(CurrentTime - 3000, true);
        }
    } else if (event->key() == 16777236){
        quickview = 20;
        SeekTime(CurrentTime + 3000, true);
    }
}

void ArchiveViewer::MenuOpen(){
    if ((this->cursor().pos().y() < ui->stackedWidget->height() + this->pos().y())
            && (this->cursor().pos().y() > this->pos().y())
            && (this->cursor().pos().x() < ui->stackedWidget->width() + this->pos().x())
            && (this->cursor().pos().x() > this->pos().x())){
        ui->stackedWidget->setCurrentIndex(1);
    } else if (quickview > 0) {
        ui->stackedWidget->setCurrentIndex(2);
        ui->TimeLabelQuick->setText(ui->TimeLabel->text());
        quickview -= 1;
    } else {
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

void ArchiveViewer::FontSizeBtnClick(){
    bool ok = false;
    QFont tempfont = QFontDialog::getFont(&ok, Displayfont, this);

    if (ok) {
        Displayfont = tempfont;
        EntryDisplayResize();
        Overwriteconfig();
    }
}

void ArchiveViewer::PlayBtnClick(){
    if (SocketOn){
        BroadcastMessage("{\"Act\":\"MChad-PlayApp\",\"UID\":\"" + UIDSocket + "\"}");
    }
    EntryTimer->start(FPS);
}

void ArchiveViewer::StopBtnClick(){
    if (SocketOn){
        BroadcastMessage("{\"Act\":\"MChad-PauseApp\",\"UID\":\"" + UIDSocket + "\"}");
    }
    EntryTimer->stop();
}

QString ArchiveViewer::StringifyTime(double timestamp){
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

void ArchiveViewer::BGSheetClick(int mode){
    if (mode == 2){
        ui->scrollArea->setStyleSheet("background-color:#" + BGSheetColour + ";");
    } else {
        ui->scrollArea->setStyleSheet("background-color:transparent;");
    }
}

void ArchiveViewer::ForwardModClick(){
    bool ok = false;
    QString DialogText = QInputDialog::getText(this, tr("Forward Modifier"),
                                         tr("Forward Modifier in second:"), QLineEdit::Normal,
                                         QString::number(forwardmod/1000), &ok, Qt::WindowStaysOnTopHint, Qt::ImhDigitsOnly);

    if (ok && !DialogText.isEmpty()) {
        bool success = false;
        double res = DialogText.toDouble(&success);
        if (success){
            if (res > 0){
                forwardmod = res*1000;
                ui->ForwardBtn->setText("Forward (" + QString::number(forwardmod/1000) + ")");
            }
        }
    }
}

void ArchiveViewer::SeekSliderMove(int value){
    ui->TimeLabel->setText(StringifyTime(value*1000));
}

void ArchiveViewer::SeekSliderRelease(){
    SeekTime(ui->SeekSlider->value()*1000, true);
    if (TimerActive){
        EntryTimer->start(FPS);
    }
}

void ArchiveViewer::SeekSliderPressed(){
    if (EntryTimer->isActive()){
        TimerActive = true;
        EntryTimer->stop();
    } else {
        TimerActive = false;
    }
}

void ArchiveViewer::SyncBtnClick(){
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

void ArchiveViewer::Overwriteconfig(){
    QFile file("AppConfig.ini");

    if(file.open(QIODevice::WriteOnly)){
        QTextStream stream(&file);
        stream << "Font=" + Displayfont.toString() << endl;
        stream <<  "BG Sheet Colour=" << BGSheetColour << endl;
        file.close();
    }
}



//----------------------------------------------- WEB SOCKET HANDLER -------------------------------------------------
void ArchiveViewer::onNewConnection()
{
    QWebSocket *pSocket = m_pWebSocketServer->nextPendingConnection();

    connect(pSocket, SIGNAL(textMessageReceived(QString)), this, SLOT(processTextMessage(QString)));
    connect(pSocket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));
    ui->SyncBtn->setText("Sync (ON)");

    m_clients << pSocket;
}


void ArchiveViewer::processTextMessage(QString message)
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

            pClient->sendTextMessage("{\"Act\":\"MChad-SetMode\",\"UID\":\"" + UIDSocket + "\",\"Mode\":\"Archive\"}");
        } else if (ChoppedMsg[0].indexOf(":\"MChad-Play") != -1) {
            if (ChoppedMsg.length() != 2){
                return;
            }

            if (ChoppedMsg[1].split("\":\"")[1].replace(QRegExp("[\"}]"),"") == UIDSocket){
                EntryTimer->start(FPS);
            }
        } else if (ChoppedMsg[0].indexOf(":\"MChad-Pause") != -1) {
            if (ChoppedMsg.length() != 2){
                return;
            }

            if (ChoppedMsg[1].split("\":\"")[1].replace(QRegExp("[\"}]"),"") == UIDSocket){
                EntryTimer->stop();
            }
        } else if (ChoppedMsg[0].indexOf(":\"MChad-TimeSet") != -1) {
            if (ChoppedMsg.length() != 3){
                return;
            }
            if (ChoppedMsg[1].split("\":\"")[1].replace(QRegExp("\""),"") == UIDSocket){
                bool success;
                double timenew = ChoppedMsg[2].split("\":")[1].replace("}", "").toDouble(&success);
                if (success){
                    quickview = 20;
                    SeekTime(timenew*1000, false);
                }
            }
        }

    }
}

void ArchiveViewer::socketDisconnected()
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());

    if (pClient) {
        m_clients.removeAll(pClient);
        pClient->deleteLater();
    }
}

void ArchiveViewer::BroadcastMessage(QString msg){
    foreach(QWebSocket *socket, m_clients){
        socket->sendTextMessage(msg);
    }
}
//=============================================== WEB SOCKET HANDLER =================================================



//--------------------------------------------- ENTRY DISPLAY HANDLER -----------------------------------------------
void ArchiveViewer::timeGUIrefresh(){
    ui->TimeLabel->setText(StringifyTime(CurrentTime));
    ui->TimeLabelQuick->setText(ui->TimeLabel->text());
    int newval = int(floor(CurrentTime/1000));
    if (newval < 0){
        ui->SeekSlider->setValue(0);
    } else if (newval > ui->SeekSlider->maximum()){
        ui->SeekSlider->setValue(ui->SeekSlider->maximum());
    } else {
        ui->SeekSlider->setValue(int(floor(CurrentTime/1000)));
    }

}

void ArchiveViewer::TimerPlay(){
    CurrentTime += FPS;
    timeGUIrefresh();

    if (CurrentIdx < Entries.count() - 1){
        if (CurrentTime + forwardmod > Entries[CurrentIdx + 1].Stime - Entries[startindex].Stime){
            EntryDisplay(++CurrentIdx);
        }
    }
}

void ArchiveViewer::ScrollToBottom(int min, int max)
{
    Q_UNUSED(min);
    if (AutoScroll){
        ui->scrollArea->verticalScrollBar()->setValue(max);
    }
}

void ArchiveViewer::ScrollValChange(int val){
    if (val > ui->scrollArea->verticalScrollBar()->maximum()*0.8){
        AutoScroll = true;
    } else {
        AutoScroll = false;
    }
}

void ArchiveViewer::EntryDisplayResize(){
    QList<QLabel*> labellist = ui->scrollAreaWidgetContents->findChildren<QLabel*>();

    for(int i = 0; i < labellist.count(); i++){
        RepaintEntry(EntryDisplayIndex[i], labellist[i]);
    }
}

void ArchiveViewer::EntryDisplay(int index){
    QList<QString> TextList;
    QList<QString> ChopppedS = Entries[index].Stext.split(" ");
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
    if (Entries[index].OC != ""){
        pen.setBrush(QColor("#" + Entries[index].OC));
    }
    painter.setPen(pen);

    painter.setBrush(Qt::white);
    if (Entries[index].CC != ""){
        painter.setBrush(QColor("#" + Entries[index].CC));
    }
    painter.drawPath(path1);

    painter.end();

    QLabel *label = new QLabel(ui->scrollAreaWidgetContents);
    label->setPixmap(QPixmap::fromImage(image));

    if (ui->verticalLayout->count() > 9){
        QWidget *wid = ui->verticalLayout->takeAt(0)->widget();
        wid->setParent(nullptr);
        delete (wid);
        EntryDisplayIndex.removeFirst();
    }

    ui->verticalLayout->addWidget(label);

    EntryDisplayIndex.append(index);
}

void ArchiveViewer::RepaintEntry(int index, QLabel *label){
    QList<QString> TextList;
    QList<QString> ChopppedS = Entries[index].Stext.split(" ");
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
    if (Entries[index].OC != ""){
        pen.setBrush(QColor("#" + Entries[index].OC));
    }
    painter.setPen(pen);

    painter.setBrush(Qt::white);
    if (Entries[index].CC != ""){
        painter.setBrush(QColor("#" + Entries[index].CC));
    }
    painter.drawPath(path1);

    painter.end();

    label->setPixmap(QPixmap::fromImage(image));
}

void ArchiveViewer::SeekTime(double timestamp, bool broadcast){
    bool timerON = EntryTimer->isActive();

    if (broadcast){
        BroadcastMessage("{\"Act\":\"MChad-TimeSetApp\",\"UID\":\"" + UIDSocket + "\",\"Time\":\"" + QString::number(floor(timestamp/1000)) + "\"}");
    }

    if (timerON){
        EntryTimer->stop();
    }

    if (ui->verticalLayout->count() > 0){
        while( ui->verticalLayout->count() > 0){
            QWidget *wid = ui->verticalLayout->takeAt(0)->widget();
            wid->setParent(nullptr);
            delete (wid);
            EntryDisplayIndex.removeFirst();

            if (ui->verticalLayout->count() == 0){
                for (int i = 0; i<Entries.count(); i++){
                    if (i == Entries.count() - 1){
                        for(int j = i - 2; j != i + 1; j++){
                            if ((j >= 0) && (j < Entries.count()) && (j >= startindex)){
                                AutoScroll = true;
                                EntryDisplay(j);
                            }
                        }
                        CurrentIdx = i;
                        break;
                    } else if (Entries[i].Stime-Entries[startindex].Stime > timestamp + forwardmod){
                        for(int j = i - 3; j != i; j++){
                            if ((j >= 0) && (j < Entries.count()) && (j >= startindex)){
                                AutoScroll = true;
                                EntryDisplay(j);
                            }
                        }
                        CurrentIdx = i - 1;
                        break;
                    }
                }

                CurrentTime = timestamp;
                timeGUIrefresh();
                if (timerON){
                    EntryTimer->start(FPS);
                }
                break;
            }
        }
    } else {
        for (int i = 0; i<Entries.count(); i++){
            AutoScroll = true;
            if (i == Entries.count() - 1){
                for(int j = i - 2; j != i +1; j++){
                    if ((j >= 0) && (j < Entries.count()) && (j >= startindex)){
                        AutoScroll = true;
                        EntryDisplay(j);
                    }
                }
                CurrentTime = timestamp;
                timeGUIrefresh();
                CurrentIdx = i;
                if (timerON){
                    EntryTimer->start(FPS);
                }
                break;
            } else if (Entries[i].Stime-Entries[startindex].Stime > timestamp + forwardmod){
                for(int j = i - 3; j != i; j++){
                    if ((j >= 0) && (j < Entries.count()) && (j >= startindex)){
                        AutoScroll = true;
                        EntryDisplay(j);
                    }
                }
                CurrentTime = timestamp;
                timeGUIrefresh();
                CurrentIdx = i - 1;
                if (timerON){
                    EntryTimer->start(FPS);
                }

                break;
            }
        }
    }
}
//============================================= ENTRY DISPLAY HANDLER ===============================================
