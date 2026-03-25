#include <QQuickWindow>

#include "log_androidauto.h"
#include "androidauto.h"

AndroidAutoPlugin::AndroidAutoPlugin(QObject *parent)
     : QObject (parent)
     , m_headunit(this)
     , m_bluetoothServer(this)
{
    m_pluginSettings.eventListeners = QStringList() << "UsbConnectionListenerPlugin::UsbDeviceAdded" << "SYSTEM::SetNightMode" << "J2534::VSS" << "J2534::Gear" << "GPSD::Location";
    m_pluginSettings.events = QStringList() << "connected";
    //gst_init(NULL, NULL);

    m_interfaceSettings.mediaStream = true;
    m_interfaceSettings.voiceStream = true;

    connect(&m_headunit, &HeadunitVideoSource::playbackStarted, this, &AndroidAutoPlugin::playbackStarted);
    connect(&m_headunit, &HeadunitVideoSource::statusChanged, this, &AndroidAutoPlugin::huStatusChanged);
    connect(&m_bluetoothServer, &BluetoothServer::deviceConnected, this, &AndroidAutoPlugin::btDeviceConnected);
}

AndroidAutoPlugin::~AndroidAutoPlugin()
{
}

QObject *AndroidAutoPlugin::getContextProperty(){
    return qobject_cast<QObject*>(&m_headunit);
}

void AndroidAutoPlugin::eventMessage(QString id, QVariant message){
    if(id == "UsbConnectionListenerPlugin::UsbDeviceAdded"){
        if (m_headunit.status() != HeadunitVideoSource::RUNNING) {
            m_headunit.startHU();
        }
    } else if(id == "UsbConnectionListenerPlugin::UsbDeviceRemoved"){
    } else if(id == "SYSTEM::SetNightMode"){
        m_headunit.setNightMode(message.toBool());
    } else if(id == "J2534::VSS") {
        double speedms = message.toInt() / 3.6;
        m_headunit.setVSS(speedms);
    } else if(id == "J2534::Gear") {
        m_headunit.setGear(message.toInt());
    } else if(id == "GPSD::Location") {
        QVariantMap map = message.toMap();
        m_headunit.setLocation(map["latitude"].toDouble(),map["longitude"].toDouble(),
            map["track"].toDouble(),map["speed"].toDouble(),map["altitude"].toDouble(),map["eph"].toDouble());
    }
}
void AndroidAutoPlugin::init(){
    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);
    m_headunit.init();
    m_headunit.startHU();

    QBluetoothAddress address;
    auto adapters = QBluetoothLocalDevice::allDevices();
    if (adapters.size() > 0) {
        address = adapters.at(0).address();
    } else {
        return;
    }

    QString ipAddr;
    foreach (QHostAddress addr, QNetworkInterface::allAddresses()) {
        if (!addr.isLoopback() && (addr.protocol() == QAbstractSocket::IPv4Protocol)) {
            ipAddr = addr.toString();
        }
    }

    BluetoothServer::Config serverConfig {
        address,
        this->m_settings["wlan_bssid"].toString(),
        this->m_settings["wlan_name"].toString(),
        this->m_settings["wlan_password"].toString(),
        this->m_settings["network_address"].toString()
    };

    qCDebug(LOG_PLUGIN_ANDROIDAUTO) << "AA Transport Type " << this->m_settings["transport_type"].toString();

    if (this->m_settings["transport_type"].toString().compare("network") == 0) {
        qCDebug(LOG_PLUGIN_ANDROIDAUTO) << "Startng BT RFCOMM server";
        m_bluetoothServer.start(serverConfig);
    }
}

void AndroidAutoPlugin::btDeviceConnected() {
    //m_headunit.startHU();
}

void AndroidAutoPlugin::start() {
    m_headunit.startMedia();
}
void AndroidAutoPlugin::stop() {
    m_headunit.stopMedia();
}
void AndroidAutoPlugin::prevTrack() {
    m_headunit.prevTrack();
}
void AndroidAutoPlugin::nextTrack() {
    m_headunit.nextTrack();
}
void AndroidAutoPlugin::setMediaVolume(uint8_t volume) {
    m_headunit.setMediaVolume(volume);
}
void AndroidAutoPlugin::setVoiceVolume(uint8_t volume) {
    m_headunit.setVoiceVolume(volume);
}

void AndroidAutoPlugin::huStatusChanged(){
    emit message("connected", (m_headunit.status() > HeadunitVideoSource::NO_CONNECTION));
}
