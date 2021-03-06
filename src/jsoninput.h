#ifndef JSONINPUT_H
#define JSONINPUT_H

#include <QJsonObject>
#include <QList>
#include <QString>

typedef enum{json_not_ready,json_is_command,json_is_rpc} json_test_result_t;

class RPCProtocol;

class JsonInput {
    public:
    JsonInput(RPCProtocol *rpc_protocol_);

    void clear_input_buffer();
    bool append_to_input_buffer(QString data);

    QJsonObject get_last_json_object();

    void json_to_controll_command_execution(const QJsonObject &obj);
private:
    json_test_result_t test_json_object(const QJsonObject &obj);

    void json_to_rpc_call(const QJsonObject &obj);
    void Json_to_cout(const QJsonObject & obj);
    QString input_buffer;
    QJsonObject last_json_object;

    RPCProtocol *rpc_protocol = nullptr;
    QJsonObject parse_json_input(QString str);
};

#endif // JSONINPUT_H
