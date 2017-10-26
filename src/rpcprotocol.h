#ifndef RPCPROTOCOL_H
#define RPCPROTOCOL_H

#include "channel_codec_wrapper.h"
#include "rpcserialport.h"
//#include "device_protocols_settings.h"
//#include "protocol.h"
#include "rpcruntime_decoder.h"
#include "rpcruntime_encoder.h"
#include "rpcruntime_protcol.h"
#include "rpcruntime_protocol_description.h"
#include <memory>
//#include <sol.hpp>

class QTreeWidgetItem;
class RPCRuntimeEncodedFunctionCall;

struct Device_data {
    QString githash;
    QString gitDate_unix;

    QString serialnumber;
    QString deviceID;
    QByteArray guid_bin;
    QString boardRevision;

    QString name;
    QString version;

    QString get_summary() const;
    //  void get_lua_data(sol::table &t) const;

    private:
    struct Description_source {
        QString description;
        const QString source;
    };
    std::vector<Description_source> get_description_source() const;
};

#if 0
class CommunicationDeviceWrapper : public RPCIODevice {
    public:

    CommunicationDeviceWrapper(CommunicationDevice &device);

    void send(std::vector<unsigned char> data, std::vector<unsigned char> pre_encodec_data) override;
    bool waitReceived(std::chrono::steady_clock::duration timeout = std::chrono::seconds(1), int bytes = 1, bool isPolling=false) override;

private:
    CommunicationDevice &com_device;

};

#endif
class RPCProtocol {
    public:
    RPCProtocol(RPCSerialPort &device, QString xml_path, std::chrono::_V2::steady_clock::duration timeout_);
    ~RPCProtocol();
    RPCProtocol(const RPCProtocol &) = delete;
    RPCProtocol(RPCProtocol &&other) = delete;

    bool is_correct_protocol();
    RPCFunctionCallResult call_and_wait(const RPCRuntimeEncodedFunctionCall &call);
    RPCFunctionCallResult call_and_wait(const RPCRuntimeEncodedFunctionCall &call, std::chrono::_V2::steady_clock::duration timeout);
    const RPCRunTimeProtocolDescription &get_description();
    void set_ui_description(QTreeWidgetItem *ui_entry);
    RPCProtocol &operator=(const RPCProtocol &&) = delete;
    // void get_lua_device_descriptor(sol::table &t) const;
    RPCRuntimeEncodedFunctionCall encode_function(const std::string &name) const;

    void clear();
    std::string get_name();

    QString get_device_summary();

    bool function_exists_for_encoding(const std::string &name) const;
private:
    std::unique_ptr<RPCRuntimeProtocol> rpc_runtime_protocol;

    void console_message(RPCConsoleLevel level, QString message);

    QMetaObject::Connection console_message_connection;
    std::unique_ptr<RPCRuntimeDecodedFunctionCall> descriptor_answer;
    Device_data device_data;
    RPCSerialPort &communication_wrapper;
    //  DeviceProtocolSetting device_protocol_setting;

    QString xml_path;
    std::chrono::_V2::steady_clock::duration timeout;
};

#endif // RPCPROTOCOL_H
