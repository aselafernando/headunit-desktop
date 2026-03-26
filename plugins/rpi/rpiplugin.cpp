#include <QFileInfo>
#include <QFile>
#include <QJsonValue>
#include <QDebug>
#include <QLoggingCategory>

#include "rpiplugin.h"

Q_LOGGING_CATEGORY(LOG_PLUGINS_RPI, "plugins.rpi")

RPiPlugin::RPiPlugin(QObject *parent) : QObject (parent)
{
    m_pluginSettings.eventListeners = QStringList() << "SYSTEM::SetNightMode";
}
void RPiPlugin::init(){
    connect(&m_settings, &QQmlPropertyMap::valueChanged, this, &RPiPlugin::settingsChanged);
    if (m_settings.contains("brightness")) {
        applyBrightness(m_settings.value("brightness").toInt());
    } else {
        applyBrightness(255);
    }
}
// maps slider value (0-100) to (0-255)
int RPiPlugin::mapBrightness(int v) {
    return v * (245/100) + 10;
}

void RPiPlugin::applyBrightness(int v) {
    if(v > 100){
        v = 100;
    }else if(v < 0){
        v = 0;
    }

    QFile file("/sys/class/backlight/rpi_backlight/brightness");
    if (file.open(QIODevice::WriteOnly | QIODevice::ExistingOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        stream << mapBrightness(v);
        file.close();
    }
}

void RPiPlugin::settingsChanged(const QString &key, const QVariant &value){
    if (key == "brightness") {
        qCDebug(LOG_PLUGINS_RPI) << "settingChanged : " << value;
        applyBrightness(value.toInt());
    }
}

void RPiPlugin::eventMessage(QString id, QVariant message) {
    if(id == "SYSTEM::SetNightMode"){
        if(message.toBool()){
            applyBrightness(1);
        } else {
            applyBrightness(100);
        }
    }
}

