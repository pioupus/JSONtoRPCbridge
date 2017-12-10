#include "mainclass.h"
#include <QDebug>
#include <iostream>
#include <memory>
#include "qt_util.h"

ConsoleInputWorker::ConsoleInputWorker()
    : in_stream{stdin} {}

ConsoleInputWorker::~ConsoleInputWorker() {}

void ConsoleInputWorker::process() {
    bool is_finished = true;
    while (is_finished) {
        in_stream.device()->waitForReadyRead(100);
        QString data = in_stream.read(1);
        if (data.count()) {
            //qDebug() << "gotrequest:" << data;
            emit input_received(QString(data));
        }
#if 0
        qint64 return_value = in_stream.device()->read(&data, 1);

        if (return_value == 1) {

        } else if (return_value < 0) {
            qDebug() << "streamerror" << return_value;
        }
#endif
    }
}

ApplicationClass::ApplicationClass(QString port_name, uint baud, QString xml_path, std::chrono::_V2::steady_clock::duration timeout, QObject *parent)
    : QObject(parent) {
    console_input_thread = new QThread();
    serial_input_thread = new QThread();

    ConsoleInputWorker *console_input_worker = new ConsoleInputWorker();
    console_input_worker->moveToThread(console_input_thread);

    RPCSerialPort *serial_port = new RPCSerialPort();

    serial_port->connect(port_name, baud);

    connect(console_input_worker, SIGNAL(input_received(QString)), this, SLOT(input_received(QString)));
    connect(console_input_thread, SIGNAL(started()), console_input_worker, SLOT(process()));
    connect(console_input_worker, SIGNAL(finished()), console_input_thread, SLOT(quit()));
    connect(console_input_worker, SIGNAL(finished()), console_input_worker, SLOT(deleteLater()));
    connect(console_input_thread, SIGNAL(finished()), console_input_thread, SLOT(deleteLater()));

    serial_input_thread->start();

    rpc_protocol = std::make_unique<RPCProtocol>(*serial_port, xml_path, timeout);

    Utility::thread_call(this,  nullptr, [this] {
      //  QCoreApplication::processEvents();
        rpc_protocol->is_correct_protocol();
        console_input_thread->start();

    });

    json_input = std::make_unique<JsonInput>(rpc_protocol.get());




   // qDebug() << "mainclass threadID:" << QThread::currentThreadId();
}

void ApplicationClass::test() {
    //qDebug() << "sdsfsd";
}

void ApplicationClass::input_received(QString str) {
    json_input->append_to_input_buffer(str);
}
