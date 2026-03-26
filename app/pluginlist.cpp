#include <QtDebug>
#include <QLoggingCategory>

#include "pluginlist.h"

Q_LOGGING_CATEGORY(LOG_APP_PLUGINLIST, "app.pluginlist")

PluginList::PluginList(QObject *parent) : QObject(parent)
{

    qCDebug(LOG_APP_PLUGINLIST) << "Plugin list constructed";
}

PluginList::~PluginList() {
    qCDebug(LOG_APP_PLUGINLIST) << "Plugin list destroyed";
}
void PluginList::initPlugins()  {
    for(PluginObject * plugin : std::as_const(m_plugins)){
        plugin->init();
    }
}

PluginObject *PluginList::addPlugin(QString pluginPath) {
    PluginObject *plugin = new PluginObject(pluginPath, this);

    m_plugins.append(plugin);
    return plugin;
}
void PluginList::addPlugin(PluginObject *plugin) {
    if(plugin) {
        m_plugins.append(plugin);
        int index = m_plugins.indexOf(plugin);
        emit pluginAdded(index);
    }
}

bool PluginList::containsPlugin(QString pluginName){
    for(PluginObject * plugin : std::as_const(m_plugins)){
        if(plugin->getName() == pluginName) {
            return true;
        }
    }
    return false;
}

PluginObject *PluginList::getPlugin(QString pluginName) {
    for(PluginObject * plugin : std::as_const(m_plugins)){
        if(plugin->getName() == pluginName) {
            return plugin;
        }
    }
    return nullptr;
}


void PluginList::handleMessage(QString id, QVariant message){
    for(PluginObject * plugin : std::as_const(m_plugins)){
        plugin->handleMessage(id, message);
    }
}

void PluginList::callSlot(QString pluginName, QString slot) {
    PluginObject * plugin = getPlugin(pluginName);
    if(plugin) {
        plugin->callSlot(slot);
    }
}
PluginObject *PluginList::at(int index){
    return m_plugins.at(index);
}
int PluginList::size(){
    return m_plugins.size();
}
int PluginList::indexOf(PluginObject * object) {
    return m_plugins.indexOf(object);
}
