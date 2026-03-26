#include <PulseAudioQt/Models>
#include <PulseAudioQt/Sink>

#include <QtDebug>
#include <QLoggingCategory>

#include "volumecontrol.h"

Q_LOGGING_CATEGORY(LOG_PLUGINS_VOLUMECONTROL, "plugins.volume-control")

VolumeControl::VolumeControl(QObject *parent) : QObject (parent)
{
    qmlRegisterType<PulseAudioQt::CardModel>("HUDVolume", 1, 0, "CardModel");
    qmlRegisterType<PulseAudioQt::SinkModel>("HUDVolume", 1, 0, "SinkModel");
    qmlRegisterType<PulseAudioQt::SinkInputModel>("HUDVolume", 1, 0, "SinkInputModel");
    qmlRegisterType<PulseAudioQt::SourceModel>("HUDVolume", 1, 0, "SourceModel");
    qmlRegisterType<PulseAudioQt::SourceOutputModel>("HUDVolume", 1, 0, "SourceOutputModel");
    qmlRegisterType<PulseAudioQt::StreamRestoreModel>("HUDVolume", 1, 0, "StreamRestoreModel");
    qmlRegisterType<PulseAudioQt::ModuleModel>("HUDVolume", 1, 0, "ModuleModel");
}

VolumeControl::~VolumeControl() {
    if(server) {
        disconnect(server, &PulseAudioQt::Server::defaultSinkChanged, this, &VolumeControl::defaultSinkChanged);
    }
}

void VolumeControl::init(){
    defaultSinkChanged();
    m_pluginSettings.actions = QStringList() << "VolumeUp" << "VolumeDown";

    context = PulseAudioQt::Context::instance();
    server = context->server();

    if (server) {
        connect(server, &PulseAudioQt::Server::defaultSinkChanged, this, &VolumeControl::defaultSinkChanged);
    }
}

void VolumeControl::setDefaultVolume(int volume){
    m_settings["volume"] = volume;

    emit m_settings.valueChanged("volume", volume);
}

void VolumeControl::defaultSinkChanged() {
    if(server == nullptr) return;

    PulseAudioQt::Sink * defaultSink = server->defaultSink();
    if(defaultSink != nullptr) {
        defaultSink->setVolume(m_settings["volume"].toInt());
        qCDebug (LOG_PLUGINS_VOLUMECONTROL) << "Setting volume to : " << m_settings["volume"].toInt();
    }
}

QObject *VolumeControl::getContextProperty(){
    return this;
}

void VolumeControl::actionMessage(QString id, __attribute__((unused)) QVariant message){
    if(server == nullptr) return;

    PulseAudioQt::Sink * defaultSink = server->defaultSink();
    if(defaultSink != nullptr) {
        int volume = 0;
        if(id == "VolumeUp"){
            volume = defaultSink->volume() + (655 * 4);
        } else if(id == "VolumeDown") {
            volume = defaultSink->volume() - (655 * 4);
        }
        if(volume > PulseAudioQt::normalVolume()){
            volume = PulseAudioQt::normalVolume();
        } else if (volume < PulseAudioQt::minimumVolume()) {
            volume = PulseAudioQt::minimumVolume();
        } else {
        }
        defaultSink->setVolume(volume);
        m_settings["volume"] = volume;

        emit m_settings.valueChanged("volume", volume);
    }
}
