#ifndef MAINCLASS_H
#define MAINCLASS_H

#include "jsoninput.h"
#include "rpcserialport.h"

#include <QObject>
#include <QThread>
#include <QTextStream>

class ConsoleInputWorker : public QObject {
    Q_OBJECT
public:
    ConsoleInputWorker();
    ~ConsoleInputWorker();
public slots:
    void process();
signals:
    void finished();
    void input_received(QString str);
private:
    QTextStream in_stream;
};


class ApplicationClass : public QObject
{
    Q_OBJECT
public:
    explicit ApplicationClass( QString port_name, uint baud, QObject *parent = 0);
    void test();

private slots:
    void input_received(QString str);

private:
    QThread* console_input_thread = nullptr;
    JsonInput json_input;
    RPCSerialPort serial_port;
};

#endif // MAINCLASS_H
