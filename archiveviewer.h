#ifndef ARCHIVEVIEWER_H
#define ARCHIVEVIEWER_H

#include <QWidget>
#include <QLabel>
#include <QtWebSockets/QWebSocketServer>
#include <QtWebSockets/QWebSocket>

namespace Ui {
class ArchiveViewer;
}

class ArchiveViewer : public QWidget
{
    Q_OBJECT

protected:
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

public:
    explicit ArchiveViewer(QWidget *parent = nullptr, int startingpoint = 0);
    ~ArchiveViewer() override;

private:
    Ui::ArchiveViewer *ui;
    QString StringifyTime(double timestamp);
    int startindex;
    QPoint mpos;

    int FPS = 1000/10;
    QTimer *Mousechecker;
    QTimer *EntryTimer;
    double CurrentTime = 0;
    int CurrentIdx;

    void Overwriteconfig();

    //   ENTRY DISPLAY HANDLER
    void EntryDisplay(int index);
    void EntryDisplayResize();
    void RepaintEntry(int index, QLabel *label);
    void SeekTime(double timestamp, bool broadcast);
    void timeGUIrefresh();
    QList<int> EntryDisplayIndex;
    bool AutoScroll = true;
    bool TimerActive = false;
    double forwardmod = 0;
    int quickview = 0;

    //  FRAMELESS RESIZEABLE
    int ResizeMode = 0;

    //  WEBSOCKET HANDLER
    void BroadcastMessage(QString msg);

private slots:
    void Closebtn();
    void MenuOpen();
    void SyncBtnClick();
    void PlayBtnClick();
    void StopBtnClick();
    void ForwardModClick();
    void FontSizeBtnClick();
    void SeekSliderMove(int value);
    void SeekSliderRelease();
    void SeekSliderPressed();
    void TPCheckclick(int mode);
    void BGSheetClick(int mode);
    void TimerPlay();
    void ScrollToBottom(int min, int max);
    void ScrollValChange(int val);
    void BGSheetClrBtnClick();

    //  WEBSOCKET HANDLER
    void onNewConnection();
    void processTextMessage(QString message);
    void socketDisconnected();

};

#endif // ARCHIVEVIEWER_H
