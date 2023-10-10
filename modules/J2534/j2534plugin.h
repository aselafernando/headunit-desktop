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

    Q_PROPERTY(double iat READ iat NOTIFY iatChanged)
    Q_PROPERTY(double maf READ maf NOTIFY mafChanged)
    Q_PROPERTY(double stftb1 READ stftb1 NOTIFY stftb1Changed)
    Q_PROPERTY(double stftb2 READ stftb2 NOTIFY stftb2Changed)
    Q_PROPERTY(double ect READ ect NOTIFY ectChanged)
    Q_PROPERTY(double vss READ vss NOTIFY vssChanged)
    Q_PROPERTY(double rpm READ rpm NOTIFY rpmChanged)
    Q_PROPERTY(double accPedal READ accPedal NOTIFY accPedalChanged)
    Q_PROPERTY(int gear READ gear NOTIFY gearChanged)

    //Q_PROPERTY(QVariantMap ports READ getPorts NOTIFY portsUpdated)
    //Q_PROPERTY(bool connected MEMBER m_connected NOTIFY connectedUpdated)

    QThread workerThread;

public:
    explicit J2534Plugin(QObject *parent = nullptr);
    ~J2534Plugin();

    void init() override;
    void deinit();
    QObject *getContextProperty() override;
    void PrintString(char * message, int length);
    double iat();
    double maf();
    double stftb1();
    double stftb2();
    double ect();
    double vss();
    double rpm();
    double accPedal();
    int gear();

public slots:
    void eventMessage(QString id, QVariant message) override;
    void handleIAT(const double&);
    void handleMAF(const double&);
    void handleSTFTB1(const double&);
    void handleSTFTB2(const double&);
    void handleECT(const double&);
    void handleVSS(const double&);
    void handleRPM(const double&);
    void handleACCPEDAL(const double&);
    void handleGEAR(const int&);

signals:
    void message(QString id, QVariant message);
    void action(QString id, QVariant message);
    void operate();
    void iatChanged();
    void mafChanged();
    void stftb1Changed();
    void stftb2Changed();
    void ectChanged();
    void vssChanged();
    void rpmChanged();
    void accPedalChanged();
    void gearChanged();

private slots:
    void settingsChanged(const QString &key, const QVariant &value);

private:
    double iatValue = 0;
    double mafValue = 0;
    double stftb1Value = 0;
    double stftb2Value = 0;
    double ectValue = 0;
    double vssValue = 0;
    double rpmValue = 0;
    double accPedalValue = 0;
    int gearValue = 0;
    int reversePageIndex = 3;

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
    void j2534Disconnect();

signals:
    void iat(const double& result);
    void maf(const double& result);
    void stftb1(const double& result);
    void stftb2(const double& result);
    void ect(const double& result);
    void accPedal(const double& result);
    void rpm(const double& result);
    void vss(const double& result);
    void gear(const int& result);
};


#endif // J2534PLUGIN_H
