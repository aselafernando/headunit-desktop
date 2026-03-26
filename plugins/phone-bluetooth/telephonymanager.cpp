#include <QtDebug>
#include <QLoggingCategory>

#include "telephonymanager.h"

Q_LOGGING_CATEGORY(LOG_PLUGINS_PHONEBLUETOOTH_BLUEZQT, "plugins.phone-bluetooth.bluezqt")
Q_LOGGING_CATEGORY(LOG_PLUGINS_PHONEBLUETOOTH_OFONO, "plugins.phone-bluetooth.ofono")

TelephonyManager::TelephonyManager(QObject *parent) : QObject(parent),
      m_mediaTrackTimer(this), m_bluez_manager(this), m_obexManager(this), m_ofonoManagerClass(this), m_phonebookModel(this), m_callHistoryModel(this)
{
    m_contactsFolder = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/contacts";

    m_interfaceSettings.mediaStream = true;

    m_pluginSettings.eventListeners = QStringList() << "AndroidAuto::connected";

    reconnectTimer.setSingleShot(true);
    connect(&reconnectTimer, &QTimer::timeout, this, &TelephonyManager::connectToNextDevice);

    BluezQt::InitManagerJob *job = m_bluez_manager.init();
    job->start();
    connect(job, &BluezQt::InitManagerJob::result, this, &TelephonyManager::initBluez);

    qDBusRegisterMetaType<ObjectPathProperties>();
    qDBusRegisterMetaType<ObjectPathPropertiesList>();

    connect(&m_mediaTrackTimer, &QTimer::timeout, this, &TelephonyManager::mediaTrackTimerElapsed);

}
TelephonyManager::~TelephonyManager(){
    //    delete m_activeDevice;
    m_mediaTrackTimer.stop();
}

QObject *TelephonyManager::getContextProperty(){
    return this;
}

void TelephonyManager::init() {
    m_ofonoManagerClass.init();
    qDBusRegisterMetaType<QPair<QString,QString>>();
    qDBusRegisterMetaType<QList<QPair<QString,QString>>>();

    m_pluginSettings.actions = QStringList() << "Answer" << "Hangup" << "VoiceRecognition";

    connect(&m_bluez_manager, &BluezQt::Manager::deviceAdded, this, &TelephonyManager::deviceAdded);
    connect(&m_bluez_manager, &BluezQt::Manager::deviceRemoved, this, &TelephonyManager::deviceRemoved);

    connect(&m_ofonoManagerClass, &OfonoManager::showOverlay, this, &TelephonyManager::showOverlay);
    connect(&m_ofonoManagerClass, &OfonoManager::hideOverlay, this, &TelephonyManager::hideOverlay);
    connect(&m_ofonoManagerClass, &OfonoManager::callFinished, this, &TelephonyManager::pullCallHistory);


    connect(&m_phonebookWatcher, &QFileSystemWatcher::fileChanged, this, &TelephonyManager::contactsChanged);
    connect(&m_phonebookWatcher, &QFileSystemWatcher::directoryChanged, this, &TelephonyManager::contactsFolderChanged);

    m_phonebookWatcher.addPath(m_contactsFolder);
    m_phonebookWatcher.addPath(m_contactsFolder + "/callHistory.vcf");
    m_phonebookWatcher.addPath(m_contactsFolder + "/contacts.vcf");
}

void TelephonyManager::initObex (BluezQt::InitObexManagerJob *job){
    disconnect(job, &BluezQt::InitObexManagerJob::result, this, &TelephonyManager::initObex);
    if(job->error() == BluezQt::Job::Error::NoError){
        qCDebug(LOG_PLUGINS_PHONEBLUETOOTH_BLUEZQT)  << "obex manager started";
        if(m_activeDevice){
            getPhonebooks(m_activeDevice->address());
        }
    } else {
        qCWarning(LOG_PLUGINS_PHONEBLUETOOTH_BLUEZQT)  << "initObex : "<< job->error() << job->errorText();
    }
}

