//#include <math.h>

#include <QImage>
#include <QColor>
#include <QTimer>

#include <QDebug>
#include <QLoggingCategory>

#include "akp03device.h"
#include "akp03plugin.h"

Q_LOGGING_CATEGORY(LOG_PLUGINS_AKP03, "plugins.akp03")

AKP03Plugin::AKP03Plugin(QObject *parent) : QObject (parent)
{
    m_pluginSettings.eventListeners = QStringList() << "MediaInput::position" << "MediaInput::track";
}

void AKP03Plugin::handleButtonPressed(int k) {
    qCDebug(LOG_PLUGINS_AKP03) << "Button" << k << "PRESSED";
    switch(k) {
        case 6:
            emit message("MediaInput", "Previous");
            break;
        case 8:
            emit message("MediaInput", "Next");
            break;
    }

}

void AKP03Plugin::handleButtonReleased(int k) {
    qCDebug(LOG_PLUGINS_AKP03) << "Button" << k << "Released";
}

void AKP03Plugin::handleEncoderPressed(int e) {
    qCDebug(LOG_PLUGINS_AKP03) << "Encoder" << e << "PRESSED";
}

void AKP03Plugin::handleEncoderReleased(int e) {
    qCDebug(LOG_PLUGINS_AKP03) << "Encoder" << e << "Released";
}

void AKP03Plugin::handleEncoderTurned(int e, int d) {
    qCDebug(LOG_PLUGINS_AKP03) << "Encoder" << e << "turned" << (d > 0 ? "CW" : "CCW") << "(" << d << ")";
    switch(e) {
        case 0:
            if(d > 0) {
                emit action("GUI::ChangePageNext", 0);
            } else {
                emit action("GUI::ChangePagePrev", 0);
            }
            break;
        case 1:
            if(d > 0) {
                emit action("VolumeControl::VolumeUp", 0);
            } else {
                emit action("VolumeControl::VolumeDown", 0);
            }
            break;
    }
}

void AKP03Plugin::init() {
    QString err;
    if (!dev.initialize(&err)) {
        qCritical().noquote() << err;
        return;
    }
    qCDebug(LOG_PLUGINS_AKP03) << "Device ready.";

    QObject::connect(&dev, &Akp03Device::buttonPressed, this, &AKP03Plugin::handleButtonPressed);
    QObject::connect(&dev, &Akp03Device::buttonReleased, this, &AKP03Plugin::handleButtonReleased);
    QObject::connect(&dev, &Akp03Device::encoderPressed, this, &AKP03Plugin::handleEncoderPressed);
    QObject::connect(&dev, &Akp03Device::encoderReleased, this, &AKP03Plugin::handleEncoderReleased);
    QObject::connect(&dev, &Akp03Device::encoderTurned, this, &AKP03Plugin::handleEncoderTurned);

    QObject::connect(&dev, &Akp03Device::deviceDisconnected, [&]() {
        qCWarning(LOG_PLUGINS_AKP03) << "Device disconnected.";
    });
    QObject::connect(&dev, &Akp03Device::errorOccurred,
                     [](const QString& m) { qWarning().noquote() << "Error:" << m; });
    static const QColor palette[Akp03Device::LcdKeyCount] = {
        QColor(220, 40, 40),  QColor(220, 140, 30), QColor(210, 200, 40),
        QColor(40, 200, 90),  QColor(40, 130, 220), QColor(160, 70, 220),
    };
    for (int k = 0; k < Akp03Device::LcdKeyCount; k++) {
        QImage tile(60, 60, QImage::Format_RGB888);
        tile.fill(palette[k]);
        dev.sendImage(k, tile);
    }
    dev.startListening();
    qCDebug(LOG_PLUGINS_AKP03) << "Listening. Press buttons / turn encoders. Ctrl-C to quit.";
}

QObject *AKP03Plugin::getContextProperty(){
    return this;
}

void AKP03Plugin::onSettingsPageDestroyed() {
}

