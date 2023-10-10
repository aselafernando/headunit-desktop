#pragma once

#include <stdint.h>

typedef enum Button {
    PLAY = 0,
    STOP,
    NEXT_TRACK,
    PREV_TRACK,
    REPEAT_ON,
    REPEAT_OFF,
    SHUFFLE_ON,
    SHUFFLE_OFF,
    FF_ON,
    FF_OFF,
    RW_ON,
    RW_OFF,
    SCAN_ON,
    SCAN_OFF,
    PREV_DISC,
    NEXT_DISC
} Button;

typedef struct {
    uint8_t startCommand;
    uint8_t length;
    uint8_t data[];
} Packet;

class PlatformCallbacks {
public:
    virtual void ButtonInputCommandCallback(Button) = 0;
    virtual void SendPacketCallback(size_t, Packet*) = 0;
    virtual void PrintString(char *, int) = 0;
};