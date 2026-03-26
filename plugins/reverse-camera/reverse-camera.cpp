#include <QQuickWindow>
#include <QRunnable>
#include <QQuickView>
#include <QDebug>
#include <QLoggingCategory>

#include "reverse-camera.h"

Q_LOGGING_CATEGORY(LOG_PLUGINS_REVERSECAMERA, "plugins.reverse-camera")

class SetPlaying : public QRunnable
{
public:
  SetPlaying(GstElement *);
  ~SetPlaying();

  void run ();

private:
  GstElement * pipeline_;
};

SetPlaying::SetPlaying (GstElement * pipeline)
{
  this->pipeline_ = pipeline ? static_cast<GstElement *> (gst_object_ref (pipeline)) : NULL;
}

SetPlaying::~SetPlaying ()
{
  if (this->pipeline_)
    gst_object_unref (this->pipeline_);
}

void
SetPlaying::run ()
{
  if (this->pipeline_)
    gst_element_set_state (this->pipeline_, GST_STATE_PLAYING);
}

ReversingCamera::ReversingCamera(QObject *parent)
    : QObject (parent)
{
    //m_pluginSettings.eventListeners = QStringList();
    //m_pluginSettings.events = QStringList();
}

ReversingCamera::~ReversingCamera() {
    destroyPipeline();
    gst_deinit ();
}

QObject *ReversingCamera::getContextProperty(){
    return this;
}

void ReversingCamera::videoItemLoaded(QQuickItem *vi) {
    videoItem = vi;
    if(videoItem) {
        g_object_set(sink, "widget", videoItem, NULL);
        rootObject = videoItem->window();
        if(rootObject) {
            qCDebug(LOG_PLUGINS_REVERSECAMERA) << "Root Object Found";
            rootObject->scheduleRenderJob (new SetPlaying (pipeline),
                QQuickWindow::BeforeSynchronizingStage);
        }
    }
}

void ReversingCamera::eventMessage(__attribute__((unused)) QString id, __attribute__((unused)) QVariant message) {
}

void ReversingCamera::actionMessage(__attribute__((unused)) QString id, __attribute__((unused)) QVariant message) {
}

void ReversingCamera::settingsChanged(const QString &key, const QVariant &) {
    if (key == "gs_src") {
         qCDebug(LOG_PLUGINS_REVERSECAMERA) << "gs_src";
    } else if(key == "gs_pipeline") {
         qCDebug(LOG_PLUGINS_REVERSECAMERA) << "gs_pipeline";
    }
}

void ReversingCamera::init() {
    if (!gst_is_initialized()){
        gst_init(NULL, NULL);
    }

    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);
    setupPipeline();
}

void ReversingCamera::destroyPipeline() {
   qCDebug(LOG_PLUGINS_REVERSECAMERA) << "destroyPipeline()";

    if(pipeline) {
        gst_element_set_state (pipeline, GST_STATE_NULL);
        gst_object_unref (pipeline);
        pipeline = nullptr;
    }
    if(processPipeline) {
        gst_object_unref (processPipeline);
        processPipeline = nullptr;
    }
    if(src) {
        gst_object_unref (src);
        src = nullptr;
    }
    if(glupload) {
        gst_object_unref (glupload);
        processPipeline = nullptr;
    }
    if(glcolorconvert) {
        gst_object_unref (processPipeline);
        processPipeline = nullptr;
    }
    if(sink) {
        gst_object_unref (sink);
        sink = nullptr;
    }
}

