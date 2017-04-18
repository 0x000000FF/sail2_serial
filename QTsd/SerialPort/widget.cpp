#include "widget.h"
#include "ui_widget.h"
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QDebug>
#include <QTimer>
QSerialPort serial;
QString com_tmp;
QByteArray TxData;
QByteArray requestData;
int currentRowcs;

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    ui->frame_file->setEnabled(false);
    ui->frame_sdin->setEnabled(false);
    my_serialport= new QSerialPort();
    timer = new QTimer(this);
    com_tmp.clear();

    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        qDebug() << "Name        : " << info.portName();
        qDebug() << "Description : " << info.description();
        qDebug() << "Manufacturer: " << info.manufacturer();
        serial.setPort(info);
        ui->comboBox->addItem(info.portName());

    }
}

Widget::~Widget()
{
    delete ui;
}

void Widget::on_pushButton_cmopen_clicked()
{
    state_change(1);
}
void Widget::on_pushButton_freshen_clicked()
{
    state_change(2);
}
void Widget::on_Button_runstop_clicked()
{
    state_change(3);
}

void Widget::on_Button_del_clicked()
{
    state_change(4);
}

void Widget::com_update()
{
    requestData = my_serialport->readAll();
    if(requestData.isNull()==false)
    {
        ui->textEdit->append(requestData);
        com_tmp+=requestData;
        qDebug()<<"com_tmp.length:"<<com_tmp.length();
        //com_tmp=QString::fromLocal8Bit(requestData).toUtf8();
        qDebug()<<"com_tmp.length:"<<com_tmp.length();
        while(com_tmp.indexOf("ok\n",0))//找OK
        {
            //处理M21
            if(com_tmp.indexOf("SD card ok\n",0)>=0)//SD卡正常插入
            {
                QByteArray TxData = "M20\r\n";
                my_serialport->write(TxData);
                com_tmp.clear();
                break;
            }
            else if(com_tmp.indexOf("SD init fail\n",0)>=0)//SD卡未插入或无法初始化
            {
                ui->listWidget_file->clear();
                ui->frame_sdin->setEnabled(false);
                break;
            }
            //M20函数
            if(com_tmp.indexOf("Begin file list\n",0)>=0)
            {
                QString com_midtmp  = com_tmp .mid(com_tmp.indexOf("Begin file list\n",0)+16, com_tmp.indexOf("End file list\n",0)-17);
                qDebug()<<"file:"<<com_tmp.indexOf("Begin file list\n",0);
                qDebug()<<"file:"<<com_tmp.indexOf("End file list\n",0);
                qDebug()<<"file:"<<com_midtmp;
                QStringList strlist = com_midtmp.split("\n");
                ui->listWidget_file->clear();
                for(int i=0;i<strlist.count();i++)
                {
                    qDebug()<<"file:"<<strlist.at(i);
                    ui->listWidget_file->addItem(strlist.at(i));
                    //ui->listWidget_file->addItem(new QListWidgetItem(QIcon("E:/work/mostfun/sail2/forSDprint/QTsd/SerialPort/icon/battery-20.png"), strlist.at(i)));
                }
                //ui->listWidget_file->setViewMode(QListView::IconMode);
                ui->listWidget_file->setCurrentRow(0);
                if(strlist.count())
                    ui->frame_sdin->setEnabled(true);
                com_midtmp.detach();
                strlist.detach();
                com_tmp.clear();
                break;
            }
            else
            {
                ui->listWidget_file->clear();
                ui->frame_sdin->setEnabled(false);
                com_tmp.clear();
                break;
            }
           com_tmp.clear();
           break;
        }//while(com_tmp.indexOf("ok\n",0))//找OK

    }
    requestData.clear();
}

void Widget::on_pushButton_send_clicked()
{
    QByteArray TxData = "M20\r\n";
    my_serialport->write(TxData);
}

void Widget::on_comboBox_activated(const QString &arg1)
{
    ui->comboBox->clear();
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        qDebug() << "Name        : " << info.portName();
        qDebug() << "Description : " << info.description();
        qDebug() << "Manufacturer: " << info.manufacturer();
        ui->comboBox->addItem(info.portName());
    }
    ui->comboBox->setCurrentText(arg1);
}

void Widget::on_comboBox_highlighted(const QString &arg1)
{
    ui->comboBox->setCurrentText(arg1);
}
void Widget::state_change(int btn_num)
{
//状态A，串口关闭
//状态B，串口打开
//状态C，串口打开，SD卡未插入
//状态D，串口打开，SD卡插入，未打印
//状态E，串口打开，SD卡插入，正在打印
    switch(btn_num)
    {
        case 1:
        if(my_serialport->isOpen()==false)
        {
            qDebug()<<ui->comboBox->currentText();
            my_serialport->setPortName(ui->comboBox->currentText());
            my_serialport->open(QIODevice::ReadWrite);
            qDebug()<<ui->lineEdit->text().toInt();
            my_serialport->setBaudRate(ui->lineEdit->text().toInt());
            my_serialport->setDataBits(QSerialPort::Data8);
            my_serialport->setParity(QSerialPort::NoParity);
            my_serialport->setStopBits(QSerialPort::OneStop);
            my_serialport->setFlowControl(QSerialPort::NoFlowControl);
            if(my_serialport->isOpen())
            {
                ui->pushButton_cmopen->setText("断开连接");
                this->setWindowTitle("mostfun sail2 连接成功");
                ui->frame_com->setEnabled(false);
                ui->frame_file->setEnabled(true);
                ui->frame_sdin->setEnabled(false);
                connect(timer, SIGNAL(timeout()), this, SLOT(com_update()));
                timer->start(200);
                QByteArray TxData = "M110\r\nM21\r\n";
                my_serialport->write(TxData);
            }
            else
            {
                timer->stop();
            }
        }
        else
        {
            timer->stop();
            my_serialport->close();
            ui->pushButton_cmopen->setText("连接机器");
            this->setWindowTitle("mostfun sail2 断开连接");
            ui->frame_com->setEnabled(true);
            ui->frame_file->setEnabled(false);
            ui->frame_sdin->setEnabled(false);
        }
        break;
    case 2:
        TxData.clear();
        TxData = "M21\r\n";
        my_serialport->write(TxData);
        break;
    case 3:
        if(ui->listWidget_file->currentItem()->text().indexOf(".GCO",0)>0)
        {
            TxData.clear();
            QString tmp="M23"+ui->listWidget_file->currentItem()->text()+"\r\n";
            TxData = tmp.toLatin1();
            my_serialport->write(TxData);
            qDebug() << "TxData:" <<TxData;
        }
        break;
    //case 4:
        break;
    }

}
