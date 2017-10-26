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
#include <assert.h>

JsonInput::JsonInput(RPCProtocol *rpc_protocol_)
    : rpc_protocol{rpc_protocol_} {}

void JsonInput::clear_input_buffer() {
    input_buffer = "";
}

bool JsonInput::append_to_input_buffer(QString data) {
    //qDebug() << data;
    bool result = false;

    input_buffer += data;
    bool input_not_empty = true;
    while (input_not_empty) {
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

        for (auto start : start_tag_positions) {
            for (auto stop : stop_tag_positions) {
                if (stop < start) {
                    continue;
                }
                auto str = input_buffer.mid(start, stop - start + QString("}/").length());
                if (test_json_input(str)) {
                    input_buffer.remove(0, stop + QString("}/").length());

                    result = true;
                    found = true;
                    break;
                }
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

bool JsonInput::test_json_input(QString str) {
    QJsonParseError p_err;
    str = str.remove(0, 1);                // remove the / of the /{
    str = str.remove(str.length() - 1, 1); // remove the / of the }/
    //qDebug() << str;

    auto doc = QJsonDocument::fromJson(str.toUtf8(), &p_err);

    if (doc.isNull()) {
        qDebug() << "Json parser error. It says:" << p_err.errorString();
        return false;
    }
    QJsonObject obj = doc.object();
    return test_json_object(obj);
}

bool JsonInput::test_json_object(const QJsonObject &obj) {
    if (obj.isEmpty()) {
        return false;
    }

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
    }

    {
        /{
        "controll":{
            "command":"quit",
            "arguments": {
            }
        }/
    }
#endif
    if (!rpc.isEmpty()) {
        if (!rpc.contains("timeout_ms")) {
            return false;
        }

        const QJsonObject &request = rpc["request"].toObject();
        if (request.isEmpty()) {
            return false;
        }

        if (!request.contains("function")) {
            return false;
        }

        if (!request.contains("arguments")) {
            return false;
        }
        qDebug() << "decoded successfully";

        json_to_rpc_call(obj);
        return true;
    } else if (!ctrl.isEmpty()) {
        if (!ctrl.contains("command")) {
            return false;
        }
        json_to_controll_command_execution(obj);
    }
    return false;
}

static void set_runtime_parameter(RPCRuntimeEncodedParam &param, QJsonValue jval) {
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

            for (std::size_t i = 0; i < arr.count(); i++) {
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

    rpc_protocol->is_correct_protocol();

    // Console::note() << QString("\"%1\" called").arg(name.c_str());

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
        RPCRuntimeEncodedParam &param = function.get_parameter(param_index++);
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
    std::cout << "}/" << std::endl;
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
    }
}