#include "Car2PCSerial.h"
#include <stdlib.h>

namespace Car2PCSerial {

    Car2PCSerial::Car2PCSerial() : 
        m_receiverState(StartByte),
        m_frameSize(0),
        m_dataBufferIndex(0),
        m_receivedBuffer(),
        m_scStatus(0),
        m_ffStatus(0),
        m_frStatus(0),
        m_mxStatus(0),
        m_rpStatus(0),
        m_plStatus(0)
        {}

    void Car2PCSerial::sendMessage(uint8_t length, char* message) {
        if (length <= MESSAGE_MAX_LENGTH) {
            size_t s = sizeof(Packet) + length;
            Packet* sendPacket = (Packet*)malloc(s*sizeof(uint8_t));
            //Start Byte
            sendPacket->startCommand = 0xFF;
            sendPacket->length = length;
            memcpy(sendPacket->data, message, length);
            m_callbacks->SendPacketCallback(s, sendPacket);
            free(sendPacket);
        }
    }

    void Car2PCSerial::receiveByte(uint8_t receivedByte) {
        /*Got the start byte restart the buffer*/

        if (receivedByte == 0xFF) {
            m_receiverState = ProtocolFrame::LengthByte;
            return;
        }

        switch (m_receiverState) {
            case ProtocolFrame::StartByte:
                return;
            case ProtocolFrame::LengthByte:
                m_frameSize = receivedByte;
                if (m_frameSize <= MESSAGE_MAX_LENGTH) {
                    m_dataBufferIndex = 0;
                    m_receiverState = ProtocolFrame::DataBuffer;
                }
                else {
                    //The frame size is too big to handle, ignore it.
                    m_receiverState = ProtocolFrame::StartByte;
                }
                return;
            case ProtocolFrame::DataBuffer:
                m_receivedBuffer[m_dataBufferIndex] = (char)receivedByte;
                if (m_dataBufferIndex == m_frameSize - 1) {
                    //Command fully loaded
                    m_receivedBuffer[m_frameSize] = '\0';
                    //m_callbacks->PrintString(m_receivedBuffer, m_frameSize);

                    if (strcmp("NT", m_receivedBuffer) == 0) {
                        m_callbacks->ButtonInputCommandCallback(Button::NEXT_TRACK);
                    }
                    else if (strcmp("PT", m_receivedBuffer) == 0) {
                        m_callbacks->ButtonInputCommandCallback(Button::PREV_TRACK);
                    }
                    else if (strcmp("PL", m_receivedBuffer) == 0) {
                        if (m_plStatus == 0) {
                            m_callbacks->ButtonInputCommandCallback(Button::PLAY);
                            m_plStatus = 1;
                        }
                    }
                    else if (strcmp("ST", m_receivedBuffer) == 0) {
                        if (m_plStatus == 1) {
                            m_callbacks->ButtonInputCommandCallback(Button::STOP);
                            m_plStatus = 0;
                        }
                    }
                    else if (strncmp("RP", m_receivedBuffer, 2) == 0) {

                        if (m_receivedBuffer[2] == '1' && m_rpStatus == 0) {
                            m_callbacks->ButtonInputCommandCallback(Button::REPEAT_ON);
                            m_rpStatus = 1;
                        }
                        else if(m_receivedBuffer[2] == '0' && m_rpStatus == 1) {
                            m_callbacks->ButtonInputCommandCallback(Button::REPEAT_OFF);
                            m_rpStatus = 0;
                        }

                    }
                    else if (strncmp("MX", m_receivedBuffer, 2) == 0) {

                        if (m_receivedBuffer[2] == '1' && m_mxStatus == 0) {
                            m_callbacks->ButtonInputCommandCallback(Button::SHUFFLE_ON);
                            m_mxStatus = 1;
                        }
                        else if (m_receivedBuffer[2] == '0' && m_mxStatus == 1) {
                            m_callbacks->ButtonInputCommandCallback(Button::SHUFFLE_OFF);
                            m_mxStatus = 0;
                        }

                    }
                    else if (strncmp("FF", m_receivedBuffer, 2) == 0) {

                        if (m_receivedBuffer[2] == '1' && m_ffStatus == 0) {
                            m_callbacks->ButtonInputCommandCallback(Button::FF_ON);
                            m_ffStatus = 1;
                        }
                        else if (m_receivedBuffer[2] == '0' && m_ffStatus == 1) {
                            m_callbacks->ButtonInputCommandCallback(Button::FF_OFF);
                            m_ffStatus = 0;
                        }

                    }
                    else if (strncmp("FR", m_receivedBuffer, 2) == 0) {

                        if (m_receivedBuffer[2] == '1' && m_frStatus == 0) {
                            m_callbacks->ButtonInputCommandCallback(Button::RW_ON);
                            m_frStatus = 1;
                        }
                        else if (m_receivedBuffer[2] == '0' && m_frStatus == 1) {
                            m_callbacks->ButtonInputCommandCallback(Button::RW_OFF);
                            m_frStatus = 0;
                        }

                    }
                    else if (strncmp("SC", m_receivedBuffer, 2) == 0) {

                        if (m_receivedBuffer[2] == '1' && m_scStatus == 0) {
                            m_callbacks->ButtonInputCommandCallback(Button::SCAN_ON);
                            m_scStatus = 1;
                        }
                        else if (m_receivedBuffer[2] == '0' && m_scStatus == 1) {
                            m_callbacks->ButtonInputCommandCallback(Button::SCAN_OFF);
                            m_scStatus = 0;
                        }

                    }
                    else if (strcmp("PD", m_receivedBuffer) == 0) {
                        m_callbacks->ButtonInputCommandCallback(Button::PREV_DISC);
                    }
                    else if (strcmp("ND", m_receivedBuffer) == 0) {
                        m_callbacks->ButtonInputCommandCallback(Button::NEXT_DISC);
                    }
                    else {
                        //unknown
                        m_callbacks->PrintString("Unknown Command", 0);
                        m_callbacks->PrintString(m_receivedBuffer, m_frameSize);
                    }

                    m_receiverState = ProtocolFrame::StartByte;
                }
                else {
                    m_dataBufferIndex++;
                }
                return;
        }
    }

}