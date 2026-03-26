#ifndef VOLUMECONTROL_H
#define VOLUMECONTROL_H

#include <QObject>
#include <QQmlEngine>
#include <QDebug>
#include <PulseAudioQt/Context>
#include <PulseAudioQt/Server>
#include <plugininterface.h>
/*
#include "pulseaudio-qt/src/models.h"
#include "pulseaudio-qt/src/sink.h"
#include "pulseaudio-qt/src/context.h"
*/

class VolumeControl : public QObject, PluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.viktorgino.headunit.volumecontrol" FILE "config.json")
    Q_INTERFACES(PluginInterface)
public:
    explicit VolumeControl(QObject *parent = nullptr);
    ~VolumeControl();

    void init() override;
    QObject *getContextProperty() override;

    Q_INVOKABLE void setDefaultVolume(int volume);
    Q_INVOKABLE void onSettingsPageDestroyed();

public slots:
    void actionMessage(QString id, QVariant message) override;
private slots:
    void defaultSinkChanged();
private:
    PulseAudioQt::Context* context = nullptr;
    PulseAudioQt::Server* server = nullptr;
//    PulseAudioQt::SinkModel m_sinkModel;
};

#endif // VOLUMECONTROL_H
