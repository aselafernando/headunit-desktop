#include "gpsdplugin.h"
#define _USE_MATH_DEFINES
#include <math.h>

double fences[1][10][2] = {{{17.529188, 78.361845},
                              {17.529840, 78.361919},
                              {17.529934, 78.362197},
                              {17.530624, 78.362507},
                              {17.530832, 78.363043},
                              {17.530999, 78.363451},
                              {17.530924, 78.363976},
                              {17.529248, 78.363288},
                              {17.529101, 78.362858},
                              {17.529040, 78.362489}}};


GPSDPlugin::GPSDPlugin(QObject *parent) : QObject (parent)
{
    //m_pluginSettings.events = QStringList() << "MediaInput::position";
}

void GPSDPlugin::init() {
    startWorker();
}

void GPSDPlugin::handleMode(const int& result) {
    this->m_mode = result;
    emit modeUpdated();
}

void GPSDPlugin::handleLocation(const double& lat, const double& lon, const bool& inFence) {
    this->m_latitude = lat;
    this->m_longitude = lon;
    this->m_inFence = inFence;
    emit latitudeUpdated();
    emit longitudeUpdated();
    emit inFenceUpdated();
}

QObject *GPSDPlugin::getContextProperty(){
    return this;
}

void GPSDPlugin::eventMessage(QString id, QVariant message) {
}

GPSDPlugin::~GPSDPlugin() {
    stopWorker();
}

void GPSDPlugin::stopWorker() {
    workerThread.quit();
    workerThread.requestInterruption();
    workerThread.wait();
}

void GPSDPlugin::startWorker() {
    m_host  = m_settings.value("host").toString();
    m_port  = m_settings.value("port").toUInt();
    m_fence = m_settings.value("fence").toInt();

    if (m_fence >= 0) {
        //Get Fence Points
        int fenceSize = sizeof(fences[m_fence])/sizeof(fences[m_fence][0]);
        char buffer[11];
        for(int i = 0; i < fenceSize; i++) {
            snprintf(buffer, 11, "fence0pt%d", i);
            QString pt = m_settings.value(buffer).toString();
            QStringList pieces = pt.split(",");
            if(pieces.length() == 2) {
                fences[0][i][0] = pieces.value(0).toDouble();
                fences[0][i][1] = pieces.value(1).toDouble();
            } else {
                fences[0][i][0] = 0;
                fences[0][i][1] = 0;
            }
        }
    }

    //Run the data colleciton in another thread
    GPSDWorker* worker = new GPSDWorker;
    worker->moveToThread(&workerThread);
    connect(&workerThread, &QThread::finished, worker, &QObject::deleteLater);
    connect(this, &GPSDPlugin::operate, worker, &GPSDWorker::connect);
    connect(worker, &GPSDWorker::mode, this, &GPSDPlugin::handleMode);
    connect(worker, &GPSDWorker::location, this, &GPSDPlugin::handleLocation);
    workerThread.start();
    operate(m_host, m_port, m_fence);
    qDebug() << "GPSD: Started worker thread: " << workerThread.isRunning();
}

void GPSDPlugin::settingsChanged(const QString &key, const QVariant &){
    stopWorker();
    startWorker();
    if(key == "host"){
    }
    else if (key == "port") {
    }
}

void GPSDPlugin::PrintString(const char *message) {
    qDebug() << "GPSD DEBUG : " << message;
}

bool GPSDWorker::pointInPolygon(double lat, double lon) {
    if (m_fence < 0) return false;

    int fenceSize = sizeof(fences[m_fence])/sizeof(fences[m_fence][0]);
    double vectors[fenceSize][2];

    for(int i = 0; i < fenceSize; i++){
        if(fences[m_fence][i][0] == 0 || fences[m_fence][i][1] == 0) {
            fenceSize = i;
            break;
        }
        vectors[i][0] = fences[m_fence][i][0] - lat;
        vectors[i][1] = fences[m_fence][i][1] - lon;
    }

    double angle = 0;
    double num, den;

    for(int i = 0; i < fenceSize; i++){
        num = (vectors[i%fenceSize][0])*(vectors[(i+1)%fenceSize][0])+ (vectors[i%fenceSize][1])*(vectors[(i+1)%fenceSize][1]);
        den = (sqrt(pow(vectors[i%fenceSize][0],2) + pow(vectors[i%fenceSize][1],2)))*(sqrt(pow(vectors[(i+1)%fenceSize][0],2) + pow(vectors[(i+1)%fenceSize][1],2)));
        angle = angle + (180*acos(num/den)/M_PI);
    }

    return (angle > 355 && angle < 365);
}

void GPSDWorker::procData(struct gps_data_t * gps) {
    if (gps->set & MODE_SET) {
        emit mode(gps->fix.mode);
        if(gps->fix.mode > MODE_2D) {
            if(gps->set & LATLON_SET)
                emit location(gps->fix.latitude, gps->fix.longitude, pointInPolygon(gps->fix.latitude, gps->fix.longitude));
        }
    }
}

void GPSDWorker::getData(std::string host, uint32_t port) {
    gpsmm gps_rec(host.c_str(), DEFAULT_GPSD_PORT);

    if(port > 0 && port <= 65535) {
        char buffer[6];
        snprintf(buffer, 6, "%d", port);
        gpsmm gps_rec(host.c_str(), buffer);
    }

    if (gps_rec.stream(WATCH_ENABLE|WATCH_JSON) == NULL) {
        qDebug() << "GPSD: No GPSD running.\n";
        QThread::sleep(1);
    }

    while(stopClient == false && QThread::currentThread()->isInterruptionRequested() == false) {
        struct gps_data_t* newdata;

        if (!gps_rec.waiting(5000000))
            continue;

        if ((newdata = gps_rec.read()) == NULL) {
            qDebug() << "GPSD: Read error\n";
            QThread::sleep(1);
        } else {
            procData(newdata);
        }
    }
}

void GPSDWorker::connect(QString host, quint32 port, int fence) {
    stopClient = false;
    this->m_fence = fence;
    getData(host.toStdString(), port);
}

void GPSDWorker::disconnect() {
    stopClient = true;
}

GPSDWorker::~GPSDWorker() {
    stopClient = true;
}
