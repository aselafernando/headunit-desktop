#include "headuniteventhandler.h"

#include <gst/app/gstappsink.h>
#include <gst/app/gstappsrc.h>
#include <gst/gst.h>
#include <gst/video/video.h>

#include <QBluetoothLocalDevice>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>

#include "headunit/includes/AndroidAuto.h"

HeadunitEventHandler::HeadunitEventHandler(QObject* parent) : QObject(parent) {
    connect(this, &HeadunitEventHandler::AudioFocusRequest, this, &HeadunitEventHandler::onAudioFocusRequest);
    connect(this, &HeadunitEventHandler::VideoFocusHappened, this, &HeadunitEventHandler::onVideoFocusHappened);
}

void HeadunitEventHandler::setHUThreadInterface(AndroidAuto::IHUAnyThreadInterface* huThreadInterface) {
    m_huThreadInterface = huThreadInterface;
}
void HeadunitEventHandler::setMediaDataHandler(HeadunitMediaDataHandlerInterface* mediaDataHandler) {
    m_mediaDataHandler = mediaDataHandler;
}

void HeadunitEventHandler::handleMicrophoneData(uint64_t timestamp, const unsigned char* bufferData, int bufferSize) {
    if (m_huThreadInterface == nullptr) {
        qWarning() << "Headunit thread interface not set!";
        return;
    }

    m_huThreadInterface->queueCommand([timestamp, bufferData, bufferSize](AndroidAuto::IHUConnectionThreadInterface& s) {
        int ret = s.sendEncodedMediaPacket(1, AndroidAuto::MicrophoneChannel, AndroidAuto::PROTOCOL_MESSAGE::MediaDataWithTimestamp, timestamp,
                                           bufferData, bufferSize);
        if (ret < 0) {
            qDebug("read_mic_data(): hu_aap_enc_send() failed with (%d)", ret);
        }
    });
}

int HeadunitEventHandler::MediaPacket(AndroidAuto::ServiceChannels chan, uint64_t timestamp, const byte* buf, int len) {
    Headunit::Pipeline pipeline = Headunit::mediaChannelToPipeline(chan);
    if (pipeline == Headunit::InvalidPipeline) {
        qDebug() << "Unknown channel : " << chan;
        return -1;
    }
    m_mediaDataHandler->handleMediaData(pipeline, timestamp, buf, len);
    return 0;
}
int HeadunitEventHandler::MediaStart(AndroidAuto::ServiceChannels chan) {
    Headunit::Pipeline pipeline = Headunit::mediaChannelToPipeline(chan);
    if (pipeline == Headunit::InvalidPipeline) {
        qDebug() << "Unknown channel : " << chan;
        return -1;
    }
    m_mediaDataHandler->mediaStart(pipeline);
    return 0;
}

int HeadunitEventHandler::MediaStop(AndroidAuto::ServiceChannels chan) {
    Headunit::Pipeline pipeline = Headunit::mediaChannelToPipeline(chan);
    if (pipeline == Headunit::InvalidPipeline) {
        qDebug() << "Unknown channel : " << chan;
        return -1;
    }
    m_mediaDataHandler->mediaStop(pipeline);
    return 0;
}

void HeadunitEventHandler::Connected() {
    qDebug("Android Device connected, starting gstreamer");
    emit phoneConnected();
}

void HeadunitEventHandler::DisconnectionOrError() {
    qDebug("Android Device disconnected, pausing gstreamer");
    emit phoneDisconnected();
}

void HeadunitEventHandler::CustomizeOutputChannel(AndroidAuto::ServiceChannels /* unused */,
                                                  HU::ChannelDescriptor::OutputStreamChannel& /* unused */) {
}
void HeadunitEventHandler::MediaSetupComplete(AndroidAuto::ServiceChannels chan) {
    if (chan == AndroidAuto::VideoChannel) {
        emit VideoFocusHappened(true, true);
    }
}

void HeadunitEventHandler::VideoFocusRequest(AndroidAuto::ServiceChannels /* unused */, const HU::VideoFocusRequest& request) {
    emit VideoFocusHappened(request.mode() == HU::VIDEO_FOCUS_MODE_FOCUSED, false);
}

std::string HeadunitEventHandler::GetCarBluetoothAddress() {
    QList<QBluetoothHostInfo> localAdapters = QBluetoothLocalDevice::allDevices();
    if (localAdapters.size() > 0) {
        return localAdapters.at(0).address().toString().toStdString();
    }
    return std::string();
}

void HeadunitEventHandler::PhoneBluetoothReceived(std::string address) {
    emit bluetoothConnectionRequest(QString::fromStdString(address));
}

