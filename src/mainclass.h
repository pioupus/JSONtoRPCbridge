#ifndef MAINCLASS_H
#define MAINCLASS_H

#include "jsoninput.h"
#include "rpcprotocol.h"
#include "rpcserialport.h"
#include "serialworker.h"

#include <QObject>
#include <QTextStream>
#include <QThread>

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

class ApplicationClass : public QObject {
    Q_OBJECT
    public:
    explicit ApplicationClass(QString port_name, uint baud, QString xml_path, std::chrono::_V2::steady_clock::duration timeout, QObject *parent = 0);
    void test();

    private slots:
    void input_received(QString str);

    private:
    QThread *console_input_thread = nullptr;
    QThread *serial_input_thread = nullptr;
    std::unique_ptr<JsonInput> json_input;
    RPCSerialPort serial_port;

    SerialThread* serialThread;

    std::unique_ptr<RPCProtocol> rpc_protocol;
};

#endif // MAINCLASS_H
