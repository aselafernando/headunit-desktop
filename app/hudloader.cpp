#include "hudloader.h"

Q_LOGGING_CATEGORY(LOG_APP_HUDLOADER, "app.hudloader")

InitThread::InitThread(HUDLoader *hudLoader)
    : QThread(hudLoader)
    , m_hudLoader(hudLoader)
{
}
void InitThread::run()
{
    m_hudLoader->init();
    emit initFinished();
}

HUDLoader::HUDLoader(QQmlApplicationEngine *engine, bool lazyLoading, QStringList pluginWhitelist, QObject *parent)
    : QObject(parent)
    , m_engine(engine)
    , m_initThread(this)
    , m_pluginWhitelist(pluginWhitelist)
    , m_lazyLoading(lazyLoading)
{
    qCDebug(LOG_APP_HUDLOADER) << "Registering HUDLoader";
    qmlRegisterUncreatableType<HUDLoader>("HUDLoader", 1, 0, "HUDLoader", "Error PluginList is uncreatable");
}

void HUDLoader::load()
{
    qCDebug(LOG_APP_HUDLOADER) << "Registering types";
    qmlRegisterType<PluginListProxyModel>("HUDPlugins", 1, 0, "PluginListModel");
    qmlRegisterAnonymousType<PluginObject>("HUDPlugins", 1);

    QCoreApplication::processEvents();

    qCDebug(LOG_APP_HUDLOADER) << "Loading Plugin List";
    m_pluginList = new PluginList(this);

    qCDebug(LOG_APP_HUDLOADER) << "Loading Media Manager";
    m_mediaManager = new MediaManager(this);

    qCDebug(LOG_APP_HUDLOADER) << "Loading Bottom Bar Model";
    m_bottomBarModel = new PanelItemsModel(this);

    qCDebug(LOG_APP_HUDLOADER) << "Registering singletons";
    qmlRegisterSingletonInstance("HUDPlugins", 1, 0, "PluginList", m_pluginList);
    qmlRegisterSingletonInstance("HUDPlugins", 1, 0, "BottomBarModel", m_bottomBarModel);
    qmlRegisterSingletonInstance("HUDPlugins", 1, 0, "MediaManager", m_mediaManager);

    qCDebug(LOG_APP_HUDLOADER) << "Loading Theme";
    m_themeManager = new ThemeManager(m_engine, m_pluginList, this);

    qCDebug(LOG_APP_HUDLOADER) << "Loading plugins";
    m_pluginManager = new PluginManager(m_engine, m_pluginList, m_mediaManager, this);

    connect(m_pluginManager, &PluginManager::themeEvent, m_themeManager, &ThemeManager::onEvent);
    connect(&m_initThread, &InitThread::initFinished, this, &HUDLoader::initFinished);
    connect(m_themeManager, &ThemeManager::themeLoaded, this, &HUDLoader::onThemeLoaded);

    m_pluginManager->loadPlugins(m_pluginWhitelist);

    QCoreApplication::processEvents();
    if (m_lazyLoading) {
        qCDebug(LOG_APP_HUDLOADER) << "Loading in a thread";
        m_initThread.start();
    } else {
        qCDebug(LOG_APP_HUDLOADER) << "Loading on main thread";
        init();
        initFinished();
    }
}

void HUDLoader::init()
{
    qCDebug(LOG_APP_HUDLOADER) << "Init theme";
    m_themeManager->initTheme("default-theme");
    qCDebug(LOG_APP_HUDLOADER) << "Init plugins";
    m_pluginList->initPlugins();
    m_mediaManager->init();
}
void HUDLoader::onThemeLoaded()
{
    m_themeManager->initFinished();
    emit themeSourceChanged(m_themeManager->getThemeSource());
}

void HUDLoader::initFinished()
{
    qCDebug(LOG_APP_HUDLOADER) << "Setting bottom bar plugin list";
    m_bottomBarModel->setPluginList(m_pluginList);
    m_bottomBarModel->removeUnusedItems();
}