void TelephonyManager::initBluez (BluezQt::InitManagerJob *job){
    disconnect(job, &BluezQt::InitManagerJob::result, this, &TelephonyManager::initBluez);
    if(job->error() == BluezQt::Job::Error::NoError){
        qCDebug(LOG_PLUGINS_PHONEBLUETOOTH_BLUEZQT)  << "Manager Started";

        bluez_agent = new BluezAgent();
        BluezQt::PendingCall *agent_pcall = m_bluez_manager.registerAgent(bluez_agent);
        agent_pcall->waitForFinished();
        if(agent_pcall->error() != BluezQt::PendingCall::NoError){
            qCWarning(LOG_PLUGINS_PHONEBLUETOOTH_BLUEZQT)  << "Error registerAgent  : " << agent_pcall->errorText();
        } else {
            qCDebug(LOG_PLUGINS_PHONEBLUETOOTH_BLUEZQT)  << "Agent registered";
        }

        agent_pcall = m_bluez_manager.requestDefaultAgent(bluez_agent);
        agent_pcall->waitForFinished();
        if(agent_pcall->error() != BluezQt::PendingCall::NoError){
            qCWarning(LOG_PLUGINS_PHONEBLUETOOTH_BLUEZQT)  << "Error requestDefaultAgent  : " << agent_pcall->errorText();
        } else {
            qCDebug(LOG_PLUGINS_PHONEBLUETOOTH_BLUEZQT)  << "Agent set as default";
        }

        if(m_bluez_manager.adapters().size() > 0) {
            initAdapter(m_bluez_manager.adapters().at(0));
        }

        BluezQt::InitObexManagerJob *obexjob = m_obexManager.init();
        obexjob->start();
        connect(obexjob, &BluezQt::InitObexManagerJob::result, this, &TelephonyManager::initObex);
    } else {
        qCWarning(LOG_PLUGINS_PHONEBLUETOOTH_BLUEZQT)  << "Error : bluezManagerStartResult : " << job->errorText();
        return;
    }
}

void TelephonyManager::initAdapter(BluezQt::AdapterPtr adapter) {
    m_bluez_adapter = adapter.data();
    connect(m_bluez_adapter, &BluezQt::Adapter::discoveringChanged, this, &TelephonyManager::deviceDiscoveringChanged);
    m_bluez_adapter->setName(m_settings["adapterName"].toString());
    m_bluez_adapter->setPowered(true);
    m_bluez_adapter->setDiscoverable(false);
    m_bluez_adapter->setPairable(false);

    //If the newly added device is trusted then disconnect from all other devices and connect to it

    for(BluezQt::DevicePtr p_device : m_bluez_manager.devices()) {
        if(p_device->isConnected()){
            if(!m_activeDevice){
                qCDebug(LOG_PLUGINS_PHONEBLUETOOTH_BLUEZQT)  << "Connected device found: " << p_device->name();
                setBluezDevice(p_device.data());
            } else {
                qCDebug(LOG_PLUGINS_PHONEBLUETOOTH_BLUEZQT)  << "Disconnecting from : " << p_device->name();
                p_device->disconnectFromDevice();
            }
        }
    }
    if(!m_activeDevice) {
        qCDebug(LOG_PLUGINS_PHONEBLUETOOTH_BLUEZQT)  << "No device connected, trying to connect";
        connectToNextDevice();
    }
}

void TelephonyManager::initOfono(QString ubi){

    qCDebug(LOG_PLUGINS_PHONEBLUETOOTH_OFONO) << "Init oFono";

    if(!m_ofonoManagerClass.setDefaultModem("/hfp" + ubi)){
        qCWarning(LOG_PLUGINS_PHONEBLUETOOTH_OFONO)  << ": No modem found";
    }
}

void TelephonyManager::updateAdapters(){
    m_adapters.clear();
    //    for(BluezQt::AdapterPtr adapter : m_bluez_manager.adapters()){
    //        m_adapters.append(adapter->systemName());
    //    }
    emit adaptersUpdated();
}

void TelephonyManager::contactsChanged(const QString &path) {
    QFile file(path);
    if(file.exists()){
        if(QUrl(path).fileName() == "contacts.vcf"){
            m_phonebookModel.importContacts(QUrl::fromLocalFile(m_contactsFolder + "/contacts.vcf"));
        } else  if(QUrl(path).fileName() == "callHistory.vcf") {
            m_callHistoryModel.importContacts(QUrl::fromLocalFile(m_contactsFolder + "/callHistory.vcf"));
        }
    }
}

void TelephonyManager::contactsFolderChanged(const QString &path) {
    QDir dir(path);
    if(dir.exists()){
        m_phonebookWatcher.addPath(path+"/contacts.vcf");
        m_phonebookWatcher.addPath(path+"/callHistory.vcf");
    }
}

