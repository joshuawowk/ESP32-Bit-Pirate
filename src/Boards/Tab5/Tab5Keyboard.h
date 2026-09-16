#pragma once

#if defined(DEVICE_TAB5)

#include <Arduino.h>

// Reader for the M5Stack "Keyboard for Tab5" (A164): a 70-key STM32-based
// I2C keyboard at 0x6D on its own bus (SDA=GPIO0, SCL=GPIO1) via Ext.Port1.
// Runs the keyboard in HID mode and maps USB-HID usage codes to the Bit Pirate
// input sentinels (KEY_OK / KEY_DEL / KEY_ARROW_* / printable chars).
//
// Singleton so the touch input (Tab5Input) and the standalone terminal input
// (Tab5KeyboardInput) share one I2C bus init and one event queue.
class Tab5Keyboard {
public:
    static Tab5Keyboard& instance();

    void begin();                 // idempotent: init the bus, probe 0x6D, select HID mode
    bool isPresent() const { return present; }
    char readChar();              // KEY_NONE if nothing queued, else a mapped char

private:
    Tab5Keyboard() = default;
    bool started = false;
    bool present = false;
};

#endif
