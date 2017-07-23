#ifndef MAINCLASS_H
#define MAINCLASS_H

#include <QObject>
#include <QThread>
#include <QTextStream>
#include "jsoninput.h"

class ConsoleInputWorker : public QObject {
    Q_OBJECT
public:
    ConsoleInputWorker();
    ~ConsoleInputWorker();
public slots:
    void process();
signals:
    void finished();
    void input_received(QString str);
private:
    QTextStream in_stream;
};


class ApplicationClass : public QObject
{
    Q_OBJECT
public:
    explicit ApplicationClass(QObject *parent = 0);
    void test();

private slots:
    void input_received(QString str);

private:
    QThread* console_input_thread = nullptr;
    JsonInput json_input;
};

#endif // MAINCLASS_H