void TelephonyManager::pullPhonebook (QString path, QString type, QString output) {
    qCDebug(LOG_PLUGINS_PHONEBLUETOOTH_BLUEZQT)  << "Pulling phonebook";

    QFile::remove(output);

    org::bluez::obex::PhonebookAccess1 pbapAccess("org.bluez.obex", path, QDBusConnection::sessionBus(), this);

    //Select phone's internal phonebook
    QDBusPendingCall selectCall = pbapAccess.Select("int", type);
    selectCall.waitForFinished();

    if(selectCall.isError()){
        qCWarning(LOG_PLUGINS_PHONEBLUETOOTH_BLUEZQT)  << "Error phonebook select:"<< selectCall.error().message();
        return;
    }

    //Pull all entries from the phone's internal phonebook
    pbapAccess.PullAll(output, QVariantMap());
}

void TelephonyManager::getPhonebooks(QString destination, bool callHistoryOnly){
    if(m_obexManager.isOperational()){
        QDir dir(m_contactsFolder);
        if(!dir.exists()){
            dir.mkpath(m_contactsFolder);
            m_phonebookWatcher.addPath(m_contactsFolder);
        }

        QVariantMap args;
        args.insert("Target", "PBAP");
        BluezQt::PendingCall * call = m_obexManager.createSession(destination, args);

        connect(call, &BluezQt::PendingCall::finished, this,
                [=, this]() {
                    if(call->value().canConvert<QDBusObjectPath>()) {
                        QDBusObjectPath objectPath = qvariant_cast<QDBusObjectPath>(call->value());
                        pullPhonebook(objectPath.path(), "cch", m_contactsFolder + "/callHistory.vcf");

                        if(!callHistoryOnly) {
                            pullPhonebook(objectPath.path(), "pb", m_contactsFolder + "/contacts.vcf");

                        }
                    }
                }
                );
    } else {
        qCWarning(LOG_PLUGINS_PHONEBLUETOOTH_BLUEZQT)  << "obexManager is not operational";
        return;
    }
}
void TelephonyManager::setBluezDevice(BluezQt::Device *device) {
    if(m_activeDevice){
        m_pairedDevices.insert(m_activeDevice->ubi(), m_activeDevice->name());
    }
    if(device){
        m_pairedDevices.remove(device->ubi());
        qCDebug(LOG_PLUGINS_PHONEBLUETOOTH_BLUEZQT)  << "Device set : " << device->name();
        initOfono(device->ubi());
    } else {
        qCDebug(LOG_PLUGINS_PHONEBLUETOOTH_BLUEZQT)  << "Device set : NULL";
    }
    m_activeDevice = device;
    initMediaPlayer();
    emit activeDeviceChanged();
    emit pairedDevicesChanged();
}

void TelephonyManager::deviceAdded(BluezQt::DevicePtr device){
    qCDebug(LOG_PLUGINS_PHONEBLUETOOTH_BLUEZQT)  << "Device added : " << device->name();

    connect(device.data(), &BluezQt::Device::connectedChanged, this, &TelephonyManager::deviceConnectionChanged);
    connect(device.data(), &BluezQt::Device::pairedChanged , this, &TelephonyManager::devicePairedChanged);
    if(device->isPaired()){
        m_pairedDevices.insert(device->ubi(),device->name());
    }
    emit pairedDevicesChanged();
}
void TelephonyManager::deviceRemoved(BluezQt::DevicePtr device){
    qCDebug(LOG_PLUGINS_PHONEBLUETOOTH_BLUEZQT)  << "Device removed : " << device->name();
    if(m_activeDevice == device){
        setBluezDevice(nullptr);
    }

    disconnect(device.data(), &BluezQt::Device::connectedChanged, this, &TelephonyManager::deviceConnectionChanged);
    disconnect(device.data(), &BluezQt::Device::pairedChanged, this, &TelephonyManager::devicePairedChanged);
    m_pairedDevices.remove(device->ubi());
    emit pairedDevicesChanged();
}

void TelephonyManager::onMediaPosition(quint32 position) {
    //TODO: Investigate why this only gets triggered sporadically or when the track changes or status changes
    // Have implemented a 2Hz timer instead to get around this
    m_mediaTrackPosition = position;
    emit mediaPositionChanged(position);
    m_mediaTrackGotPosition = true;
    m_mediaTrackTimer.start(500);
}

void TelephonyManager::mediaTrackTimerElapsed() {
    if(m_activeDevice) {
        if(m_mediaTrackGotPosition) {
            m_mediaTrackPosition += 500;
            emit mediaPositionChanged(m_mediaTrackPosition);
        }
    } else {
        m_mediaTrackPosition = 0;
        m_mediaTrackGotPosition = false;
        m_mediaTrackTimer.stop();
        emit mediaPositionChanged(m_mediaTrackPosition);
    }
}

