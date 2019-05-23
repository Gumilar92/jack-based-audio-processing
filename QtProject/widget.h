#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

private slots:

    void on_ClientButton_clicked();

    void on_ServerButton_clicked();

    void on_FilterType_activated(int index);

    void on_Parameter_clicked();

private:
    Ui::Widget *ui;
    int filter;

};

#endif // WIDGET_H