void AKP03Plugin::eventMessage(QString id, QVariant message) {
    //Track Name: NMTest Name
    //Track Time: TMHHMMSS
    //Track Num : TR000
    /*if (id == "MediaInput::position") {
        uint32_t seconds = (uint32_t)floor(message.toUInt() / 1000.0);
        uint32_t minutes = seconds / 60;
        uint32_t hours = minutes / 60;
        char timestamp[9];
        snprintf(timestamp, 9, "TM%02d%02d%02d", hours % 99, minutes % 60, seconds % 60);
        m_serialProtocol.sendMessage(8, timestamp);
    } else if (id == "MediaInput::track") {
        char buffer[6];
        QVariantMap track = message.toMap();
        snprintf(buffer, 6, "TR%03d", track["number"].toUInt() % 999);
        m_serialProtocol.sendMessage(5, buffer);
        if(m_text) {
            QString trackName = QString("NM%1 - %2").arg(track["artist"].toString()).arg(track["title"].toString());
            m_serialProtocol.sendMessage(trackName.length() > 254 ? 254 : trackName.length(), trackName.toLocal8Bit().data());
        }
    }*/
}

void AKP03Plugin::settingsChanged(const QString &key, const QVariant &){
    /*if(key == "serial_port"){
        serialDisconnect();
        serialConnect();
    }
    else if (key == "text") {

    }*/
}

//void AKP03Plugin::ButtonInputCommandCallback(k) {
//    QString cmd = "";
/*
    switch(btn) {
        case Button::PLAY:
            cmd = m_settings.value("PLAY").toString();
            if(cmd == "")
                emit message("MediaInput", "Play");
            PrintString("PLAY", 4);
            return;
        case Button::STOP:
            cmd = m_settings.value("STOP").toString();
            if(cmd == "")
                emit message("MediaInput", "Stop");
            PrintString("STOP", 4);
            return;
        case Button::NEXT_TRACK:
            cmd = m_settings.value("NEXT_TRACK").toString();
            if(cmd == "")
                emit message("MediaInput", "Next");
            PrintString("NEXT_TRACK", 10);
            return;
        case Button::PREV_TRACK:
            cmd = m_settings.value("PREV_TRACK").toString();
            if(cmd == "")
                emit message("MediaInput", "Previous");
            PrintString("PREV_TRACK", 10);
            return;
        case Button::NEXT_DISC:
            cmd = m_settings.value("NEXT_DISC").toString();
            PrintString("NEXT_DISC", 9);
            break;
        case Button::PREV_DISC:
            cmd = m_settings.value("PREV_DISC").toString();
            PrintString("PREV_DISC", 4);
            break;
        case Button::SCAN_ON:
            cmd = m_settings.value("SCAN_ON").toString();
            PrintString("SCAN_ON", 7);
            break;
        case Button::SCAN_OFF:
            cmd = m_settings.value("SCAN_OFF").toString();
            PrintString("SCAN_OFF", 8);
            break;
        case Button::REPEAT_ON:
            PrintString("REPEAT_ON", 9);
            cmd = m_settings.value("REPEAT_ON").toString();
            if(cmd == "")
                emit message("MediaInput", "RepeatOn");
            break;
        case Button::REPEAT_OFF:
            PrintString("REPEAT_OFF", 10);
            cmd = m_settings.value("REPEAT_OFF").toString();
            if(cmd == "")
                emit message("MediaInput", "RepeatOff");
            break;
        case Button::SHUFFLE_ON:
            PrintString("SHUFFLE_ON", 10);
            cmd = m_settings.value("SHUFFLE_ON").toString();
            if(cmd == "")
                emit message("MediaInput", "ShuffleOn");
            break;
        case Button::SHUFFLE_OFF:
            PrintString("SHUFFLE_OFF", 11);
            cmd = m_settings.value("SHUFFLE_OFF").toString();
            if(cmd == "")
                emit message("MediaInput", "ShuffleOff");
            break;
        case Button::FF_ON:
            cmd = m_settings.value("FF_ON").toString();
            PrintString("FF_ON", 5);
            break;
        case Button::FF_OFF:
            cmd = m_settings.value("FF_OFF").toString();
            PrintString("FF_OFF", 6);
            break;
        case Button::RW_ON:
            cmd = m_settings.value("RW_ON").toString();
            PrintString("RW_ON", 5);
            break;
        case Button::RW_OFF:
            cmd = m_settings.value("RW_OFF").toString();
            PrintString("RW_OFF", 6);
            break;
    }

    if (cmd != "") {
        emit action(cmd, 0);
        qCDebug(LOG_PLUGINS_AKP03) << "Calling Action: " << cmd;
    }*/
//}

/*void AKP03Plugin::PrintString(const char* message, int length) {
    qCDebug(LOG_PLUGINS_AKP03) << "" << QString::fromUtf8(message, length);
}*/
