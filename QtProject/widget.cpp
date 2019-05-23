#include "widget.h"
#include "ui_widget.h"
#include "dialog.h"
#include <bits/stdc++.h>
#include <jack/jack.h>

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

}

Widget::~Widget()
{
    delete ui;
}

void Widget::on_ClientButton_clicked()
{
    system("gcc -o /home/pi/Qt_Projekt/test/client /home/pi/Qt_Projekt/test/test_client.c -l jack");
    system("/home/pi/Qt_Projekt/test/client &");
}

void Widget::on_ServerButton_clicked()
{
    system("pidof client >> /home/pi/Qt_Projekt/test/client.pid");
    system("kill $(cat /home/pi/Qt_Projekt/test/client.pid)");
}
void Widget::on_FilterType_activated(int index)
{
    filter = index;

}

void Widget::on_Parameter_clicked()
{
    Dialog d(this);
    d.set_Filter(filter);
    d.setModal(true);
    d.exec();
}