void ReversingCamera::setupPipeline() {
    GError* error = nullptr;
    //rtspsrc location=rtsp://10.20.1.34:554/axis-media/media.amp?videocodec=h264 latency=100 buffer-mode=auto ! rtph264depay ! h264parse ! capssetter caps=\"video/x-h264,colorimetry=bt709\" ! avdec_h264
    pipeline = gst_pipeline_new (NULL);

    QString gs_src = m_settings["gs_src"].toString();
    QString gs_pipeline = m_settings["gs_pipeline"].toString();

    qCDebug(LOG_PLUGINS_REVERSECAMERA) << "GStreamer Src:" << gs_src;
    qCDebug(LOG_PLUGINS_REVERSECAMERA) << "GStreamer Pipeline:" << gs_pipeline;

    if(gs_src != "") {
        src = gst_parse_bin_from_description_full (gs_src.toUtf8().constData(), TRUE, NULL, GST_PARSE_FLAG_NO_SINGLE_ELEMENT_BINS, &error);
        //src = gst_parse_bin_from_description_full ("rtspsrc location=rtsp://10.20.1.34:554/axis-media/media.amp?videocodec=h264 latency=100 buffer-mode=auto", TRUE, NULL, GST_PARSE_FLAG_NO_SINGLE_ELEMENT_BINS, &error);
        //src = gst_parse_bin_from_description_full ("videotestsrc", TRUE, NULL, GST_PARSE_FLAG_NO_SINGLE_ELEMENT_BINS, &error);
        if (error) {
            qCDebug(LOG_PLUGINS_REVERSECAMERA) << error->message;
            g_error_free(error);
            destroyPipeline();
            return;
        }
        g_assert (src);
    }

    processPipeline = gst_parse_bin_from_description (gs_pipeline.toUtf8().constData(), TRUE, &error);
    //processPipeline = gst_parse_bin_from_description ("rtph264depay ! h264parse ! capssetter caps=\"video/x-h264,colorimetry=bt709\" ! avdec_h264", TRUE, &error);
    //processPipeline = gst_parse_bin_from_description ("capsfilter caps=\"video/x-raw,format=YV12\"", TRUE, &error);
    if (error) {
        qCDebug(LOG_PLUGINS_REVERSECAMERA) << error->message;
        g_error_free(error);
        destroyPipeline();
        return;
    }

    g_assert (processPipeline);

    if(!processPipeline) {
        qCDebug(LOG_PLUGINS_REVERSECAMERA) << "gstreamer process pipeline error in configuration!";
        destroyPipeline();
        return;
    }

    sink = gst_element_factory_make ("qml6glsink", NULL);

    /*glupload = gst_element_factory_make ("glupload", NULL);
    glcolorconvert = gst_element_factory_make ("glcolorconvert", NULL);

    if(!glupload) {
        qCDebug(LOG_PLUGINS_REVERSECAMERA) << "gstreamer glupload missing";
        destroyPipeline();
        return;
    }

    if(!glcolorconvert) {
        qCDebug(LOG_PLUGINS_REVERSECAMERA) << "gstreamer glcolorconvert missing";
        destroyPipeline();
        return;
    }
    */
    if(!sink) {
        qCDebug(LOG_PLUGINS_REVERSECAMERA) << "gstreamer qml6glsink missing?";
        destroyPipeline();
        return;
    }

    if(src) {
        qCDebug(LOG_PLUGINS_REVERSECAMERA) << "Linking with delayed pads";
        gst_bin_add_many(GST_BIN(pipeline), src, processPipeline, sink, NULL);

        g_signal_connect(src, "pad-added", G_CALLBACK(+[](__attribute__((unused)) GstElement *src, GstPad *pad, GstElement *sink) {
                qCDebug(LOG_PLUGINS_REVERSECAMERA) << "GStreamer src pad-added";
                GstPad *sinkpad = gst_element_get_static_pad(sink, "sink");
                gst_pad_link(pad, sinkpad);
                gst_object_unref(sinkpad);
        }), processPipeline);

    } else {
        qCDebug(LOG_PLUGINS_REVERSECAMERA) << "Linking with present pads";
        gst_bin_add_many(GST_BIN(pipeline), processPipeline, sink, NULL);
    }

    gst_element_link_many(processPipeline, sink, NULL);

    qCDebug(LOG_PLUGINS_REVERSECAMERA) << "Pipeline Made";
}

void ReversingCamera::onSettingsPageDestroyed(){
    //setupPipeline();
    qCDebug(LOG_PLUGINS_REVERSECAMERA) << "Settings Destroyed";
    destroyPipeline();
    setupPipeline();
}
