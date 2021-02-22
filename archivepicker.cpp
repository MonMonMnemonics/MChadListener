#include "archivepicker.h"
#include "ui_archivepicker.h"
#include "sharedvar.h"
#include <QWidget>
#include <QResizeEvent>
#include <QtMath>

Archivepicker::Archivepicker(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Archivepicker)
{
    ui->setupUi(this);

    connect(ui->BackBtn, SIGNAL(released()), this, SLOT(Closebtn()));
    connect(ui->PageDisplay, SIGNAL(linkActivated(QString)), this, SLOT(ClickLink(QString)));

    QString S = "";
    double Starter = 0;

    for (int i = 0; i < Entries.count(); i++)    {
        if (i == 0){
            Starter = Entries[0].Stime;
        }

        S += StringifyTime(Entries[i].Stime - Starter) + " <a href=\"" + QString::number(i) + "\" style=\" text-decoration: none; color:#000000;\">" + Entries[i].Stext + "</a><br>";
    }

    ui->PageDisplay->setText(S);
    ui->PageDisplay->adjustSize();
    ui->scrollAreaWidgetContents->setMinimumSize(0, ui->PageDisplay->height());
    Arpicker = this;
}

Archivepicker::~Archivepicker()
{
    delete ui;
}

void Archivepicker::resizeEvent(QResizeEvent* event){
    if (event->oldSize() != QSize(-1,-1)){
        ui->Title->resize(this->window()->size().width() - ui->Title->pos().x() - 10, ui->Title->size().height());
        ui->scrollArea->resize(this->window()->size().width() - ui->scrollArea->pos().x() - 20, this->window()->size().height() - ui->scrollArea->pos().y() - 20);
        //ui->PagePrev->move(ui->PagePrev->pos().x(), this->window()->size().height() - ui->PagePrev->size().height() - 10);
        //ui->PageNext->move(this->window()->size().width() - ui->PageNext->size().width() - 10, this->window()->size().height() - ui->PagePrev->size().height() - 10);
        ui->PageDisplay->resize(ui->scrollArea->size().width() - 10, ui->PageDisplay->heightForWidth(ui->scrollArea->size().width() - 20));
    }
}

void Archivepicker::ClickLink(const QString &link){
    Arview = new ArchiveViewer(nullptr, link.toInt());
    Arview->show();
    this->close();
}

void Archivepicker::Closebtn(){
    Frontpage->show();
    this->close();
}

QString Archivepicker::StringifyTime(double timestamp){
    QString S = "";
    int temp;

    temp = int(floor(timestamp/3600000));
    if (temp < 10){
        S += "0" + QString::number(temp) + ":";
    } else {
        S += QString::number(temp) + ":";
    }
    timestamp -= 3600000*temp;

    temp = int(floor(timestamp/60000));
    if (temp < 10){
        S += "0" + QString::number(temp) + ":";
    } else {
        S += QString::number(temp) + ":";
    }
    timestamp -= 60000*temp;

    temp = int(floor(timestamp/1000));
    if (temp < 10){
        S += "0" + QString::number(temp) + ".";
    } else {
        S += QString::number(temp) + ".";
    }
    timestamp -= 1000*temp;

    S += QString::number(timestamp);

    while (S.length() < 12) {
        S += "0";
    }

    return (S);
}
