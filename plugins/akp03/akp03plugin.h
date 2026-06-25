#ifndef AKP03PLUGIN_H
#define AKP03PLUGIN_H

#include <QObject>
#include <QStringList>
#include <QByteArray>
#include <QTimer>
#include <QCoreApplication>
#include <plugininterface.h>

#include <QImage>
#include <QColor>

#include "akp03device.h"

class AKP03Plugin : public QObject, PluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.aselafernando.headunit.akp03" FILE "config.json")
    Q_INTERFACES(PluginInterface)

public:
    explicit AKP03Plugin(QObject *parent = nullptr);
    ~AKP03Plugin();

    void init() override;
    QObject *getContextProperty() override;
    void actionMessage(QString id, QVariant message) override;

    Q_INVOKABLE void onSettingsPageDestroyed() override;

public slots:
    void eventMessage(QString id, QVariant message) override;

signals:
    void message(QString id, QVariant message);
    void action(QString id, QVariant message);

private slots:
    void settingsChanged(const QString &key, const QVariant &value);
    void handleButtonPressed(int k);
    void handleButtonReleased(int k);
    void handleEncoderPressed(int e);
    void handleEncoderReleased(int e);
    void handleEncoderTurned(int e, int d);
private:
    Akp03Device dev;
    QString pluginName;
};

#endif // AKP03PLUGIN_H
