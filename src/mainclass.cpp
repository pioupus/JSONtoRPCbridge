#include "mainclass.h"
#include <QDebug>
#include <iostream>
#include <memory>

ConsoleInputWorker::ConsoleInputWorker()
    : in_stream{stdin} {}

ConsoleInputWorker::~ConsoleInputWorker() {}

void ConsoleInputWorker::process() {
    bool is_finished = true;
    while (is_finished) {
        QString str = in_stream.read(1);
        emit input_received(str);
    }
}

ApplicationClass::ApplicationClass(QString port_name, uint baud, QString xml_path, std::chrono::_V2::steady_clock::duration timeout, QObject *parent)
    : QObject(parent) {
    serial_port.connect(port_name, baud);
    rpc_protocol = std::make_unique<RPCProtocol>(serial_port, xml_path, timeout);

    json_input = std::make_unique<JsonInput>(rpc_protocol.get());
    console_input_thread = new QThread();
    ConsoleInputWorker *console_input_worker = new ConsoleInputWorker();
    console_input_worker->moveToThread(console_input_thread);
    connect(console_input_worker, SIGNAL(input_received(QString)), this, SLOT(input_received(QString)));
    connect(console_input_thread, SIGNAL(started()), console_input_worker, SLOT(process()));
    connect(console_input_worker, SIGNAL(finished()), console_input_thread, SLOT(quit()));
    connect(console_input_worker, SIGNAL(finished()), console_input_worker, SLOT(deleteLater()));
    connect(console_input_thread, SIGNAL(finished()), console_input_thread, SLOT(deleteLater()));
    console_input_thread->start();
}

void ApplicationClass::test() {
    qDebug() << "sdsfsd";
}

void ApplicationClass::input_received(QString str) {
    json_input->append_to_input_buffer(str);
}
