#include "car2pcplugin.h"
#include <math.h>
//Car2PC device only supports 9600 baud 8N1
#define CAR2PC_BAUD 9600

Car2PCPlugin::Car2PCPlugin(QObject *parent) : QObject (parent), m_serial(this), m_serialProtocol(), m_serialRetryTimer(this)
{
    m_serialProtocol.setCallbacks(this);
    m_pluginSettings.eventListeners = QStringList() << "MediaInput::position" << "PhoneBluetooth::position" << "PhoneBluetooth::trackNumber";
    m_text = m_settings.value("text").toString() == "true";
}

void Car2PCPlugin::init() {
    updatePorts();

    connect(&m_serial, &QSerialPort::readyRead, this, &Car2PCPlugin::handleSerialReadyRead);
    connect(&m_serial, &QSerialPort::errorOccurred, this, &Car2PCPlugin::handleSerialError);
    connect(&m_settings, &QQmlPropertyMap::valueChanged, this, &Car2PCPlugin::settingsChanged);
    connect(&m_serialRetryTimer, &QTimer::timeout, this, &Car2PCPlugin::serialConnect);
    serialConnect();
}

QObject *Car2PCPlugin::getContextProperty(){
    return this;
}

void Car2PCPlugin::eventMessage(QString id, QVariant message) {
    //Track Name: NMTest Name
    //Track Time: TMHHMMSS
    //Track Num : TR000
    if (id == "PhoneBluetooth::position" || id == "MediaInput::position") {
        uint32_t seconds = (uint32_t)floor(message.toUInt() / 1000.0);
        uint32_t minutes = seconds / 60;
        uint32_t hours = minutes / 60;
        char timestamp[14]; //even though this only needs to be 9 g++ complains if less than 14
        snprintf(timestamp, 14, "TM%02d%02d%02d", hours, minutes % 60, seconds % 60);
        m_serialProtocol.sendMessage(8, timestamp);
    } else if (id == "PhoneBluetooth::trackNumber") {
        char track[6];
        snprintf(track, 6, "TR%03d", message.toUInt());
        m_serialProtocol.sendMessage(5, track);
    }
}

void Car2PCPlugin::SendPacketCallback(size_t s, Packet* p) {
    if (m_serial.isOpen()) {
        m_serial.write((char*)p, s);
        m_serial.flush();
    }
}

void Car2PCPlugin::settingsChanged(const QString &key, const QVariant &){
    if(key == "serial_port"){
        serialDisconnect();
        serialConnect();
    }
    else if (key == "text") {

    }
}

void Car2PCPlugin::updatePorts() {
    m_ports.clear();
    for(const QSerialPortInfo &port : QSerialPortInfo::availablePorts()){
        QString displayName = QString("%1 (%2 - %3)").arg(port.portName()).arg(port.manufacturer()).arg(port.productIdentifier());
        m_ports.insert(port.portName(),displayName);
    }
    emit portsUpdated();
}

void Car2PCPlugin::serialConnect(){
    m_serialRetryTimer.stop();
    m_serial.setPortName(m_settings.value("serial_port").toString());
    m_serial.setBaudRate(CAR2PC_BAUD);

    if (!m_serial.open(QIODevice::ReadWrite)) {
        qDebug() << QObject::tr("Car2PC: Failed to open port %1, error: %2")
                        .arg(m_settings.value("port").toString(), m_serial.errorString());
    } else {
        qDebug() << "Car2PC: Connected to Serial : " << m_serial.portName() << m_serial.baudRate();
        m_connected = true;
        emit connectedUpdated();
    }
}

void Car2PCPlugin::serialDisconnect(){
    if(m_serial.isOpen()){
        m_serial.close();
        qDebug() << "Car2PC: Disconnected from serial : " << m_serial.portName();
    }
    m_connected = false;
    emit connectedUpdated();
}

void Car2PCPlugin::serialRestart(){
    emit message("GUI::Notification", "{\"image\":\"qrc:/qml/icons/alert.png\", \"title\":\"Car2PC Serial\", \"description\":\"Serial Port restarted\"}");
    serialDisconnect();
    serialConnect();
}

