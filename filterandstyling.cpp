#include "filterandstyling.h"
#include "ui_filterandstyling.h"
#include "sharedvar.h"
#include <QColorDialog>
#include <QFile>
#include <QTimer>

FilterandStyling::FilterandStyling(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FilterandStyling)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::WindowStaysOnTopHint);
    this->setParent(nullptr); // Create TopLevel-Widget

    ui->ModCCBtn->setStyleSheet("border: 1px solid white; background-color:#" + CCmod + ";");
    ui->ModOCBtn->setStyleSheet("border: 1px solid white; background-color:#" + OCmod + ";");
    ui->CustomCCBtn->setStyleSheet("border: 1px solid white; background-color:#" + CCFilter + ";");
    ui->CustomOCBtn->setStyleSheet("border: 1px solid white; background-color:#" + OCFilter + ";");
    ui->KeywordInpt->setText(WSKeywords);
    ui->AuthInpt->setText(WSTagList);

    if ((OCFilter == "FFFFFF") && (CCFilter == "000000")) {
        ui->CustomColour->setCheckState(Qt::Unchecked);
    } else {
        ui->CustomColour->setCheckState(Qt::Checked);
    }

    //      CONNECTION CONFIGURATIONS
    connect(ui->Cancelbtn, SIGNAL(released()), this, SLOT(Closebtn()));
    connect(ui->SSbtn, SIGNAL(released()), this, SLOT(SaveSyncBtnClick()));
    connect(ui->ModCCBtn, SIGNAL(released()), this, SLOT(ModCCBtnClick()));
    connect(ui->ModOCBtn, SIGNAL(released()), this, SLOT(ModOCBtnClick()));
    connect(ui->CustomCCBtn, SIGNAL(released()), this, SLOT(CustomCCBtnClick()));
    connect(ui->CustomOCBtn, SIGNAL(released()), this, SLOT(CustomOCBtnClick()));
    connect(ui->ReloadDef, SIGNAL(released()), this, SLOT(ReloadDefault()));
    connect(ui->SetDef, SIGNAL(released()), this, SLOT(SaveDefaultClick()));
}

FilterandStyling::~FilterandStyling()
{
    delete ui;
}

void FilterandStyling::Closebtn(){
    this->close();
}

void FilterandStyling::SaveSyncBtnClick(){
    WSTagList = ui->AuthInpt->text();
    WSKeywords = ui->KeywordInpt->text();
    CCmod = OCFilter = ui->ModCCBtn->styleSheet().split("background-color:#")[1].replace(QRegExp(";"), "");
    OCmod = OCFilter = ui->ModOCBtn->styleSheet().split("background-color:#")[1].replace(QRegExp(";"), "");

    if (ui->CustomColour->checkState() == Qt::Checked){
        CCFilter = ui->CustomCCBtn->styleSheet().split("background-color:#")[1].replace(QRegExp(";"), "");
        OCFilter = ui->CustomOCBtn->styleSheet().split("background-color:#")[1].replace(QRegExp(";"), "");
    } else {
        OCFilter = "FFFFFF";
        CCFilter = "000000";
    }

    if (SocketOn){
        foreach(QWebSocket *socket, m_clients){
            socket->sendTextMessage("{\"Act\":\"MChad-FilterApp\",\"Atr\":\"TAG\",\"Val\":\"" + WSTagList + "\"}");
            socket->sendTextMessage("{\"Act\":\"MChad-FilterApp\",\"Atr\":\"KEYWORD\",\"Val\":\"" + WSKeywords + "\"}");
        }
    }

    this->close();
}

void FilterandStyling::ModCCBtnClick(){
    QColor DialogColor = QColorDialog::getColor(QColor(ui->ModCCBtn->styleSheet().split("background-color:")[1].replace(QRegExp(";"), "")),
                                            this, "MOD / OWNER FONT COLOUR");

    if (DialogColor.isValid())   {
       ui->ModCCBtn->setStyleSheet("border: 1px solid white; background-color:" + DialogColor.name(QColor::HexRgb) + ";");
    }
}

void FilterandStyling::ModOCBtnClick(){
    QColor DialogColor = QColorDialog::getColor(QColor(ui->ModOCBtn->styleSheet().split("background-color:")[1].replace(QRegExp(";"), "")),
                                            this, "MOD / OWNER OUTLINE COLOUR");

    if (DialogColor.isValid())   {
       ui->ModOCBtn->setStyleSheet("border: 1px solid white; background-color:" + DialogColor.name(QColor::HexRgb) + ";");
    }
}

