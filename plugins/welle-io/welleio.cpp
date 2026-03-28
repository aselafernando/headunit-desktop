#include <QtDebug>
#include <QLoggingCategory>

#include "welleio.h"

Q_LOGGING_CATEGORY(LOG_PLUGINS_WELLEIO, "plugins.welle-io")

WelleIoPlugin::WelleIoPlugin(QObject *parent) : QObject (parent), m_translator(this)
{
    QVariantMap commandLineOptions;
    commandLineOptions["dumpFileName"] = "test.dump";

    m_radioController = new CRadioController (commandLineOptions, this);
    m_guiHelper = new CGUIHelper (m_radioController, this);
}

void WelleIoPlugin::handleMotChanged(QString pictureName, QString categoryTitle, int categoryId, int slideId) {
    if(this->motImg) {
        qCDebug(LOG_PLUGINS_WELLEIO) << "MOT Image Changed -"<< "PictureName:" << pictureName << "Category Title:" << categoryTitle << "Category ID:"<< categoryId << "Slide ID:" << slideId;
        this->motImg->setProperty("source", QUrl("image://SLS/" + pictureName));
    }
}

void WelleIoPlugin::handleMotReseted() {
    if(this->motImg) {
        qCDebug(LOG_PLUGINS_WELLEIO) << "Resetting MOT image";
        this->motImg->setProperty("source", QUrl("image://SLS/empty"));
    }
}

void WelleIoPlugin::init(){
    m_guiHelper->setTranslator(&m_translator);
    connect(m_guiHelper, &CGUIHelper::motChanged, this, &WelleIoPlugin::handleMotChanged);
    connect(m_guiHelper, &CGUIHelper::motReseted, this, &WelleIoPlugin::handleMotReseted);

    connect(&m_settings, &QQmlPropertyMap::valueChanged, this, &WelleIoPlugin::settingsChanged);

    for(QString key : m_settings.keys()) {
        settingsChanged(key, m_settings[key]);
    }

    onSettingsPageDestroyed();
    m_radioController->selectFFTWindowPlacement(1);
    m_radioController->setFreqSyncMethod(1);
    playLastStation();
}

void WelleIoPlugin::imageItemLoaded(QQuickItem *img) {
    QQmlEngine *engine = qmlEngine(img);
    if (img && engine && m_guiHelper) {
        qCDebug(LOG_PLUGINS_WELLEIO) << "SLS image provider set";
        engine->addImageProvider(QLatin1String("SLS"), m_guiHelper->motImageProvider);
    } else {
        qCDebug(LOG_PLUGINS_WELLEIO) << "Unable to set SLS image provider!";
    }

    this->motImg = img;
}

void WelleIoPlugin::settingsChanged(const QString &key, const QVariant &value){
    if(key == "auto_gain"){
        m_radioController->setAGC(value.toBool());
    } else if(key == "gain"){
        m_radioController->setGain(value.toInt());
    } else if(key == "auto_receiver"){
        m_adapterChanged = true;
    } else if(key == "receiver"){
        m_adapterChanged = true;
    } else if(key == "airspy_bias_tee"){
        m_guiHelper->setBiasTeeAirspy(value.toBool());
    } else if(key == "rtlsdr_bias_tee"){
        m_guiHelper->setBiasTeeRtlSdr(value.toBool());
    } else if(key == "soapysdr_antenna"){
        m_guiHelper->setAntennaSoapySdr(value.toString());
    } else if(key == "soapysdr_clock"){
        m_guiHelper->setClockSourceSoapySdr(value.toString());
    } else if(key == "soapysdr_driver_args"){
        m_guiHelper->setDriverArgsSoapySdr(value.toString());
    } else if(key == "rtltcp_host"){
        m_adapterChanged = true;
    } else if(key == "rtltcp_port"){
        m_adapterChanged = true;
    } else if(key == "raw_file"){
        m_adapterChanged = true;
    } else if(key == "raw_type"){
        m_adapterChanged = true;
    } else if(key == "coarse_corrector"){
        m_radioController->disableCoarseCorrector(!value.toBool());
    } else if(key == "coarse_corrector_algo"){
        m_radioController->setFreqSyncMethod(value.toInt());
    } else if(key == "fft_window_algo"){
        m_radioController->selectFFTWindowPlacement(value.toInt());
    }
}