void Car2PCPlugin::handleSerialError(QSerialPort::SerialPortError error){
    switch (error) {
        case QSerialPort::WriteError:
        case QSerialPort::ReadError:
        case QSerialPort::NotOpenError:
        case QSerialPort::DeviceNotFoundError:
        case QSerialPort::PermissionError:
        case QSerialPort::TimeoutError:
            if(m_serial.isOpen()){
                m_serial.close();
            }
            m_serial.clearError();
            m_connected = false;
            emit connectedUpdated();
            qDebug() << "Car2PC: Error : " << error;
            m_serialRetryTimer.start(2000);
            break;
        default:
            break;
    }
}

void Car2PCPlugin::handleSerialReadyRead(){
    if(m_serial.isOpen()){
        uint8_t buffer;
        while(m_serial.read((char*)&buffer, 1)){
            uint8_t rec = static_cast<uint8_t>(buffer);
            m_serialProtocol.receiveByte(rec);
            if(!m_serial.isOpen()){
                break;
            }
        }
    }
}

void Car2PCPlugin::ButtonInputCommandCallback(Button btn) {
    QString cmd = "";

    switch(btn) {
        case Button::PLAY:
            cmd = m_settings.value("PLAY").toString();
            if(cmd == "")
                emit message("MediaInput", "Play");
            PrintString("PLAY", 4);
            return;
        case Button::STOP:
            cmd = m_settings.value("STOP").toString();
            if(cmd == "")
                emit message("MediaInput", "Stop");
            PrintString("STOP", 4);
            return;
        case Button::NEXT_TRACK:
            cmd = m_settings.value("NEXT_TRACK").toString();
            if(cmd == "")
                emit message("MediaInput", "Next");
            PrintString("NEXT_TRACK", 10);
            return;
        case Button::PREV_TRACK:
            cmd = m_settings.value("PREV_TRACK").toString();
            if(cmd == "")
                emit message("MediaInput", "Previous");
            PrintString("PREV_TRACK", 10);
            return;
        case Button::NEXT_DISC:
            cmd = m_settings.value("NEXT_DISC").toString();
            PrintString("NEXT_DISC", 9);
            break;
        case Button::PREV_DISC:
            cmd = m_settings.value("PREV_DISC").toString();
            PrintString("PREV_DISC", 4);
            break;
        case Button::SCAN_ON:
            cmd = m_settings.value("SCAN_ON").toString();
            PrintString("SCAN_ON", 7);
            break;
        case Button::SCAN_OFF:
            cmd = m_settings.value("SCAN_OFF").toString();
            PrintString("SCAN_OFF", 8);
            break;
        case Button::REPEAT_ON:
            PrintString("REPEAT_ON", 9);
            cmd = m_settings.value("REPEAT_ON").toString();
            if(cmd == "")
                emit message("MediaInput", "RepeatOn");
            break;
        case Button::REPEAT_OFF:
            PrintString("REPEAT_OFF", 10);
            cmd = m_settings.value("REPEAT_OFF").toString();
            if(cmd == "")
                emit message("MediaInput", "RepeatOff");
            break;
        case Button::SHUFFLE_ON:
            PrintString("SHUFFLE_ON", 10);
            cmd = m_settings.value("SHUFFLE_ON").toString();
            if(cmd == "")
                emit message("MediaInput", "ShuffleOn");
            break;
        case Button::SHUFFLE_OFF:
            PrintString("SHUFFLE_OFF", 11);
            cmd = m_settings.value("SHUFFLE_OFF").toString();
            if(cmd == "")
                emit message("MediaInput", "ShuffleOff");
            break;
        case Button::FF_ON:
            cmd = m_settings.value("FF_ON").toString();
            PrintString("FF_ON", 5);
            break;
        case Button::FF_OFF:
            cmd = m_settings.value("FF_OFF").toString();
            PrintString("FF_OFF", 6);
            break;
        case Button::RW_ON:
            cmd = m_settings.value("RW_ON").toString();
            PrintString("RW_ON", 5);
            break;
        case Button::RW_OFF:
            cmd = m_settings.value("RW_OFF").toString();
            PrintString("RW_OFF", 6);
            break;
    }

    if (cmd != "") {
        emit action(cmd, 0);
        qDebug() << "Car2PC Calling Action: " << cmd;
    }
}

void Car2PCPlugin::PrintString(const char* message, int length) {
    qDebug() << "Car2PC: " << QString::fromUtf8(message, length);
}