void HeadunitEventHandler::HandlePhoneStatus(AndroidAuto::IHUConnectionThreadInterface& stream, const HU::PhoneStatus& phoneStatus) {
}
void HeadunitEventHandler::HandleNaviStatus(AndroidAuto::IHUConnectionThreadInterface& stream, const HU::NAVMessagesStatus& request) {
}
void HeadunitEventHandler::HandleNaviTurn(AndroidAuto::IHUConnectionThreadInterface& stream, const HU::NAVTurnMessage& request) {
}
void HeadunitEventHandler::HandleNaviTurnDistance(AndroidAuto::IHUConnectionThreadInterface& stream, const HU::NAVDistanceMessage& request) {
}

uint64_t HeadunitEventHandler::get_cur_timestamp() {
    struct timespec tp;
    /* Fetch the time stamp */
    clock_gettime(CLOCK_REALTIME, &tp);

    return tp.tv_sec * 1000000 + tp.tv_nsec / 1000;
}

void HeadunitEventHandler::onAudioFocusRequest(AndroidAuto::ServiceChannels chan, const HU::AudioFocusRequest& request) {
    HU::AudioFocusResponse response;
    if (request.focus_type() == HU::AudioFocusRequest::AUDIO_FOCUS_RELEASE) {
        response.set_focus_type(HU::AudioFocusResponse::AUDIO_FOCUS_STATE_LOSS);
    } else {
        response.set_focus_type(HU::AudioFocusResponse::AUDIO_FOCUS_STATE_GAIN);
    }

    if (m_huThreadInterface == nullptr) {
        qWarning() << "Headunit thread interface not set!";
        return;
    }

    m_huThreadInterface->queueCommand([chan, response](AndroidAuto::IHUConnectionThreadInterface& s) {
        s.sendEncodedMessage(0, chan, AndroidAuto::PROTOCOL_MESSAGE::AudioFocusResponse, response);
    });
}

void HeadunitEventHandler::onVideoFocusHappened(bool hasFocus, bool unrequested) {
    if (m_huThreadInterface == nullptr) {
        qWarning() << "Headunit thread interface not set!";
        return;
    }

    m_huThreadInterface->queueCommand([hasFocus, unrequested](AndroidAuto::IHUConnectionThreadInterface& s) {
        HU::VideoFocus videoFocusGained;
        videoFocusGained.set_mode(hasFocus ? HU::VIDEO_FOCUS_MODE_FOCUSED : HU::VIDEO_FOCUS_MODE_UNFOCUSED);
        videoFocusGained.set_unrequested(unrequested);
        s.sendEncodedMessage(0, AndroidAuto::VideoChannel, AndroidAuto::MEDIA_CHANNEL_MESSAGE::VideoFocus, videoFocusGained);
    });

    // Set speed to 0
    /*HU::SensorEvent sensorEvent;
    sensorEvent.add_location_data()->set_speed(0);
    m_huThreadInterface->queueCommand([sensorEvent](AndroidAuto::IHUConnectionThreadInterface& s) {
        s.sendEncodedMessage(0, AndroidAuto::SensorChannel, AndroidAuto::SENSOR_CHANNEL_MESSAGE::SensorEvent, sensorEvent);
    });*/
}

void HeadunitEventHandler::startMedia() {
    if (m_huThreadInterface == nullptr) {
        qWarning() << "Headunit thread interface not set!";
        return;
    }

    m_huThreadInterface->queueCommand([](AndroidAuto::IHUConnectionThreadInterface& s) {
        HU::InputEvent inputEvent;
        inputEvent.set_timestamp(HeadunitEventHandler::get_cur_timestamp());
        HU::ButtonInfo* buttonInfo = inputEvent.mutable_button()->add_button();
        buttonInfo->set_is_pressed(true);
        buttonInfo->set_meta(0);
        buttonInfo->set_long_press(false);
        buttonInfo->set_scan_code(AndroidAuto::HUIB_START);

        s.sendEncodedMessage(0, AndroidAuto::TouchChannel, AndroidAuto::INPUT_CHANNEL_MESSAGE::InputEvent, inputEvent);
        buttonInfo->set_is_pressed(false);
        s.sendEncodedMessage(0, AndroidAuto::TouchChannel, AndroidAuto::INPUT_CHANNEL_MESSAGE::InputEvent, inputEvent);
    });
}

