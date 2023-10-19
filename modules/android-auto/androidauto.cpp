#include "androidauto.h"


AndroidAutoPlugin::AndroidAutoPlugin(QObject *parent) : QObject (parent)
{
    m_pluginSettings.eventListeners = QStringList() << "UsbConnectionListenerPlugin::UsbDeviceAdded" << "SYSTEM::SetNightMode" << "J2534::VSS" << "J2534::Gear" << "GPSD::Location";
    m_pluginSettings.events = QStringList() << "connected";
    gst_init(NULL, NULL);
    headunit = new Headunit();
    m_interfaceSettings.mediaStream = true;
    m_interfaceSettings.voiceStream = true;

    connect(headunit, &Headunit::playbackStarted, this, &AndroidAutoPlugin::playbackStarted);
    connect(headunit, &Headunit::statusChanged, this, &AndroidAutoPlugin::huStatusChanged);
}

QObject *AndroidAutoPlugin::getContextProperty(){
    return qobject_cast<QObject *>(headunit);
}

void AndroidAutoPlugin::eventMessage(QString id, QVariant message){
    if(id == "UsbConnectionListenerPlugin::UsbDeviceAdded"){
        if(headunit->status() != Headunit::RUNNING){
            headunit->startHU();
        }
    } else if(id == "UsbConnectionListenerPlugin::UsbDeviceRemoved"){
    } else if(id == "SYSTEM::SetNightMode"){
        headunit->setNigthmode(message.toBool());
    } else if(id == "J2534::VSS") {
        double speedms = message.toInt() / 3.6;
        headunit->setVSS(speedms);
    } else if(id == "J2534::Gear") {
        headunit->setGear(message.toInt());
    } else if(id == "GPSD::Location") {
        QVariantMap map = message.toMap();
        headunit->setLocation(map["latitude"].toDouble(),map["longitude"].toDouble(),
            map["track"].toDouble(),map["speed"].toDouble(),map["altitude"].toDouble(),map["eph"].toDouble());
    }
}
void AndroidAutoPlugin::init(){
    headunit->init();
    headunit->startHU();
}

void AndroidAutoPlugin::start() {
    headunit->startMedia();
}
void AndroidAutoPlugin::stop() {
    headunit->stopMedia();
}
void AndroidAutoPlugin::prevTrack() {
    headunit->prevTrack();
}
void AndroidAutoPlugin::nextTrack() {
    headunit->nextTrack();
}
void AndroidAutoPlugin::setMediaVolume(uint8_t volume) {
    headunit->setMediaVolume(volume);
}
void AndroidAutoPlugin::setVoiceVolume(uint8_t volume) {
    headunit->setVoiceVolume(volume);
}

void AndroidAutoPlugin::huStatusChanged(){
    emit message("connected", (headunit->status() > Headunit::NO_CONNECTION));
}
