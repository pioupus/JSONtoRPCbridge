#include "rpcprotocol.h"

#include "channel_codec_wrapper.h"

#include "rpc_ui.h"
#include "rpcruntime_decoded_function_call.h"
#include "rpcruntime_decoder.h"
#include "rpcruntime_encoded_function_call.h"
#include "rpcruntime_encoder.h"
#include "rpcruntime_protocol_description.h"
#include <QByteArray>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QObject>
#include <QSettings>
#include <QString>

#include <cassert>
#include <fstream>

using namespace std::chrono_literals;

static void set_description_data(Device_data &dd, const RPCRuntimeDecodedParam &param, const std::string parent_field_name) {
    switch (param.get_desciption()->get_type()) {
        case RPCRuntimeParameterDescription::Type::array: {
            std::string parameter_name = param.get_desciption()->get_parameter_name();
            const auto &param_value = param.as_string();

            if (parameter_name == "name") {
                dd.name = dd.name + QString().fromStdString(param_value);
            } else if (parameter_name == "version") {
                dd.version = dd.version + QString::fromStdString(param_value);
            } else if (parameter_name == "guid") {
                const auto &param_guid = param.as_full_string();
                dd.guid_bin = QByteArray::fromStdString(param_guid);
            } else {
                std::string field_name = param.get_desciption()->get_parameter_name();
                for (int i = 0; i < param.get_desciption()->as_array().number_of_elements; i++) {
                    set_description_data(dd, param.as_array()[i], field_name);
                }
            }
        }

        break;
        case RPCRuntimeParameterDescription::Type::character: {
        } break;
        case RPCRuntimeParameterDescription::Type::enumeration:
            break;
        case RPCRuntimeParameterDescription::Type::integer: {
            std::string parameter_name = param.get_desciption()->get_parameter_name();
            if (parameter_name == "") {
                parameter_name = parent_field_name;
            }
            if (parameter_name == "githash") {
                const auto &param_value = param.as_integer();
                dd.githash = QString::number(param_value, 16);
            } else if (parameter_name == "gitDate_unix") {
                const auto &param_value = param.as_integer();
                dd.gitDate_unix = QDateTime::fromTime_t(param_value).toString();
            } else if (parameter_name == "serialnumber") {
                const auto &param_value = param.as_integer();
                dd.serialnumber = QString::number(param_value);
            } else if (parameter_name == "deviceID") {
                const auto &param_value = param.as_integer();
                dd.deviceID = QString::number(param_value);
            } else if (parameter_name == "boardRevision") {
                const auto &param_value = param.as_integer();
                dd.boardRevision = QString::number(param_value);
            }
        } break;
        case RPCRuntimeParameterDescription::Type::structure: {
            std::string field_name = param.get_desciption()->get_parameter_name();
            for (auto &member : param.as_struct()) {
                set_description_data(dd, member.type, field_name);
            }
        } break;
    }
}

static Device_data get_description_data(const RPCRuntimeDecodedFunctionCall &call) {
    Device_data dd;
    for (auto &param : call.get_decoded_parameters()) {
        set_description_data(dd, param, "");
    }
    return dd;
}

QString Device_data::get_summary() const {
    QStringList statustip;
    for (auto &d : get_description_source()) {
        if (d.source.isEmpty() == false) {
            statustip << d.description + ": " + d.source;
        }
    }
    return statustip.join("\n");
}

std::vector<Device_data::Description_source> Device_data::get_description_source() const {
    return {{"GitHash", githash},   {"GitDate", gitDate_unix},     {"Serialnumber", serialnumber},
            {"DeviceID", deviceID}, {"GUID", guid_bin.toHex()},    {"BoardRevision", boardRevision},
            {"Name", name},         {"Version", version}};
}

RPCProtocol::RPCProtocol(RPCSerialPort &device, QString xml_path, std::chrono::steady_clock::duration timeout_)
    : communication_wrapper(device)
    , xml_path(xml_path)
    , timeout{timeout_} {
    rpc_runtime_protocol = std::make_unique<RPCRuntimeProtocol>(communication_wrapper, timeout);
#if 1
    console_message_connection =
        QObject::connect(rpc_runtime_protocol.get(), &RPCRuntimeProtocol::console_message, [this](RPCConsoleLevel level, const QString &data) { //
            this->console_message(level, data);
        });
    assert(console_message_connection);
#endif

#if 0
    connection = QObject::connect(&device, &CommunicationDevice::received, [&cc = channel_codec](const QByteArray &data) {
        //qDebug() << "RPC-Protocol received" << data.size() << "bytes from device";
        cc.add_data(reinterpret_cast<const unsigned char *>(data.data()), data.size());
    });
#endif
    // QObject::connect(rpc_runtime_protocol.get(), SIGNAL(console_message(RPCConsoleLevel,QString)), this, SLOT());
}