void WelleIoPlugin::onSettingsPageDestroyed() {
    qCDebug(LOG_PLUGINS_WELLEIO) << "WelleIoPlugin onSettingsPageDestroyed";
    if(m_adapterChanged){
        if(m_settings["auto_receiver"].toBool()){
            qCDebug(LOG_PLUGINS_WELLEIO) << "Auto finding receiver";
            m_guiHelper->openAutoDevice();
        } else {
            //COmbobox needs to convert to string first then to int
            int rx = m_settings["receiver"].toString().toInt();
            switch (rx) {
            case 1:
                qCDebug(LOG_PLUGINS_WELLEIO) << "Opening Airspy";
                m_guiHelper->openAirspy();
                break;
            case 2:
                qCDebug(LOG_PLUGINS_WELLEIO) << "Opening RTL-SDR";
                m_guiHelper->openRtlSdr();
                break;
            case 3:
                qCDebug(LOG_PLUGINS_WELLEIO) << "Opening SoapySDR";
                m_guiHelper->openSoapySdr();
                break;
            case 4:
                qCDebug(LOG_PLUGINS_WELLEIO) << "Opening RTL-TCP";
                m_guiHelper->openRtlTcp(m_settings["rtltcp_host"].toString(), m_settings["rtltcp_port"].toInt(), true);
                break;
            case 5:
                qCDebug(LOG_PLUGINS_WELLEIO) << "Opening Raw";
                m_guiHelper->openRawFile(m_settings["raw_file"].toString(), m_settings["raw_type"].toString());
                break;
            default:
                qCDebug(LOG_PLUGINS_WELLEIO) << "Unknown receiver specified!" << rx;
                break;
            }

        }
        m_adapterChanged = false;
    }

}

void WelleIoPlugin::play(QString channel, QString title, quint32 service) {
    QSettings settings;
    settings.beginGroup("WelleIoPlugin");
    settings.setValue("lastChannel", channel);
    settings.setValue("lastChannelTitle", title);
    settings.setValue("lastService", service);

    m_radioController->play(channel,title,service);
}


void WelleIoPlugin::playLastStation() {
    QSettings settings;
    settings.beginGroup("WelleIoPlugin");

    if(settings.contains("lastChannel") && settings.contains("lastService")){
        m_radioController->play(settings.value("lastChannel").toString(), settings.value("lastChannelTitle").toString(), settings.value("lastService").toULongLong());
    }
}

WelleIoPlugin::~WelleIoPlugin() {
    qCDebug(LOG_PLUGINS_WELLEIO) << "Deleting WelleIoPlugin";

    if(motImg) {
        QQmlEngine *engine = qmlEngine(motImg);
        if (engine) {
            qCDebug(LOG_PLUGINS_WELLEIO) << "SLS image provider removed";
            engine->removeImageProvider(QLatin1String("SLS"));
        }
    }

    if(m_guiHelper){
        disconnect(m_guiHelper, &CGUIHelper::motChanged, this, &WelleIoPlugin::handleMotChanged);
        disconnect(m_guiHelper, &CGUIHelper::motReseted, this, &WelleIoPlugin::handleMotReseted);
        delete m_guiHelper;
    }

    if(m_radioController){
        delete m_radioController;
    }

    disconnect(&m_settings, &QQmlPropertyMap::valueChanged, this, &WelleIoPlugin::settingsChanged);
}

CGUIHelper *WelleIoPlugin::guiHelper(){
    return m_guiHelper;
}

CRadioController *WelleIoPlugin::radioController(){
    return m_radioController;
}

QObject *WelleIoPlugin::getContextProperty(){
    return this;
}

QQuickImageProvider *WelleIoPlugin::getImageProvider() {
    return m_guiHelper->motImageProvider;
}
