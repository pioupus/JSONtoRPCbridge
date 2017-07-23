#include "jsoninputtest.h"

#include "../src/jsoninput.h"

#include <iostream>
#include <QString>
#include <QDebug>

void JsonInputTest::initTestCase(){

}

void JsonInputTest::cleanupTestCase(){

}

void JsonInputTest::JSonEscapingTest()
{
    JsonInput ji;

    QString garbage1 = R"(
            /{
            "rpc":{
                "timeout_ms":100,
                "request":{
                    "function": "test_function",
                    "arguments":
            )";


    QString garbage2 = R"(
                 "param_int1": 1,
                 "param_enum1": "enum_val_test",
                 "param_struct1": {

        }/
    )";

    QString garbage3 = R"(
                 "param_int1": 1,
                 "param_enum1": "enum_val}/_test",
                 "param_struct1": {

        }/
    )";

    QString garbage4 = R"(
                 "param_int1": 1,
                 "param_enum1": "enum_val/{_test",
                 "param_struct1": {

        }/
    )";

    QString request_a = R"(
    /{
        "rpc":{
            "timeout_ms":100,
            "request":{
                "function": "test_function",
                "arguments": {



            )";

    QString request_b = R"(


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

            )";


    QVERIFY(!ji.append_to_input_buffer(request_a));
    QVERIFY(ji.append_to_input_buffer(request_b));

    ji.clear_input_buffer();
    QVERIFY(!ji.append_to_input_buffer(garbage1));
    QVERIFY(!ji.append_to_input_buffer(request_a));
    QVERIFY(ji.append_to_input_buffer(request_b));

    ji.clear_input_buffer();
    QVERIFY(!ji.append_to_input_buffer(garbage1));
    QVERIFY(!ji.append_to_input_buffer(request_a));
    QVERIFY(!ji.append_to_input_buffer(garbage1));
    QVERIFY(!ji.append_to_input_buffer(request_b));
    QVERIFY(!ji.append_to_input_buffer(request_a));
    QVERIFY(ji.append_to_input_buffer(request_b));

    QVERIFY(!ji.append_to_input_buffer(request_a));
    QVERIFY(ji.append_to_input_buffer(request_b));

    QVERIFY(!ji.append_to_input_buffer(request_a));
    QVERIFY(ji.append_to_input_buffer(request_b));

    QVERIFY(!ji.append_to_input_buffer(garbage1));
    QVERIFY(!ji.append_to_input_buffer(garbage2));
    QVERIFY(!ji.append_to_input_buffer(garbage3));
    QVERIFY(!ji.append_to_input_buffer(garbage4));

    QVERIFY(!ji.append_to_input_buffer(request_a));
    QVERIFY(ji.append_to_input_buffer(request_b));
}
