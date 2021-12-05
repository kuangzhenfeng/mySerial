#include "mySerial.h"
#include "ui_myserial.h"
#include <QDebug>
#include <QFile>

MySerial::MySerial(QWidget *parent) : QWidget(parent), ui(new Ui::MySerial)
{
    ui->setupUi(this);

    mainSerial = new QSerialPort(this);
    portUpdate();

    portUpdateTimer = new QTimer();
    connect(portUpdateTimer, SIGNAL(timeout()), this, SLOT(portUpdate()));
    portUpdateTimer->start(1000);   //开启定时器，更新端口信息
}

MySerial::~MySerial()
{
    delete ui;
}

void MySerial::portUpdate()
{
    //qDebug() << "portUpdate()";

    if(mainSerial->isOpen())    //当前串口状态为打开状态
    {
        //在找到的有效串口号中遍历当前打开的串口号
        foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
        {
            //qDebug() << ui->portComboBox->currentText() << "  " << info.portName();
            if(ui->portComboBox->currentText() == info.portName())
                return;
        }
        //若找不到已打开的串口:表示当前打开的串口已失效
        emit ui->openPushButton->clicked();
        ui->portComboBox->removeItem(ui->portComboBox->currentIndex());
        ui->stateLabel->setText("串口已失效！");
    }
    else    //当前串口状态为关闭状态
    {
        //检查有效串口和combobox中的串口号个数是否不同
        //qDebug() << ui->portComboBox->count() << "  " << QSerialPortInfo::availablePorts().count();
        if(ui->portComboBox->count() != QSerialPortInfo::availablePorts().count())
        {
            ui->portComboBox->clear();
            foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
            {
                //qDebug() << ui->portComboBox->currentText() << "  " << info.portName();
                ui->portComboBox->addItem(info.portName());
            }
            ui->stateLabel->setText("串口列表已更新！");
        }
    }
}

void MySerial::on_openPushButton_clicked()
{
    if(mainSerial->isOpen())
    {
        mainSerial->close();
        ui->portComboBox->setEnabled(true);
        ui->baudComboBox->setEnabled(true);
        ui->parityComboBox->setEnabled(true);
        ui->dataComboBox->setEnabled(true);
        ui->stopComboBox->setEnabled(true);
        ui->openPushButton->setText("打开串口");
        ui->stateLabel->setText("串口已关闭");
    }
    else
    {
        if(ui->portComboBox->count() != 0)
        {
            mainSerial->setPortName(ui->portComboBox->currentText());
            mainSerial->setBaudRate(ui->baudComboBox->currentText().toInt());
            switch (ui->parityComboBox->currentIndex())
            {
            case 0: mainSerial->setParity(QSerialPort::NoParity); break;
            case 1: mainSerial->setParity(QSerialPort::EvenParity); break;
            case 2: mainSerial->setParity(QSerialPort::OddParity); break;
            case 3: mainSerial->setParity(QSerialPort::SpaceParity); break;
            case 4: mainSerial->setParity(QSerialPort::MarkParity); break;
            }
            switch (ui->dataComboBox->currentIndex())
            {
            case 0: mainSerial->setDataBits(QSerialPort::Data5); break;
            case 1: mainSerial->setDataBits(QSerialPort::Data6); break;
            case 2: mainSerial->setDataBits(QSerialPort::Data7); break;
            case 3: mainSerial->setDataBits(QSerialPort::Data8); break;
            }
            switch (ui->stopComboBox->currentIndex())
            {
            case 0: mainSerial->setStopBits(QSerialPort::OneStop); break;
            case 1: mainSerial->setStopBits(QSerialPort::OneAndHalfStop); break;
            case 2: mainSerial->setStopBits(QSerialPort::TwoStop); break;
            }
            mainSerial->setFlowControl(QSerialPort::NoFlowControl);

            if(!mainSerial->open(QIODevice::ReadWrite))     //串口打开失败，则不执行后面的操作
            {
                ui->stateLabel->setText("串口打开失败！");
                return;
            }
            connect(mainSerial, SIGNAL(readyRead()), this, SLOT(serialRecMsgEvent()));
            ui->portComboBox->setEnabled(false);
            ui->baudComboBox->setEnabled(false);
            ui->parityComboBox->setEnabled(false);
            ui->dataComboBox->setEnabled(false);
            ui->stopComboBox->setEnabled(false);
            ui->openPushButton->setText("关闭串口");
            ui->stateLabel->setText("串口已打开");
        }
        else
        {
            ui->stateLabel->setText("串口不存在！");
        }
    }
}

void MySerial::setStyle(const QString &style)
{
    QFile qss("../serial/style/" + style + ".qss");
    qss.open(QFile::ReadOnly);
    qApp->setStyleSheet(qss.readAll());
    qss.close();
}

//接收数据
void MySerial::serialRecMsgEvent()
{
    QByteArray buf;
    buf = mainSerial->readAll();
    receiveBytesCount += buf.count();
    ui->receiveBytesLabel->setText(QString::number(receiveBytesCount));    //更新接收字节数
    if(!buf.isEmpty())
    {
        if(ui->hexReceiveCheckBox->isChecked())
        {
            buf = buf.toHex(' ');
        }
        QString str;
        str = QString::fromLocal8Bit(buf);
        if(!ui->stopDisplayPushButton->isChecked())
        {
            ui->receiveTextEdit->moveCursor(QTextCursor::End);
            ui->receiveTextEdit->insertPlainText(str);
            //ui->receiveTextEdit->append(str);
        }
    }
}

//发送数据
void MySerial::on_sendPushButton_clicked()
{
    if(!mainSerial->isOpen())
    {
        ui->stateLabel->setText("请先打开串口！");
        return;
    }
    QString str;
    str = ui->sendTextEdit->toPlainText();
    if(ui->hexSendCheckBox->isChecked())
    {
        QStringList strList = str.split(' ');
        foreach (QString strTmp, strList)
        {
            str += strTmp.toInt(0, 16);
        }
    }
    QByteArray buf;
    buf = str.toLocal8Bit();

    mainSerial->write(buf);
    sendBytesCount += buf.count();
    ui->sendBytesLabel->setText(QString::number(sendBytesCount));    //更新发送字节数
    ui->sendTextEdit->clear();
}



void MySerial::on_clearCountPushButton_clicked()
{
    receiveBytesCount = 0;
    sendBytesCount = 0;

    ui->receiveBytesLabel->setText(QString::number(receiveBytesCount));
    ui->sendBytesLabel->setText(QString::number(sendBytesCount));
}

void MySerial::on_clearReceivePushButton_clicked()
{
    ui->receiveTextEdit->clear();
}

void MySerial::on_clearSendPushButton_clicked()
{
    ui->sendTextEdit->clear();
}
