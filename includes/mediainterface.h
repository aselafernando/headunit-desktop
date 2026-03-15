#ifndef MEDIAINTERFACE_H
#define MEDIAINTERFACE_H

#include <QObject>

class MediaInterface
{
    struct MediaInterfaceSettings {
        bool mediaStream = false;
        bool voiceStream = false;
    };
public:
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void prevTrack() = 0;
    virtual void nextTrack() = 0;
    virtual void setMediaVolume(__attribute__((unused)) uint8_t volume) {};
    virtual void setVoiceVolume(__attribute__((unused)) uint8_t volume) {};
    virtual void mediaPositionChanged(__attribute__((unused)) quint32 position) {};
    virtual void trackChanged(__attribute__((unused)) QVariantMap track) {};
    virtual void playbackStarted() = 0;

    MediaInterfaceSettings getSettings() {
        return m_interfaceSettings;
    }
protected:
    MediaInterfaceSettings m_interfaceSettings;
};

#endif // MEDIAINTERFACE_H
