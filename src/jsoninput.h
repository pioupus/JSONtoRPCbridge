#ifndef JSONINPUT_H
#define JSONINPUT_H

#include <QJsonObject>
#include <QList>
#include <QString>

class RPCProtocol;

class JsonInput {
    public:
    JsonInput(RPCProtocol *rpc_protocol_);

    void clear_input_buffer();
    bool append_to_input_buffer(QString data);

    QJsonObject get_last_json_object();

    void json_to_controll_command_execution(const QJsonObject &obj);
private:
    bool test_json_input(QString str);
    bool test_json_object(const QJsonObject &obj);

    void json_to_rpc_call(const QJsonObject &obj);
    void Json_to_cout(const QJsonObject & obj);
    QString input_buffer;
    QJsonObject last_json_object;

    RPCProtocol *rpc_protocol = nullptr;
};

#endif // JSONINPUT_H
