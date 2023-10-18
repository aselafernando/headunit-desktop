#ifndef J2534PLUGIN_H
#define J2534PLUGIN_H

#include <QObject>
#include <QStringList>
#include <QDebug>
#include <QByteArray>
#include <QTimer>
#include <QCoreApplication>
#include <QThread>
#include <plugininterface.h>

#include "J2534_V0404.h"

class J2534Plugin : public QObject, PluginInterface {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.aselafernando.headunit.j2534" FILE "config.json")
    Q_INTERFACES(PluginInterface)

    Q_PROPERTY(double iat      MEMBER m_iat      NOTIFY iatUpdated)
    Q_PROPERTY(double maf      MEMBER m_maf      NOTIFY mafUpdated)
    Q_PROPERTY(double stftb1   MEMBER m_stftb1   NOTIFY stftb1Updated)
    Q_PROPERTY(double stftb2   MEMBER m_stftb2   NOTIFY stftb2Updated)
    Q_PROPERTY(double ect      MEMBER m_ect      NOTIFY ectUpdated)
    Q_PROPERTY(int    vss      MEMBER m_vss      NOTIFY vssUpdated)
    Q_PROPERTY(double rpm      MEMBER m_rpm      NOTIFY rpmUpdated)
    Q_PROPERTY(double accPedal MEMBER m_accPedal NOTIFY accPedalUpdated)
    Q_PROPERTY(int    gear     MEMBER m_gear     NOTIFY gearUpdated)

    //Q_PROPERTY(QVariantMap ports READ getPorts NOTIFY portsUpdated)
    //Q_PROPERTY(bool connected MEMBER m_connected NOTIFY connectedUpdated)

    QThread workerThread;

public:
    explicit J2534Plugin(QObject *parent = nullptr);
    ~J2534Plugin();

    void init() override;
    void deinit();
    QObject *getContextProperty() override;
    void PrintString(char * message);

public slots:
    void eventMessage(QString id, QVariant message) override;

    void handleIAT(const double&);
    void handleMAF(const double&);
    void handleSTFTB1(const double&);
    void handleSTFTB2(const double&);
    void handleECT(const double&);
    void handleVSS(const int&);
    void handleRPM(const double&);
    void handleACCPEDAL(const double&);
    void handleGEAR(const int&);

signals:
    void message(QString id, QVariant message);
    void action(QString id, QVariant message);

    void operate();

    void iatUpdated();
    void mafUpdated();
    void stftb1Updated();
    void stftb2Updated();
    void ectUpdated();
    void vssUpdated();
    void rpmUpdated();
    void accPedalUpdated();
    void gearUpdated();

private slots:
    void settingsChanged(const QString &key, const QVariant &value);

private:
    double m_iat = 0;
    double m_maf = 0;
    double m_stftb1 = 0;
    double m_stftb2 = 0;
    double m_ect = 0;
    int m_vss = 0;
    double m_rpm = 0;
    double m_accPedal = 0;
    int m_gear = 0;
    int m_reversePageIndex = 3;

    void startWorker();
    void stopWorker();

};

class J2534Worker : public QObject {
    Q_OBJECT

private:
    uint32_t deviceID = 0;
    uint32_t channelID = 0;
    uint32_t msgID[3];

    bool startMsgFilters();
    bool setConfig();
    bool fastInit(uint8_t address);
    void keepReading();
    double requestPID(uint8_t address, uint8_t mode, uint8_t pid);
    void getData();

public slots:
    void j2534Connect();

signals:
    void iat(const double& result);
    void maf(const double& result);
    void stftb1(const double& result);
    void stftb2(const double& result);
    void ect(const double& result);
    void accPedal(const double& result);
    void rpm(const double& result);
    void vss(const int& result);
    void gear(const int& result);
};


#endif // J2534PLUGIN_H