RPCProtocol::~RPCProtocol() {
    assert(console_message_connection);
    auto result = QObject::disconnect(console_message_connection);
    assert(result);
}

QString RPCProtocol::get_client_hash() {
    return rpc_runtime_protocol.get()->get_client_hash();
}

bool RPCProtocol::is_correct_protocol() {
    // const CommunicationDevice::Duration TIMEOUT = std::chrono::milliseconds{100};
    _is_xml_loaded = false;
    if (rpc_runtime_protocol.get()->load_xml_file(xml_path)) {
        if (rpc_runtime_protocol.get()->description.has_function("get_device_descriptor")) {
            auto get_device_descriptor_function = RPCRuntimeEncodedFunctionCall{rpc_runtime_protocol.get()->description.get_function("get_device_descriptor")};
            if (get_device_descriptor_function.are_all_values_set()) {
                descriptor_answer = call_and_wait(get_device_descriptor_function, timeout).decoded_function_call_reply;
                if (descriptor_answer) {
                    device_data = get_description_data(*descriptor_answer);
                }
            } else {
                qDebug() << "RPC-function \"get_device_descriptor\" requires unknown parameters";
            }
        } else {
            qDebug() << "No RPC-function \"get_device_descriptor\" available";
        }
        _is_xml_loaded = true;
        return true;
    }

    return false;
}

RPCFunctionCallResult RPCProtocol::call_and_wait(const RPCRuntimeEncodedFunctionCall &call) {
    return call_and_wait(call, timeout);
}

RPCFunctionCallResult RPCProtocol::call_and_wait(const RPCRuntimeEncodedFunctionCall &call, std::chrono::_V2::steady_clock::duration timeout) {
    return rpc_runtime_protocol.get()->call_and_wait(call, timeout);
}

const RPCRunTimeProtocolDescription &RPCProtocol::get_description() {
    return rpc_runtime_protocol.get()->description;
}

QString RPCProtocol::get_device_summary() {
    return device_data.get_summary();
}

void RPCProtocol::console_message(RPCConsoleLevel level, QString message) {
    switch (level) {
        case RPCConsoleLevel::note:
            //  Console::note() << message;
            break;

        case RPCConsoleLevel::error:
            //Console::error() << message;
            break;

        case RPCConsoleLevel::debug:
            // Console::debug() << message;
            break;

        case RPCConsoleLevel::warning:
            // Console::warning() << message;
            break;
    }
}

RPCRuntimeEncodedFunctionCall RPCProtocol::encode_function(const std::string &name) const {
    return rpc_runtime_protocol.get()->encode_function(name);
}

bool RPCProtocol::function_exists_for_encoding(const std::string &name) const {
    return rpc_runtime_protocol.get()->function_exists_for_encoding(name);
}

bool RPCProtocol::is_xml_loaded()
{
    return _is_xml_loaded;
}

void RPCProtocol::clear() {
    rpc_runtime_protocol.get()->clear();
}

std::string RPCProtocol::get_name() {
    return device_data.name.toStdString();
}

#if 0
CommunicationDeviceWrapper::CommunicationDeviceWrapper(CommunicationDevice &device)
    : com_device(device) {
    connect(this, SIGNAL(decoded_received(const QByteArray &)), &com_device, SIGNAL(decoded_received(const QByteArray &)));

    connect(this, SIGNAL(message(const QByteArray &)), &com_device, SIGNAL(message(const QByteArray &)));

    connect(&com_device, SIGNAL(received(const QByteArray &)), this, SIGNAL(received(const QByteArray &)));

}

void CommunicationDeviceWrapper::send(std::vector<unsigned char> data, std::vector<unsigned char> pre_encodec_data) {
    com_device.send(data, pre_encodec_data);
}

bool CommunicationDeviceWrapper::waitReceived(std::chrono::_V2::steady_clock::duration timeout, int bytes, bool isPolling) {
    return com_device.waitReceived(timeout, bytes, isPolling);
}
#endif
