#pragma once

#include <string.h>
#include "../common.h"

#include <stdio.h>

#define MESSAGE_MAX_LENGTH 254

namespace Car2PCSerial{
    typedef enum ProtocolFrame {
        StartByte,
        LengthByte,
        DataBuffer
    } ProtocolFrame;

    class Car2PCSerial
    {
        public:
            Car2PCSerial() ;
            void receiveByte(uint8_t receivedByte);
            void sendMessage(uint8_t length, char *message);

            virtual void setCallbacks(PlatformCallbacks *callbacks) {
                m_callbacks = callbacks;
            }
        private:
            PlatformCallbacks *m_callbacks;
            //Receiver state machine
            ProtocolFrame m_receiverState;
            uint8_t m_frameSize;
            uint8_t m_dataBufferIndex;

            uint8_t m_scStatus;
            uint8_t m_ffStatus;
            uint8_t m_frStatus;
            uint8_t m_mxStatus;
            uint8_t m_rpStatus;
            uint8_t m_plStatus;

            char m_receivedBuffer[MESSAGE_MAX_LENGTH+1];
    };
}
extern Car2PCSerial::Car2PCSerial car2pcSerial;
