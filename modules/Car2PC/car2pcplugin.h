#ifndef CAR2PCPLUGIN_H
#define CAR2PCPLUGIN_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QStringList>
#include <QDebug>
#include <QByteArray>
#include <QTimer>
#include <QCoreApplication>
#include <plugininterface.h>
#include "CRL/Car2PCSerial/Car2PCSerial.h"
#include "CRL/common.h"

class Car2PCPlugin : public QObject, PluginInterface, PlatformCallbacks
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.aselafernando.headunit.car2pc" FILE "config.json")
    Q_INTERFACES(PluginInterface)

    Q_PROPERTY(QVariantMap ports READ getPorts NOTIFY portsUpdated)
    Q_PROPERTY(bool connected MEMBER m_connected NOTIFY connectedUpdated)

public:
    explicit Car2PCPlugin(QObject *parent = nullptr);

    void init() override;
    QObject *getContextProperty() override;

    void ButtonInputCommandCallback(Button btn) override;
    void PrintString(char * message, int length) override;
    void SendPacketCallback(size_t s, Packet* p);

public slots:
    void eventMessage(QString id, QVariant message) override;
    void serialRestart();

signals:
    void portsUpdated();
    void message(QString id, QVariant message);
    void connectedUpdated();
    void action(QString id, QVariant message);

private slots:
    void handleSerialReadyRead();
    void handleSerialError(QSerialPort::SerialPortError error);
    void settingsChanged(const QString &key, const QVariant &value);

private:
    QSerialPort m_serial;
    QVariantMap m_ports;
    Car2PCSerial::Car2PCSerial m_serialProtocol;
    bool m_connected;
    bool m_text;
    QTimer m_serialRetryTimer;

    void updatePorts();
    void serialConnect();
    void serialDisconnect();

    QVariantMap getPorts() {
        updatePorts();
        return m_ports;
    }
};

#endif // CAR2PCPLUGIN_H
