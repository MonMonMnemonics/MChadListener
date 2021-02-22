#include "login.h"
#include "sharedvar.h"
#include <QApplication>
#include <QFile>

Login *Frontpage;
Archivepicker *Arpicker;
QList<EntryData> Entries = *(new QList<EntryData>());
bool ResizeDrag;
QString UIDSocket;
bool SocketOn;
QString WSTagList = "";
QString WSKeywords = "[EN],[英訳/EN]";
QString CCmod = "3431E8";
QString OCmod = "FFFFFF";
QString CCFilter = "FFFFFF";
QString OCFilter = "000000";
QString BGSheetColour = "32000000";
QFont Displayfont;
QNetworkAccessManager *NetworkManager = new QNetworkAccessManager();;
QList<StreamWindow> StreamWindowsList;
QList<NetworkReply> ReplyList;

int main(int argc, char *argv[])
{
    Displayfont.fromString("Source Code Pro,25,-1,5,75,0,0,0,0,0");

    if (QFile::exists("AppConfig.ini")){
        QFile file("AppConfig.ini");

        if(file.open(QIODevice::ReadOnly | QIODevice::Text)){
            QTextStream in(&file);
            while (!in.atEnd())
            {
                QString line = in.readLine();
                if (line.indexOf("Font=") != -1){
                    bool success = Displayfont.fromString(line.split("Font=")[1]);
                    if (!success){
                        Displayfont.fromString(line.split("Font=")[1]);
                    }
                } else if (line.indexOf("BG Sheet Colour=") != -1){
                    QColor res = QColor("#" + line.split("BG Sheet Colour=")[1]);
                    if (res.isValid()){
                        BGSheetColour = res.name(QColor::HexArgb).replace("#", "");
                    }
                }
            }
            file.close();
        }
    }

    if (QFile::exists("MSyncConfig.ini")){
        QFile file("MSyncConfig.ini");

        if(file.open(QIODevice::ReadOnly | QIODevice::Text)){
            QTextStream in(&file);
            while (!in.atEnd())
            {
                QString line = in.readLine();
                if (line.indexOf("Keywords=") != -1){
                    WSKeywords = line.split("Keywords=")[1].replace(QRegExp("\n"), "");
                } else if (line.indexOf("CCMod Colour=") != -1){
                    QColor res = QColor("#" + line.split("CCMod Colour=")[1]);
                    if (res.isValid()){
                        CCmod = res.name(QColor::HexRgb).replace("#", "");
                    }
                } else if (line.indexOf("OCMod Colour=") != -1){
                    QColor res = QColor("#" + line.split("OCMod Colour=")[1]);
                    if (res.isValid()){
                        OCmod = res.name(QColor::HexRgb).replace("#", "");
                    }
                } else if (line.indexOf("CCFilter Colour=") != -1){
                    QColor res = QColor("#" + line.split("CCFilter Colour=")[1]);
                    if (res.isValid()){
                        CCFilter = res.name(QColor::HexRgb).replace("#", "");
                    }
                } else if (line.indexOf("OCFilter Colour=") != -1){
                    QColor res = QColor("#" + line.split("OCFilter Colour=")[1]);
                    if (res.isValid()){
                        OCFilter = res.name(QColor::HexRgb).replace("#", "");
                    }
                }
            }
            file.close();
        }
    }


    QApplication a(argc, argv);
    Login w;
    w.show();

    return a.exec();
}
