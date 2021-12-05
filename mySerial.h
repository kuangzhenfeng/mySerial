#ifndef SERIAL_H
#define SERIAL_H

#include <QWidget>
#include <QTimer>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

namespace Ui {
class MySerial;
}

class MySerial : public QWidget
{
    Q_OBJECT

public:
    explicit MySerial(QWidget *parent = 0);
    ~MySerial();

private slots:
    void portUpdate();
    void serialRecMsgEvent();

    void on_openPushButton_clicked();


    void on_sendPushButton_clicked();

    void on_clearCountPushButton_clicked();

    void on_clearReceivePushButton_clicked();

    void on_clearSendPushButton_clicked();

private:
    Ui::MySerial *ui;
    QSerialPort *mainSerial;
    QTimer *portUpdateTimer;
    uint32_t receiveBytesCount = 0;
    uint32_t sendBytesCount = 0;

    void setStyle(const QString &style);    //app换肤

};

#endif // SERIAL_H
