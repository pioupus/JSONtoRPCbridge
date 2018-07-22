
#include "mainclass.h"

#include <QByteArray>
#include <QCommandLineParser>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QTextStream>
#include <QtCore/QCoreApplication>
#include <cassert>

enum CommandLineParseResult { CommandLineOk, CommandLineError, CommandLineVersionRequested, CommandLineHelpRequested };

static QtMessageHandler old_handler;

void message_handler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    (void)context;
    switch (type) {
        case QtCriticalMsg:
        case QtFatalMsg:
            qDebug() << msg;
            //QMessageBox::critical(MainWindow::mw, "Qt Error", '\"' + msg + '\"' + "\nwas caused by" + context.function + " in " + context.file + ":" +
            //                                                      QString::number(context.line) +
            //                                                      ".\nAdd a breakpoint in main.cpp:28 to inspect the stack.\n"
            //                                                      "Press CTRL+C to copy the content of this message box to your clipboard.");
            break;
        case QtWarningMsg:
            qDebug() << msg;
            //QMessageBox::warning(MainWindow::mw, "Qt Warning", '\"' + msg + '\"' + "\nwas caused by" + context.function + " in " + context.file + ":" +
            //                                                       QString::number(context.line) +
            //                                                       ".\nAdd a breakpoint in main.cpp:28 to inspect the stack.\n"
            //                                                       "Press CTRL+C to copy the content of this message box to your clipboard.");
            break;
        case QtDebugMsg: {
            QTextStream cout(stdout, QIODevice::WriteOnly);
            cout << msg << endl;

        } // qDebug() << "debug" <<msg;
        break;
        case QtInfoMsg: {
            QTextStream cout(stdout, QIODevice::WriteOnly);
            cout << msg << endl;
        } break;
    }
   // old_handler(type, context, msg);
}

int main(int argc, char *argv[]) {
    old_handler = qInstallMessageHandler(message_handler);
    QCoreApplication a(argc, argv);

#if 1
    //https://bugreports.qt.io/browse/QTBUG-28467
    // Trying to use as much digits as possible, the stored value is in fact 0.12340123401234013
    uint32_t d1 = 1532267105;
    // Store the value in a QJsonObject and use toJson() method to obtain a JSON parseable object
    QJsonObject jObject;
    jObject.insert("unix_time_in_json", QJsonValue(double(d1)));
    QJsonDocument jDocument1(jObject);
    QByteArray json_dump(jDocument1.toJson());
    // Parse back the JSON object into a new QJsonDocument
    QJsonDocument jDocument2(QJsonDocument::fromJson(json_dump));
    // Obtain the stored value
    uint32_t d2(jDocument2.object().value("unix_time_in_json").toDouble());
    if (d1 == d2) {
        //  qDebug() << "value " + QString(d1) + " correctly parsed in json";
    } else {
        qDebug() << "value " + QString(d1) + " inccorrectly parsed in json. resulted in: " + QString(d2);
    }
#endif

    QCommandLineParser parser;
    parser.setApplicationDescription("A bridge to RPC commands controlled by JSON-structures.");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("comport", "the comport where the RPC-devices is attached to.");
    parser.addPositionalArgument("baud", "Baudrate which is used by the RPC device.");
    parser.addPositionalArgument("xml", "Directory where RPC-protocoll description can be found.");

    QCommandLineOption timeout_ms_Option("t", "Specifies timeout in ms. Default is 200ms");
    timeout_ms_Option.setDefaultValue("500");
    parser.addOption(timeout_ms_Option);

    // Process the actual command line arguments given by the user
    parser.process(a);

    const QStringList args = parser.positionalArguments();

    if (args.size() != 3) {
        qDebug() << "Wrong arguments specified.";
        return CommandLineError;
    }

    QString comport = args.at(0);
    uint baud = args.at(1).toInt();
    QString xmlpath = args.at(2);
    uint timeout_ms = parser.value(timeout_ms_Option).toInt();

    qDebug() << "RPC JSON Bridge";

    std::chrono::_V2::steady_clock::duration timeout;

    timeout = std::chrono::milliseconds(timeout_ms);
    ApplicationClass mainclass{comport, baud, xmlpath, timeout, nullptr};
    (void)mainclass;

    return a.exec();
}
