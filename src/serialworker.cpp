#include <qdebug.h>
#include <thread>
#include <chrono>
#include "serialworker.h"




SerialThread::SerialThread(QObject *parent) :
    QObject(parent)
{

    thread = new QThread();

    serialWorker = new SerialWorker(this);
   // serialWorkerForRPCFunc = serialWorker;
    serialWorker->moveToThread(thread);

    connect(thread, SIGNAL(started()), serialWorker, SLOT(process()));
    connect(serialWorker, SIGNAL(finished()), thread, SLOT(quit()));
    connect(serialWorker, SIGNAL(finished()), serialWorker, SLOT(deleteLater()));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

    connect(this, SIGNAL(openPort(QString, int)), serialWorker, SLOT(openPort(QString, int)));
    connect(this, SIGNAL(closePort()), serialWorker, SLOT(closePort()));
    connect(this, SIGNAL(isPortOpened()), serialWorker, SLOT(isPortOpened()));
    connect(this, SIGNAL(sendData(QByteArray)), serialWorker, SLOT(sendData(QByteArray)));

   thread->start();

}

void SerialThread::open(QString name, int baudrate)
{

    emit openPort(name,baudrate);

}

void SerialThread::close()
{

    emit closePort();

}

bool SerialThread::isOpen()
{
    bool result;
    result = emit isPortOpened();
    return result;
}

void SerialThread::sendByteData(QByteArray data)
{
    emit sendData(data);
}



SerialWorker::SerialWorker(SerialThread *serialThread, QObject *parent):
    QObject(parent)
{
    serialport = new QSerialPort(this);
    connect(serialport,SIGNAL(readyRead()),this,SLOT(on_readyRead()));



}



void SerialWorker::openPort(QString name, int baudrate)
{
    qDebug() << "opening "<< name << "with process"<<QThread::currentThreadId();
    serialport->setPortName(name);
    serialport->setBaudRate(baudrate);
    serialport->open(QIODevice::ReadWrite);
}

void SerialWorker::closePort()
{
    serialport->close();
}

bool SerialWorker::isPortOpened()
{
    bool result = serialport->isOpen();
    return result;
}

void SerialWorker::sendData(QByteArray data)
{
    qDebug() << "writing "<< data.toHex()<< "with process"<<QThread::currentThreadId();
    serialport->write(data);
}


void SerialWorker::process()
{
    qDebug()<<"serial process "<<QThread::currentThreadId();
}

void SerialWorker::on_readyRead()
{

    //qDebug() << "on read ID:" << thread()->currentThreadId();
    //qDebug() << "on read:" << QThread::currentThreadId();
    QByteArray inbuffer = serialport->readAll();

    if (inbuffer.count() == 512){
        qDebug() << "Rechner langsam";
    }
  //  for (int i=0;i<inbuffer.count();i++){
 //       channel_push_byte_to_RPC(&channel_codec_instance[channel_codec_comport_transmission], inbuffer[i]);
   //     if (inbuffer[i] == '\n'){
           // qDebug() << linebuffer;

    //        linebuffer = "";
    //    }else{
    //        linebuffer += inbuffer[i];
    //    }
//    }

}