void TelephonyManager::onMediaStatus(BluezQt::MediaPlayer::Status status) {
    qCDebug(LOG_PLUGINS_PHONEBLUETOOTH_BLUEZQT) << "Media player status: " << status;
    switch(status) {
        case BluezQt::MediaPlayer::Playing:
            if(!m_androidAutoConnected)
                emit playbackStarted();
            break;
        default:
            m_mediaTrackTimer.stop();
            m_mediaTrackGotPosition = false;
            break;
    }
}

void TelephonyManager::onMediaTrack(BluezQt::MediaPlayerTrack track) {
    qCDebug(LOG_PLUGINS_PHONEBLUETOOTH_BLUEZQT) << "Media player track: #" << track.trackNumber() << " | " << track.artist() << track.title();
    m_mediaTrackGotPosition = false;

    QVariantMap vTrack;
    vTrack.insert("number",track.trackNumber());
    vTrack.insert("artist",track.artist());
    vTrack.insert("title",track.title());
    vTrack.insert("duration",track.duration());
    emit trackChanged(vTrack);
}

void TelephonyManager::onMediaPlayer(BluezQt::MediaPlayerPtr mediaPlayer) {
    if(!mediaPlayer.isNull()) {
        connect(mediaPlayer.get(), &BluezQt::MediaPlayer::positionChanged, this, &TelephonyManager::onMediaPosition);
        connect(mediaPlayer.get(), &BluezQt::MediaPlayer::trackChanged, this, &TelephonyManager::onMediaTrack);
        connect(mediaPlayer.get(), &BluezQt::MediaPlayer::statusChanged, this, &TelephonyManager::onMediaStatus);
    } else {
        disconnect(mediaPlayer.get(), &BluezQt::MediaPlayer::positionChanged, this, &TelephonyManager::onMediaPosition);
        disconnect(mediaPlayer.get(), &BluezQt::MediaPlayer::trackChanged, this, &TelephonyManager::onMediaTrack);
        disconnect(mediaPlayer.get(), &BluezQt::MediaPlayer::statusChanged, this, &TelephonyManager::onMediaStatus);
    }
}

void TelephonyManager::initMediaPlayer() {
    if(m_activeDevice) {
        BluezQt::MediaPlayerPtr mediaPlayer = m_activeDevice->mediaPlayer();
        if(!mediaPlayer.isNull()) {
            connect(m_activeDevice, &BluezQt::Device::mediaPlayerChanged, this, &TelephonyManager::onMediaPlayer);
            onMediaPlayer(mediaPlayer);
            onMediaTrack(mediaPlayer->track());
            onMediaPosition(mediaPlayer->position());
            onMediaStatus(mediaPlayer->status());
        } else {
            disconnect(m_activeDevice, &BluezQt::Device::mediaPlayerChanged, this, &TelephonyManager::onMediaPlayer);
        }
    }
}

void TelephonyManager::deviceConnectionChanged(bool connected){
    BluezQt::Device* device = dynamic_cast<BluezQt::Device*>(QObject::sender());
    if(connected){
        qCDebug(LOG_PLUGINS_PHONEBLUETOOTH_BLUEZQT) << "Device connected : " << device->name() << device->ubi();

        for(BluezQt::DevicePtr p_device : m_bluez_manager.devices()){
            if(device != p_device.data()){
                if(p_device->isConnected()){
                    qCDebug(LOG_PLUGINS_PHONEBLUETOOTH_BLUEZQT) << "Disconnecting : " << p_device->name();
                    p_device->disconnectFromDevice();
                }
            }
        }
        setBluezDevice(device);

        initMediaPlayer();
        getPhonebooks(m_activeDevice->address());

    } else {
        qCDebug(LOG_PLUGINS_PHONEBLUETOOTH_BLUEZQT) << "Device disconnected : " << device->name();
        if(m_activeDevice == device){
            setBluezDevice(nullptr);
            qCDebug(LOG_PLUGINS_PHONEBLUETOOTH_BLUEZQT) << "Trying again : " << device->name();
            m_previouslyTriedDevice = device;
            BluezQt::PendingCall * connectCall = device->connectToDevice();
            connectCall->setUserData(device->ubi());
            connect(connectCall, &BluezQt::PendingCall::finished, this, &TelephonyManager::connectToDeviceCallback);
        }
    }
}

void TelephonyManager::devicePairedChanged(bool paired) {
    BluezQt::Device* device = dynamic_cast<BluezQt::Device*>(QObject::sender());
    if(paired && device->isConnected()){
        deviceConnectionChanged(true);
    }
}

