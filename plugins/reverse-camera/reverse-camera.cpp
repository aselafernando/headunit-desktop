#include <QRunnable>
#include <QQuickView>
#include <QDebug>
#include <QLoggingCategory>

#include "reverse-camera.h"

Q_LOGGING_CATEGORY(LOG_PLUGINS_REVERSECAMERA, "plugins.reverse-camera")

bool has_always_source_pad(GstElement *element) {
    if (!element) return false;

    GstElementClass *klass = GST_ELEMENT_GET_CLASS(element);
    const GList *templates = gst_element_class_get_pad_template_list(klass);

    for (const GList *l = templates; l != nullptr; l = l->next) {
        GstPadTemplate *templ = (GstPadTemplate *)l->data;

        // Check if it's a source pad and if its presence is 'ALWAYS'
        if (templ->direction == GST_PAD_SRC &&
            templ->presence == GST_PAD_ALWAYS) {
            return true;
        }
    }

    return false;
}

bool remove_element(GstElement *element) {
    if (!element) return false;

    // 1. Get the parent bin/pipeline
    GstElement *parent = GST_ELEMENT_CAST(gst_element_get_parent(element));
    
    if (!parent || !GST_IS_BIN(parent)) {
        // Element doesn't have a parent bin to be removed from
        if (parent) gst_object_unref(parent);
        return false;
    }

    // 2. Stop the element before removal
    gst_element_set_state(element, GST_STATE_NULL);

    // 3. Remove from the bin
    bool success = gst_bin_remove(GST_BIN(parent), element);

    // 4. Release the parent reference returned by gst_element_get_parent
    gst_object_unref(parent);

    return success;
}

bool remove_element_by_name(GstElement *pipeline, const char *name) {
    if (!pipeline || !name) return false;

    // 1. Find the element in the pipeline
    // Note: gst_bin_get_by_name returns a reference that we must unref later
    GstElement *element = gst_bin_get_by_name(GST_BIN(pipeline), name);
    
    if (!element) {
        g_printerr("Element '%s' not found in pipeline.\n", name);
        return false;
    }

    // 2. Set element state to NULL to stop data flow and release resources
    // This is required before removal to avoid GStreamer-CRITICAL errors
    gst_element_set_state(element, GST_STATE_NULL);

    // 3. Remove from the bin/pipeline
    // This unparents the element and handles unlinking automatically
    bool success = gst_bin_remove(GST_BIN(pipeline), element);

    // 4. Release the reference returned by gst_bin_get_by_name
    gst_object_unref(element);

    return success;
}

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
  qDebug(LOG_PLUGINS_REVERSECAMERA) << "SetPlaying::run()";
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
    qCDebug(LOG_PLUGINS_REVERSECAMERA) << "Video item loaded";
    videoItem = vi;
    if(videoItem) {
        GstElement* sink = gst_bin_get_by_name(GST_BIN(totalPipeline), "vid_sink");
        if(!sink) {
            qCDebug(LOG_PLUGINS_REVERSECAMERA) << "Could not find vid_sink in pipeline!";
        } else {
            g_object_set(sink, "widget", videoItem, NULL);
            QQuickWindow *rootWindow = videoItem->window();
            if(rootWindow) {
                rootWindow->scheduleRenderJob (new SetPlaying (totalPipeline),
                    QQuickWindow::BeforeSynchronizingStage);
            }
        }
    }
}

void ReversingCamera::eventMessage(__attribute__((unused)) QString id, __attribute__((unused)) QVariant message) {
}

void ReversingCamera::actionMessage(__attribute__((unused)) QString id, __attribute__((unused)) QVariant message) {
}

