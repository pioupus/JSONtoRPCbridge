
#include "mainclass.h"

#include <QCommandLineParser>
#include <QDebug>
#include <QtCore/QCoreApplication>
#include <cassert>

enum CommandLineParseResult { CommandLineOk, CommandLineError, CommandLineVersionRequested, CommandLineHelpRequested };

static QtMessageHandler old_handler;

void message_handler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
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
        case QtDebugMsg:
        case QtInfoMsg:;
    }
    old_handler(type, context, msg);
}

int main(int argc, char *argv[]) {
    old_handler = qInstallMessageHandler(message_handler);
    QCoreApplication a(argc, argv);

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

    qDebug() << "Hello world";

    std::chrono::_V2::steady_clock::duration timeout;

    timeout = std::chrono::milliseconds(timeout_ms);
    ApplicationClass mainclass{comport, baud, xmlpath, timeout, nullptr};
    (void)mainclass;

    return a.exec();
}