void TelephonyManager::deviceDiscoveringChanged(bool discovering){
    qCDebug(LOG_PLUGINS_PHONEBLUETOOTH_BLUEZQT)  << "Discovering changed : " << discovering;
}

void TelephonyManager::connectToNextDevice(){
    BluezQt::Device *device = nullptr;
    bool activeDeviceFound = false;
    for(BluezQt::DevicePtr p_device : m_bluez_manager.devices()){
        if(p_device->isTrusted()){
            if(!m_previouslyTriedDevice) {
                device = p_device.data();
                break;
            } else if(p_device == m_previouslyTriedDevice){
                activeDeviceFound = true;
            } else if(activeDeviceFound){
                device = p_device.data();
                break;
            }
        }
    }

    m_previouslyTriedDevice = device;

    if(device){
        qCDebug(LOG_PLUGINS_PHONEBLUETOOTH_BLUEZQT)  << "Connecting to : " << device->name();
        BluezQt::PendingCall * connectCall = device->connectToDevice();
        connectCall->setUserData(device->ubi());
        connect(connectCall, &BluezQt::PendingCall::finished, this, &TelephonyManager::connectToDeviceCallback);
    } else {
        if(activeDeviceFound) {
            reconnectTimer.start(10000);
        }
    }
}

void TelephonyManager::connectToDeviceCallback(BluezQt::PendingCall *call){
    disconnect(call, &BluezQt::PendingCall::finished, this, &TelephonyManager::connectToDeviceCallback);
    BluezQt::DevicePtr device = m_bluez_manager.deviceForUbi(call->userData().toString());
    if(call->error() == BluezQt::PendingCall::NoError){
        qCDebug(LOG_PLUGINS_PHONEBLUETOOTH_BLUEZQT)  << "Finished connecting to : " << device->name();
    } else {
        qCDebug(LOG_PLUGINS_PHONEBLUETOOTH_BLUEZQT)  << "Error connecting to : " << device->name() << " : " << call->errorText();
        if(device == m_previouslyTriedDevice){
            connectToNextDevice();
        }
    }
}

void TelephonyManager::enablePairing(){
    if(m_bluez_adapter){
        m_bluez_adapter->setDiscoverable(true);
        m_bluez_adapter->setPairable(true);
    }
}
void TelephonyManager::disablePairing(){
    if(m_bluez_adapter){
        m_bluez_adapter->setDiscoverable(false);
        m_bluez_adapter->setPairable(false);
    }
}

void TelephonyManager::settingsChanged(const QString &key, const QVariant &value){
    if(key == "adapterName") {
        if(value.canConvert<QString>()){
            m_bluez_adapter->setName(value.toString());
        }
    }
}

void TelephonyManager::connectToDevice(QString ubi){
    BluezQt::DevicePtr device = m_bluez_manager.deviceForUbi(ubi);
    if(device){
        if(m_activeDevice){
            m_activeDevice->disconnectFromDevice();
        }
        device->connectToDevice();
    }
}
void TelephonyManager::mediaPlaybackStarted() {
    if(!m_androidAutoConnected){
        emit playbackStarted();
    }
}

void TelephonyManager::eventMessage(QString id, QVariant message) {
    if(id == "AndroidAuto::connected"){
        m_androidAutoConnected = message.toBool();
    }
}

//Allow control from external plugins
void TelephonyManager::actionMessage(QString id, __attribute__((unused)) QVariant message) {
    qCDebug(LOG_PLUGINS_PHONEBLUETOOTH_BLUEZQT) << "Action Message: " << id;

    if (id == "Answer") {
        m_ofonoManagerClass.answerCall();
    }
    else if (id == "Hangup") {
        m_ofonoManagerClass.hangupCall();
    }
    else if (id == "VoiceControl") {
        m_ofonoManagerClass.activateVoiceControl();
    }
}

void TelephonyManager::showOverlay(){
    if(m_androidAutoConnected) {
        emit action("GUI::changePageIndex", 0);
    } else {
        QVariantMap map;
        map["source"] = "qrc:/PhoneBluetooth/CallNotification.qml";
        emit action("GUI::OpenOverlay", map);
    }
}

void TelephonyManager::hideOverlay() {
    if(m_androidAutoConnected) {
        emit action("GUI::changePagePrevIndex", 0);
    } else {
        emit action("GUI::CloseOverlay", QVariant());
    }
}

void TelephonyManager::pullCallHistory() {
    getPhonebooks(m_activeDevice->address(), true);
}

