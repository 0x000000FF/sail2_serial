#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QTimer>

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
    void on_pushButton_cmopen_clicked();
    void com_update();

    void on_pushButton_send_clicked();

    void on_comboBox_activated(const QString &arg1);

    void on_comboBox_highlighted(const QString &arg1);

    void Widget::state_change(int btn_num);

    void on_pushButton_freshen_clicked();

    void on_Button_runstop_clicked();

    void on_Button_del_clicked();
private:
    Ui::Widget *ui;
    QSerialPort *my_serialport;
    QByteArray requestData;
    QTimer *timer;
};

#endif // WIDGET_H
