#ifndef SERIALWORKER_H
#define SERIALWORKER_H

#include <QObject>
#include <QThread>
#include <QtSerialPort/QSerialPort>


struct SerialThread;
class SerialWorker : public QObject
{
    Q_OBJECT
public:
    explicit SerialWorker(SerialThread* serialThread, QObject *parent = 0);


public slots:

#if 1
    void openPort(QString name, int baudrate);
    void closePort();
    bool isPortOpened();
    void sendData(QByteArray data);
#endif
    void process();

signals:
    void finished();



private slots:
    void on_readyRead();
private:
    QSerialPort* serialport;
    QString linebuffer;

};

struct SerialThread : public QObject
{
    Q_OBJECT
public:
    explicit SerialThread(QObject *parent = 0);
    QThread* thread;
    //QSerialPort* serialport;
    SerialWorker * serialWorker;
    void open(QString name, int baudrate);
    void close();
    bool isOpen();

    void sendByteData(QByteArray data);
signals:

    void openPort(QString name, int baudrate);
    void closePort(void);
    bool isPortOpened();
    void sendData(QByteArray data);


public slots:

};

extern SerialWorker *serialWorkerForRPCFunc;

#endif // SERIALWORKER_H
