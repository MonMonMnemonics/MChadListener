#ifndef FILTERANDSTYLING_H
#define FILTERANDSTYLING_H

#include <QDialog>

namespace Ui {
class FilterandStyling;
}

class FilterandStyling : public QDialog
{
    Q_OBJECT

public:
    explicit FilterandStyling(QWidget *parent = nullptr);
    ~FilterandStyling();

private:
    Ui::FilterandStyling *ui;

private slots:
    void Closebtn();
    void SaveSyncBtnClick();
    void ModCCBtnClick();
    void ModOCBtnClick();
    void CustomCCBtnClick();
    void CustomOCBtnClick();
    void SaveDefaultClick();
    void ResetDef();
    void ReloadDefault();
};

#endif // FILTERANDSTYLING_H
