#ifndef ARCHIVEPICKER_H
#define ARCHIVEPICKER_H

#include <QWidget>
#include <archiveviewer.h>

namespace Ui {
class Archivepicker;
}

class Archivepicker : public QWidget
{
    Q_OBJECT

protected:
    void resizeEvent(QResizeEvent *event) override;

public:
    explicit Archivepicker(QWidget *parent = nullptr);
    ~Archivepicker() override;

private:
    Ui::Archivepicker *ui;
    ArchiveViewer *Arview;
    QString StringifyTime(double timestamp);

private slots:
    void Closebtn();
    void ClickLink(const QString &link);
};


#endif // ARCHIVEPICKER_H
