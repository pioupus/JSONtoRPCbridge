#include "jsoninput.h"
#include "rpcprotocol.h"
#include "rpcruntime_decoded_function_call.h"
#include "rpcruntime_decoder.h"
#include "rpcruntime_encoded_function_call.h"
#include "rpcruntime_encoder.h"
#include "rpcruntime_protocol_description.h"
#include <QByteArray>
#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonValue>
#include <QThread>
#include <assert.h>
#include <vc.h>

JsonInput::JsonInput(RPCProtocol *rpc_protocol_)
    : rpc_protocol{rpc_protocol_} {
    //promised rpc_protocol->is_correct_protocol();
}

void JsonInput::clear_input_buffer() {
    input_buffer = "";
}

bool JsonInput::append_to_input_buffer(QString data) {
    if (data.size() == 0) {
        return false;
    }
    //  static uint32_t counter=0;
    //  qDebug() << data << counter++;
    bool result = false;

    input_buffer += data;
    bool input_not_empty = true;
    //qDebug() << "append_to_input_buffer" << data;
    while (input_not_empty) {
        json_test_result_t json_test_result = json_not_ready;
        QList<int> start_tag_positions;
        QList<int> stop_tag_positions;
        bool found = false;
        int i = -1;
        do {
            i = input_buffer.indexOf("/{", i + 1);
            if (i > -1) {
                start_tag_positions.append(i);
            }
        } while (i > -1);

        i = -1;
        do {
            i = input_buffer.indexOf("}/", i + 1);
            if (i > -1) {
                stop_tag_positions.append(i);
            }
        } while (i > -1);
        QJsonObject obj;
        for (auto start : start_tag_positions) {
            for (auto stop : stop_tag_positions) {
                if (stop < start) {
                    continue;
                }
                auto str = input_buffer.mid(start, stop - start + QString("}/").length());
                //qDebug() << "test JSON";
                //qDebug() << "jsontest threadID:" << QThread::currentThreadId();
                str = str.remove(0, 1);                // remove the / of the /{
                str = str.remove(str.length() - 1, 1); // remove the / of the }/

                obj = parse_json_input(str);

                json_test_result = test_json_object(obj);
                switch (json_test_result) {
                    case json_not_ready:
                  //      qDebug() << "JSON not ok";
                        break;
                    case json_is_command:
                    case json_is_rpc:
                    //    qDebug() << "JSON ok";
                        //qDebug() << "input_buffer" << input_buffer;
                        input_buffer.remove(0, stop + QString("}/").length());
                        //qDebug() << "input_buffer" << input_buffer;
                        result = true;
                        found = true;
                }
                if (found) {
                    break;
                }
            }
            switch (json_test_result) {
                case json_not_ready:
                    break;
                case json_is_command:
                    json_to_controll_command_execution(obj);

                    break;
                case json_is_rpc:
                    json_to_rpc_call(obj);
            }
            if (found) {
                break;
            }
        }
        if (found == false) {
            input_not_empty = false;
        }
    }
    return result;
}

QJsonObject JsonInput::get_last_json_object() {
    return last_json_object;
}

QJsonObject JsonInput::parse_json_input(QString str) {
    QJsonParseError p_err;

    //qDebug() << "test_json_input" << str;
    auto doc = QJsonDocument::fromJson(str.toUtf8(), &p_err);

    if (doc.isNull()) {
        qDebug() << "Json parser error. It says:" << p_err.errorString();
    }
    return doc.object();
}

