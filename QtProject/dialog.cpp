#include "dialog.h"
#include "ui_dialog.h"
#include "widget.h"
#include "kaiserbessel.h"
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
    Widget w(this);
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::on_calculateButton_clicked()
{

    kaiser_bessel kb;
    //setlocale(LC_NUMERIC, "en_US.UTF-8");
    if(ftype == 0){
        kb.set_fa(0);
        kb.set_fb(fb);
    }else if (ftype == 1){
        kb.set_fa(fa);
        kb.set_fb(24000);
    }

    float coefficients[kb.get_length()];
    kb.calculate_coefficients(coefficients);

    out_file = fopen("/home/pi/Qt_Projekt/test/out_coef.txt","w");
    if(out_file == nullptr){
        printf("there is no such file");
    }
    for(int i=0;i<length;i++){
        fprintf(out_file, "%f\n", coefficients[i]);
    }
    fclose(out_file);

}

void Dialog::on_hSlider_valueChanged(int value)
{
    if(ftype == 0){
        fb = value;
    }else if (ftype == 1){
        fa = value;
    }
}

void Dialog::set_Filter(int ftype){
    this->ftype = ftype;
    if(ftype == 0){
        ui->textEdit->setText("Tiefpass");
    }else if (ftype == 1){
        ui->textEdit->setText("Hochpass");
    }else {
        ui->textEdit->setText("Undefined");
    }
}