void FilterandStyling::CustomCCBtnClick(){
    QColor DialogColor = QColorDialog::getColor(QColor(ui->CustomCCBtn->styleSheet().split("background-color:")[1].replace(QRegExp(";"), "")),
                                            this, "FONT COLOUR");

    if (DialogColor.isValid())   {
       ui->CustomCCBtn->setStyleSheet("border: 1px solid white; background-color:" + DialogColor.name(QColor::HexRgb) + ";");
    }
}

void FilterandStyling::CustomOCBtnClick(){
    QColor DialogColor = QColorDialog::getColor(QColor(ui->CustomOCBtn->styleSheet().split("background-color:")[1].replace(QRegExp(";"), "")),
                                            this, "OUTLINE COLOUR");

    if (DialogColor.isValid())   {
       ui->CustomOCBtn->setStyleSheet("border: 1px solid white; background-color:" + DialogColor.name(QColor::HexRgb) + ";");
    }
}

void FilterandStyling::SaveDefaultClick(){
    QFile file("MSyncConfig.ini");

    if(file.open(QIODevice::WriteOnly)){
        QTextStream stream(&file);
        stream << "Keywords=" + ui->KeywordInpt->text() << endl;
        stream << "CCMod Colour=" << ui->ModCCBtn->styleSheet().split("background-color:#")[1].replace(QRegExp(";"), "") << endl;
        stream << "OCMod Colour=" << ui->ModOCBtn->styleSheet().split("background-color:#")[1].replace(QRegExp(";"), "") << endl;
        stream << "CCFilter Colour=" << ui->CustomCCBtn->styleSheet().split("background-color:#")[1].replace(QRegExp(";"), "") << endl;
        stream << "OCFilter Colour=" << ui->CustomOCBtn->styleSheet().split("background-color:#")[1].replace(QRegExp(";"), "") << endl;
        file.close();
    }

    ui->SetDef->setText("SAVED !");
    QTimer::singleShot(1000, this, &FilterandStyling::ResetDef);
}

void FilterandStyling::ResetDef(){
    ui->SetDef->setText("Set Default");
}

void FilterandStyling::ReloadDefault(){
    if (QFile::exists("MSyncConfig.ini")){
        QFile file("MSyncConfig.ini");

        if(file.open(QIODevice::ReadOnly | QIODevice::Text)){
            QTextStream in(&file);
            while (!in.atEnd())
            {
                QString line = in.readLine();
                if (line.indexOf("Keywords=") != -1){
                    ui->KeywordInpt->setText(line.split("Keywords=")[1].replace(QRegExp("\n"), ""));
                } else if (line.indexOf("CCMod Colour=") != -1){
                    QColor res = QColor("#" + line.split("CCMod Colour=")[1]);
                    if (res.isValid()){
                        ui->ModCCBtn->setStyleSheet("border: 1px solid white; background-color:" + res.name(QColor::HexRgb) + ";");
                    }
                } else if (line.indexOf("OCMod Colour=") != -1){
                    QColor res = QColor("#" + line.split("OCMod Colour=")[1]);
                    if (res.isValid()){
                        ui->ModOCBtn->setStyleSheet("border: 1px solid white; background-color:" + res.name(QColor::HexRgb) + ";");
                    }
                } else if (line.indexOf("CCFilter Colour=") != -1){
                    QColor res = QColor("#" + line.split("CCFilter Colour=")[1]);
                    if (res.isValid()){
                        ui->CustomCCBtn->setStyleSheet("border: 1px solid white; background-color:" + res.name(QColor::HexRgb) + ";");
                    }
                } else if (line.indexOf("OCFilter Colour=") != -1){
                    QColor res = QColor("#" + line.split("OCFilter Colour=")[1]);
                    if (res.isValid()){
                        ui->CustomOCBtn->setStyleSheet("border: 1px solid white; background-color:" + res.name(QColor::HexRgb) + ";");
                    }
                }
            }
            file.close();
        }
    } else {
        ui->ModCCBtn->setStyleSheet("border: 1px solid white; background-color:#3431E8;");
        ui->ModOCBtn->setStyleSheet("border: 1px solid white; background-color:#FFFFFF;");
        ui->CustomCCBtn->setStyleSheet("border: 1px solid white; background-color:#FFFFFF;");
        ui->CustomOCBtn->setStyleSheet("border: 1px solid white; background-color:#000000;");
        ui->KeywordInpt->setText("[EN],[英訳/EN]");
    }
}