void ReversingCamera::settingsChanged(const QString &key, const QVariant &) {
    if(key == "gs_pipeline") {
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

    if(videoItem) {
       GstElement* sink = gst_bin_get_by_name(GST_BIN(totalPipeline), "vid_sink");
       g_object_set(sink, "widget", nullptr, NULL);
       gst_object_unref(sink);
    }

    if(totalPipeline) {
        if(gst_element_set_state (totalPipeline, GST_STATE_NULL) == GST_STATE_CHANGE_FAILURE) {
            qCDebug(LOG_PLUGINS_REVERSECAMERA) << "Failed to stop pipeline";
            return;
        }
    }

    if(totalPipeline) {
        gst_object_unref(totalPipeline);
        totalPipeline = nullptr;
        qCDebug(LOG_PLUGINS_REVERSECAMERA) << "Pipeline destroyed";
    }
}

void ReversingCamera::setupPipeline() {
    GError* error = nullptr;

    if(totalPipeline) {
        qCDebug(LOG_PLUGINS_REVERSECAMERA) << "Pipeline still there!!!";
        return;
    }

    totalPipeline = gst_pipeline_new (NULL);

    QString gs_pipeline = m_settings["gs_pipeline"].toString();

    qCDebug(LOG_PLUGINS_REVERSECAMERA) << "GStreamer Pipeline:" << gs_pipeline;

    GstElement* gsPipeline = gst_parse_bin_from_description (gs_pipeline.toUtf8().constData(), TRUE, &error);
    //GstElement* gsPipeline = gst_parse_bin_from_description ("rtspsrc location=rtsp://10.20.1.34:554/axis-media/media.amp?videocodec=h264 latency=10 buffer-mode=auto name=vid_src ! rtph264depay ! h264parse ! capssetter caps=\"video/x-h264,colorimetry=bt709\" ! avdec_h264", TRUE, &error);
    //GstElement* gsPipeline = gst_parse_bin_from_description ("videotestsrc name=vid_src ! capsfilter caps=\"video/x-raw,format=YV12\"", TRUE, &error);
    if (error) {
        qCDebug(LOG_PLUGINS_REVERSECAMERA) << error->message;
        g_error_free(error);
        destroyPipeline();
        return;
    }

    if(!gsPipeline) {
        qCDebug(LOG_PLUGINS_REVERSECAMERA) << "gstreamer process pipeline error in configuration!";
        destroyPipeline();
        return;
    }

    GstElement* sink = gst_element_factory_make ("qml6glsink", "vid_sink");
    GstElement* glupload = gst_element_factory_make ("glupload", NULL);
    GstElement* glcolorconvert = gst_element_factory_make ("glcolorconvert", NULL);

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

    if(!sink) {
        qCDebug(LOG_PLUGINS_REVERSECAMERA) << "gstreamer qml6glsink missing?";
        destroyPipeline();
        return;
    }

    GstElement* src = gst_bin_get_by_name(GST_BIN(gsPipeline), "vid_src");

    if(!src) {
        qCDebug(LOG_PLUGINS_REVERSECAMERA) << "Could not find vid_src in pipeline! You are missing an element with name=vid_src";
        destroyPipeline();
        return;
    }

    if(!has_always_source_pad(src)) {
        qCDebug(LOG_PLUGINS_REVERSECAMERA) << "Linking with delayed pads";
        //remove from the processPipeline to manually link
        remove_element(src);

        gst_bin_add_many(GST_BIN(totalPipeline), src, gsPipeline, glupload, glcolorconvert, sink, NULL);

        g_signal_connect(src, "pad-added", G_CALLBACK(+[](__attribute__((unused)) GstElement *src, GstPad *pad, GstElement *sink) {
                qCDebug(LOG_PLUGINS_REVERSECAMERA) << "GStreamer src pad-added";
                GstPad *sinkpad = gst_element_get_static_pad(sink, "sink");
                gst_pad_link(pad, sinkpad);
                gst_object_unref(sinkpad);
        }), gsPipeline);

    } else {
        qCDebug(LOG_PLUGINS_REVERSECAMERA) << "Linking with present pads";
        gst_bin_add_many(GST_BIN(totalPipeline), gsPipeline, glupload, glcolorconvert, sink, NULL);
    }

    gst_element_link_many(gsPipeline, glupload, glcolorconvert, sink, NULL);

    qCDebug(LOG_PLUGINS_REVERSECAMERA) << "Pipeline Made";
}

void ReversingCamera::onSettingsPageDestroyed(){
    qCDebug(LOG_PLUGINS_REVERSECAMERA) << "Settings Destroyed";
    destroyPipeline();
    setupPipeline();
    videoItemLoaded(this->videoItem);
}