void HeadunitEventHandler::stopMedia() {
    if (m_huThreadInterface == nullptr) {
        qWarning() << "Headunit thread interface not set!";
        return;
    }

    m_huThreadInterface->queueCommand([](AndroidAuto::IHUConnectionThreadInterface& s) {
        HU::InputEvent inputEvent;
        inputEvent.set_timestamp(HeadunitEventHandler::get_cur_timestamp());
        HU::ButtonInfo* buttonInfo = inputEvent.mutable_button()->add_button();
        buttonInfo->set_is_pressed(true);
        buttonInfo->set_meta(0);
        buttonInfo->set_long_press(false);
        buttonInfo->set_scan_code(AndroidAuto::HUIB_STOP);

        s.sendEncodedMessage(0, AndroidAuto::TouchChannel, AndroidAuto::INPUT_CHANNEL_MESSAGE::InputEvent, inputEvent);
        buttonInfo->set_is_pressed(false);
        s.sendEncodedMessage(0, AndroidAuto::TouchChannel, AndroidAuto::INPUT_CHANNEL_MESSAGE::InputEvent, inputEvent);
    });
}

void HeadunitEventHandler::prevTrack() {
    if (m_huThreadInterface == nullptr) {
        qWarning() << "Headunit thread interface not set!";
        return;
    }

    m_huThreadInterface->queueCommand([](AndroidAuto::IHUConnectionThreadInterface& s) {
        HU::InputEvent inputEvent;
        inputEvent.set_timestamp(HeadunitEventHandler::get_cur_timestamp());
        HU::ButtonInfo* buttonInfo = inputEvent.mutable_button()->add_button();
        buttonInfo->set_is_pressed(true);
        buttonInfo->set_meta(0);
        buttonInfo->set_long_press(false);
        buttonInfo->set_scan_code(AndroidAuto::HUIB_PREV);

        s.sendEncodedMessage(0, AndroidAuto::TouchChannel, AndroidAuto::INPUT_CHANNEL_MESSAGE::InputEvent, inputEvent);
        buttonInfo->set_is_pressed(false);
        s.sendEncodedMessage(0, AndroidAuto::TouchChannel, AndroidAuto::INPUT_CHANNEL_MESSAGE::InputEvent, inputEvent);
    });
}

void HeadunitEventHandler::nextTrack() {
    if (m_huThreadInterface == nullptr) {
        qWarning() << "Headunit thread interface not set!";
        return;
    }

    m_huThreadInterface->queueCommand([](AndroidAuto::IHUConnectionThreadInterface& s) {
        HU::InputEvent inputEvent;
        inputEvent.set_timestamp(HeadunitEventHandler::get_cur_timestamp());
        HU::ButtonInfo* buttonInfo = inputEvent.mutable_button()->add_button();
        buttonInfo->set_is_pressed(true);
        buttonInfo->set_meta(0);
        buttonInfo->set_long_press(false);
        buttonInfo->set_scan_code(AndroidAuto::HUIB_NEXT);

        s.sendEncodedMessage(0, AndroidAuto::TouchChannel, AndroidAuto::INPUT_CHANNEL_MESSAGE::InputEvent, inputEvent);
        buttonInfo->set_is_pressed(false);
        s.sendEncodedMessage(0, AndroidAuto::TouchChannel, AndroidAuto::INPUT_CHANNEL_MESSAGE::InputEvent, inputEvent);
    });
}

void HeadunitEventHandler::touchEvent(HU::TouchInfo::TOUCH_ACTION action, const QPoint& point) {
    if (m_huThreadInterface == nullptr) {
        qWarning() << "Headunit thread interface not set!";
        return;
    }

    m_huThreadInterface->queueCommand([action, point](AndroidAuto::IHUConnectionThreadInterface& s) {
        HU::InputEvent inputEvent;
        inputEvent.set_timestamp(HeadunitEventHandler::get_cur_timestamp());
        HU::TouchInfo* touchEvent = inputEvent.mutable_touch();
        touchEvent->set_action(action);
        HU::TouchInfo::Location* touchLocation = touchEvent->add_location();
        touchLocation->set_x(point.x());
        touchLocation->set_y(point.y());
        touchLocation->set_pointer_id(0);

        /* Send touch event */

        int ret = s.sendEncodedMessage(0, AndroidAuto::TouchChannel, AndroidAuto::INPUT_CHANNEL_MESSAGE::InputEvent, inputEvent);
        if (ret < 0) {
            qDebug("aa_touch_event(): hu_aap_enc_send() failed with (%d)", ret);
        }
    });
}

void HeadunitEventHandler::setNightMode(bool night) {
    if (m_huThreadInterface == nullptr) {
        qWarning() << "Headunit thread interface not set!";
        return;
    }

    HU::SensorEvent sensorEvent;
    sensorEvent.add_night_mode()->set_is_night(night);
    m_huThreadInterface->queueCommand([sensorEvent](AndroidAuto::IHUConnectionThreadInterface& s) {
        s.sendEncodedMessage(0, AndroidAuto::SensorChannel, AndroidAuto::SENSOR_CHANNEL_MESSAGE::SensorEvent, sensorEvent);
    });
}

