#ifndef REVERSECAMERAPLUGIN_H
#define REVERSECAMERAPLUGIN_H

#include <QObject>
#include <QQuickWindow>
#include <QQuickItem>
#include <gst/gst.h>

#include <plugininterface.h>

class ReversingCamera : public QObject, PluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.aselafernando.headunit.reversing-camera" FILE "config.json")
    Q_INTERFACES(PluginInterface)

public:
    explicit ReversingCamera(QObject *parent = nullptr);
    ~ReversingCamera();
    QObject *getContextProperty() override;

    void init() override;
    Q_INVOKABLE void onSettingsPageDestroyed() override;

signals:
    void message(QString id, QVariant message);
    void action(QString id, QVariant message);

public slots:
    void eventMessage(QString id, QVariant message) override;
    void actionMessage(QString id, QVariant message);

    void videoItemLoaded(QQuickItem *videoItem);

private slots :
    void settingsChanged(const QString &key, const QVariant &);

private:
    //Pipeline
    GstElement *totalPipeline = nullptr;

    //QML Output
    QQuickItem *videoItem = nullptr;

    void setupPipeline();
    void destroyPipeline();
};


#endif // REVERSECAMERAPLUGIN_H
