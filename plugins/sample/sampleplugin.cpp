#include <QDebug>
#include <QLoggingCategory>

#include "sampleplugin.h"

Q_LOGGING_CATEGORY(LOG_PLUGINS_SAMPLE, "plugins.sample")


SamplePlugin::SamplePlugin(QObject *parent) : QObject (parent)
{

}

void SamplePlugin::init() {
}

QObject *SamplePlugin::getContextProperty(){
    return this;
}

void SamplePlugin::eventMessage(__attribute__((unused)) QString id, __attribute__((unused)) QVariant message) {
}

void SamplePlugin::actionMessage(__attribute__((unused)) QString id, __attribute__((unused)) QVariant message) {
}

void SamplePlugin::testNotification(){
    emit message("GUI::Notification", "{\"image\":\"qrc:/qml/icons/alert.png\", \"title\":\"Test notification\", \"description\":\"This is a test notification\"}");
}


void SamplePlugin::onSettingsPageDestroyed() {
    qCDebug(LOG_PLUGINS_SAMPLE) << "Sample plugin destroyed";
}