void HeadunitEventHandler::setVSS(double speedms) {
    if (m_huThreadInterface == nullptr) {
        qWarning() << "Headunit thread interface not set!";
        return;
    }

    int32_t restriction = HU::SensorEvent_DrivingStatus::DRIVE_STATUS_UNRESTRICTED;
    int32_t speed = (int32_t)qRound(speedms * 1E3);

    if(this->m_gear == HU::SensorEvent_GearData_GEAR::SensorEvent_GearData_GEAR_GEAR_REVERSE && speed > 0) {
        speed = speed * -1; //AA expects speeds in reverse to be negative
    }

    //At speed greater than 0.5m/s we restrict to avoid driver distraction
    if(speed >= 500 || speed <= -500) {
        restriction = HU::SensorEvent_DrivingStatus::DRIVE_STATUS_NO_CONFIG |
                      HU::SensorEvent_DrivingStatus::DRIVE_STATUS_NO_KEYBOARD_INPUT |
                      HU::SensorEvent_DrivingStatus::DRIVE_STATUS_NO_VIDEO;
    }

    HU::SensorEvent sensorEvent;
    sensorEvent.add_speed_data()->set_speed_e6(speed);
    sensorEvent.add_driving_status()->set_status(restriction);
    m_huThreadInterface->queueCommand([sensorEvent](AndroidAuto::IHUConnectionThreadInterface& s) {
        s.sendEncodedMessage(0, AndroidAuto::SensorChannel, AndroidAuto::SENSOR_CHANNEL_MESSAGE::SensorEvent, sensorEvent);
    });
}

void HeadunitEventHandler::setGear(int gear) {
    if (m_huThreadInterface == nullptr) {
        qWarning() << "Headunit thread interface not set!";
        return;
    }

    switch(gear) {
        case 0: //N
            this->m_gear = HU::SensorEvent_GearData_GEAR::SensorEvent_GearData_GEAR_GEAR_NEUTRAL;
            break;
        case 1: //1st
            this->m_gear = HU::SensorEvent_GearData_GEAR::SensorEvent_GearData_GEAR_GEAR_FIRST;
            break;
        case 2: //2nd
            this->m_gear = HU::SensorEvent_GearData_GEAR::SensorEvent_GearData_GEAR_GEAR_SECOND;
            break;
        case 3: //3rd
            this->m_gear = HU::SensorEvent_GearData_GEAR::SensorEvent_GearData_GEAR_GEAR_THIRD;
            break;
        case 4: //4th
            this->m_gear = HU::SensorEvent_GearData_GEAR::SensorEvent_GearData_GEAR_GEAR_FOURTH;
            break;
        case 5: //5th
            this->m_gear = HU::SensorEvent_GearData_GEAR::SensorEvent_GearData_GEAR_GEAR_FIFTH;
            break;
        case 6: //6th
            this->m_gear = HU::SensorEvent_GearData_GEAR::SensorEvent_GearData_GEAR_GEAR_SIXTH;
            break;
        case 254:
            //Reverse
            this->m_gear = HU::SensorEvent_GearData_GEAR::SensorEvent_GearData_GEAR_GEAR_REVERSE;
            break;
        case 255:
            //Shifting
            return;
        default:
            return;
     }

    HU::SensorEvent sensorEvent;
    sensorEvent.add_gear_data()->set_gear(this->m_gear);
    m_huThreadInterface->queueCommand([sensorEvent](AndroidAuto::IHUConnectionThreadInterface& s) {
        s.sendEncodedMessage(0, AndroidAuto::SensorChannel, AndroidAuto::SENSOR_CHANNEL_MESSAGE::SensorEvent, sensorEvent);
    });
}

void HeadunitEventHandler::setLocation(double latitude, double longitude, double track, double speed, double altitude, double eph) {
    if (m_huThreadInterface == nullptr) {
        qWarning() << "Headunit thread interface not set!";
        return;
    }

    HU::SensorEvent sensorEvent;
    HU::SensorEvent::LocationData* location = sensorEvent.add_location_data();
    location->set_timestamp(get_cur_timestamp());
    location->set_latitude(static_cast<int32_t>(latitude * 1E7));
    location->set_longitude(static_cast<int32_t>(longitude * 1E7));
    location->set_bearing(static_cast<int32_t>(track * 1E6));
    location->set_speed(static_cast<int32_t>(speed * 1E3));
    location->set_altitude(static_cast<int32_t>(altitude * 1E2));
    location->set_accuracy(static_cast<uint32_t>(eph * 1E3));

    m_huThreadInterface->queueCommand([sensorEvent](AndroidAuto::IHUConnectionThreadInterface& s) {
        s.sendEncodedMessage(0, AndroidAuto::SensorChannel, AndroidAuto::SENSOR_CHANNEL_MESSAGE::SensorEvent, sensorEvent);
    });
}