json_test_result_t JsonInput::test_json_object(const QJsonObject &obj) {
   // qDebug() << "test_json_object1";
    if (obj.isEmpty()) {
        return json_not_ready;
    }
    //qDebug() << "test_json_object2";
    const QJsonObject &rpc = obj["rpc"].toObject();
    const QJsonObject &ctrl = obj["controll"].toObject();

#if 0
    {
        /{
            "rpc":{
                "timeout_ms":100,
                "request":{
                    "function": "test_function",
                    "arguments": {
                        "param_int1": 1,
                        "param_enum1": "enum_val_test",
                        "param_struct1": {
                            "field_int1":1,
                            "field_int2":2
                        }
                    }
                }
            }
        }/

        /{
            "rpc":{
                "timeout_ms":100,
                "request":{
                    "function": "get_adc_values",
                    "arguments": {


                    }
                }
            }
        }/
    }

    {
        /{
        "controll":{
            "command":"quit",
            "arguments": {
            }
        }/

        /{
        "controll":{
            "command":"get_hash",
            "result": {
                "hash":"sdfsdfsdfsdfsdf"
            }
        }/
    }
#endif
#if 0
    qDebug() << "received: " << obj;
#endif
    if (!rpc.isEmpty()) {
        if (!rpc.contains("timeout_ms")) {
            return json_not_ready;
        }

        const QJsonObject &request = rpc["request"].toObject();
        if (request.isEmpty()) {
            return json_not_ready;
        }

        if (!request.contains("function")) {
            return json_not_ready;
        }

        if (!request.contains("arguments")) {
            return json_not_ready;
        }
        //qDebug() << "test_json_object3";

        //qDebug() << "test_json_object4";
        return json_is_rpc;
    } else if (!ctrl.isEmpty()) {
        if (!ctrl.contains("command")) {
            return json_not_ready;
        }

        return json_is_command;
    }
    return json_not_ready;
}

static void set_runtime_parameter(RPCRuntimeEncodedParam &param, QJsonValue jval) {
    //qDebug() <<
    if (param.get_description()->get_type() == RPCRuntimeParameterDescription::Type::array && param.get_description()->as_array().number_of_elements == 1) {
        return set_runtime_parameter(param[0], jval);
    }

    switch (jval.type()) {
        case QJsonValue::Type::Bool:
            param.set_value(jval.toBool());
            break;

        case QJsonValue::Type::Double:
            param.set_value(jval.toInt());
            break;
        case QJsonValue::Type::Null:
        case QJsonValue::Type::Undefined:
            throw std::runtime_error("Cannot pass an object of type nil to RPC");
        case QJsonValue::Type::String:
            param.set_value(jval.toString().toStdString());
            break;
        case QJsonValue::Type::Array: {
            const QJsonArray &arr = jval.toArray();

            for (int i = 0; i < arr.count(); i++) {
                set_runtime_parameter(param[i], arr[i]);
            }
        } break;
        case QJsonValue::Type::Object: {
            const QJsonObject &obj = jval.toObject();
            auto keys = obj.keys();
            for (auto &key : keys) {
                set_runtime_parameter(param[key.toStdString()], obj[key]);
            }
        } break;

        default:
            throw std::runtime_error("Cannot pass an object of unknown type " + std::to_string(static_cast<int>(jval.type())) + " to RPC");
    }
}

static QString rpc_error_to_string(RPCError err) {
    switch (err) {
        case RPCError::success:
            return "Success";
            break;
        case RPCError::timeout_happened:
            return "Timeout Error";
            break;
    }
    return "Error";
}

static QJsonValue create_json_object_from_RPC_answer(const RPCRuntimeDecodedParam &param) {
    switch (param.get_desciption()->get_type()) {
        case RPCRuntimeParameterDescription::Type::array: {
            auto array = param.as_array();
            if (array.front().get_desciption()->get_type() == RPCRuntimeParameterDescription::Type::character) {
                std::string result_string = param.as_string();
                return QJsonValue(QString::fromStdString(result_string));
            } else {
                if (array.size() == 1) {
                    return create_json_object_from_RPC_answer(array.front());
                }
                QJsonArray table;
                for (auto &element : array) {
                    table.append(create_json_object_from_RPC_answer(element));
                }
                return table;
            }
#if 0
            auto array = param.as_array();
            if (array.size() == 1) {
                return create_json_object_from_RPC_answer(array.front());
            }
            QJsonArray table;
            for (auto &element : array) {
                table.append(create_json_object_from_RPC_answer(element));
            }
            return table;
#endif
        }
        case RPCRuntimeParameterDescription::Type::character:
            throw std::runtime_error("TODO: Parse return value of type character");
        case RPCRuntimeParameterDescription::Type::enumeration:
            return QJsonValue(param.as_enum().value);
        case RPCRuntimeParameterDescription::Type::structure: {
            QJsonObject jobj;
            for (Decoded_struct &element : param.as_struct()) {
                jobj[QString::fromStdString(element.name)] = create_json_object_from_RPC_answer(element.type);
            }
            return jobj;
        }
        case RPCRuntimeParameterDescription::Type::integer:
            return QJsonValue(static_cast<qint64>(param.as_integer()));
    }
    assert(!"Invalid type of RPCRuntimeParameterDescription");
    return QJsonValue();
}

static QJsonObject call_rpc_function(RPCProtocol *rpc_protocol, const QString &name, const QJsonObject &object, std::chrono::steady_clock::duration timeout) {
#if 0
    if (QThread::currentThread()->isInterruptionRequested()) {
        throw sol::error("Abort Requested");
    }
#endif

    QJsonObject result;
    QJsonObject rpc;
    QJsonObject reply;

    if (rpc_protocol->is_xml_loaded() == false) {
        rpc_protocol->is_correct_protocol();
    }

    if (!rpc_protocol->function_exists_for_encoding(name.toStdString())) {
        reply["result"] = "ERR_FUNCTION_NOT_FOUND";

        reply["function"] = name;
        rpc["reply"] = reply;
        result["rpc"] = rpc;
        return result;
    }

    RPCRuntimeEncodedFunctionCall function = rpc_protocol->encode_function(name.toStdString());

    int param_count = function.get_parameter_count();
    for (int param_index = 0; param_index < param_count; param_index++) {
        RPCRuntimeEncodedParam &param = function.get_parameter(param_index);
        QString param_name = QString::fromStdString(param.get_description()->get_parameter_name());
        if (object.contains(param_name)) {
            QJsonValue value = object[param_name];
            set_runtime_parameter(param, value);
        } else {
            throw std::runtime_error("key '" + param_name.toStdString() + "' doesnt exist in JSon request.");
        }
    }
    if (function.are_all_values_set()) {
#if 0
        "rpc":{

            "reply":{
                "function": "test_function",
                "result": "SUCCESS",
                "result": "Timeout Error",
                "result": "ERR_FUNCTION_NOT_FOUND",
                "result": "ERR_MISSING_PARAMETERS",
                "duration_ms": 200,
                "trials": 1,
                "arguments": {
                    "param_int1": 1,
                    "param_enum1": "enum_val_test",
                    "param_struct1": {
                        "field_int1":1,
                        "field_int2":2
                    }
                }
            }
        }
#endif
        auto request_start_ms = QDateTime::currentMSecsSinceEpoch();
        auto rpc_result = rpc_protocol->call_and_wait(function, timeout);
        auto request_end_ms = QDateTime::currentMSecsSinceEpoch();
        auto duration_ms = request_end_ms - request_start_ms;
        reply["duration_ms"] = duration_ms;
        reply["result"] = rpc_error_to_string(rpc_result.error);
        reply["trials"] = static_cast<qint64>(rpc_result.trials_needed);
        if (rpc_result.error == RPCError::success) {
            try {
                auto output_params = rpc_result.decoded_function_call_reply->get_decoded_parameters();
                if (output_params.empty()) {
                } else if (output_params.size() == 1) {
                    reply["arguments"] = create_json_object_from_RPC_answer(output_params.front());

                } else {
                    //else: multiple variables, need to make a table
                    //return sol::make_object(lua->lua_state(), "TODO: Not Implemented: Parse multiple return values");
                }
                reply["function"] = name;
                rpc["reply"] = reply;
                result["rpc"] = rpc;
                return result;
            } catch (const std::runtime_error &e) {
                qDebug() << e.what();
                throw;
            }
        }
        //throw std::runtime_error("Call to \"" + name.toStdString() + "\" failed due to timeout");
        return result;
    }

    //qDebug() << object;
    reply["result"] = "ERR_MISSING_PARAMETERS";
    reply["function"] = name;
    rpc["reply"] = reply;
    result["rpc"] = rpc;

    //not all values set, error
    // throw std::runtime_error("Failed calling function, missing parameters");
    return result;
}

void JsonInput::Json_to_cout(const QJsonObject &obj) {
    QJsonDocument doc(obj);

    std::cout << "/{" << std::endl;
    std::cout << QString(doc.toJson()).toStdString() << std::endl;
    std::cout << "}/" << std::endl << std::flush;

#if 0
    qDebug() << "Json_to_cout: /{" << QString(doc.toJson()) << "}/";
#endif
}

void JsonInput::json_to_rpc_call(const QJsonObject &obj) {
    if (rpc_protocol) {
        const QJsonObject &rpc = obj["rpc"].toObject();
        const uint timeout_ms = rpc["timeout_ms"].toInt();
        std::chrono::steady_clock::duration timeout = std::chrono::milliseconds(timeout_ms);
        const QJsonObject &request = rpc["request"].toObject();
        const QString function_name = request["function"].toString();

        const auto &obj = call_rpc_function(rpc_protocol, function_name, request["arguments"].toObject(), timeout);
        Json_to_cout(obj);
    }
}

void JsonInput::json_to_controll_command_execution(const QJsonObject &obj) {
    const QJsonObject &ctrl = obj["controll"].toObject();
    QString command = ctrl["command"].toString().toLower().trimmed();
    if (command == "quit") {
        QCoreApplication::quit();
    } else if (command == "get_hash") {
        QJsonObject controll;
#if 0
        /{
        "controll":{
            "command":"get_hash",
            "result": {
                "hash":"sdfsdfsdfsdfsdf"
            }
        }/
#endif
        QJsonObject result;
        QJsonObject command_result;
        result["hash"] = rpc_protocol->get_client_hash();
        command_result["result"] = result;
        controll["command"] = command;
        controll["result"] = result;
        command_result["controll"] = controll;
        Json_to_cout(command_result);
        //qDebug() << rpc_protocol->get_client_hash();
    } else if (command == "get_version") {
        QJsonObject controll;
#if 0
        /{
        "controll":{
            "command":"get_version",
            "result": {
                "git_hash":0x495ac3b,
                "git_unix_date":213123123,
                "git_string_date":"2017-06-16"
            }
        }/
#endif
        QJsonObject result;
        QJsonObject command_result;
        result["git_hash"] = GITHASH;
        result["git_unix_date"] = GITUNIX;
        result["git_string_date"] = GITDATE;

        command_result["result"] = result;
        controll["command"] = command;
        controll["result"] = result;
        command_result["controll"] = controll;
        Json_to_cout(command_result);
        //qDebug() << rpc_protocol->get_client_hash();
    }
}
