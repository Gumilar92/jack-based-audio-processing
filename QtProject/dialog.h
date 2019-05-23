#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <stdlib.h>

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();

    void set_Filter(int ftype);

private slots:

    void on_calculateButton_clicked();

    void on_hSlider_valueChanged(int value);

private:
    Ui::Dialog *ui;
    int fa,fb,ftype,att = 60,length = 51;
    FILE *out_file;
};

#endif // DIALOG_H
