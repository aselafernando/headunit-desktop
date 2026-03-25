#ifndef REVERSECAMERAPLUGIN_H
#define REVERSECAMERAPLUGIN_H

#include <QObject>
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

signals:
    void message(QString id, QVariant message);
    void action(QString id, QVariant message);

public slots:
    void onSettingsPageDestroyed();

    void eventMessage(QString id, QVariant message) override;
    void actionMessage(QString id, QVariant message);

    void videoItemLoaded(QQuickItem *videoItem);

private slots :
    void settingsChanged(const QString &key, const QVariant &);

private:
    //GstElement *rtph264depay = nullptr;
    //GstElement *h264parse = nullptr;
    //GstElement *capssetter = nullptr;
    //GstElement *h264dec = nullptr;
    //GstElement *capsfilter = nullptr;

    GstElement *pipeline = nullptr;

    GstElement *src = nullptr;
    GstElement *processPipeline = nullptr;

    GstElement *glupload = nullptr;
    GstElement *glcolorconvert = nullptr;
    GstElement *sink = nullptr;

    QQuickItem *videoItem = nullptr;
    QQuickWindow *rootObject = nullptr;

    void setupPipeline();
    void destroyPipeline();
};


#endif // REVERSECAMERAPLUGIN_H
