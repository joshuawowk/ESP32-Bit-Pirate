#pragma once

#include <Interfaces/IHostSerial.h>
#include "Boards/Common/BoardCapabilities.h"

class DefaultHostSerial : public IHostSerial {
public:
    void begin(unsigned long baud) override {
        Serial.begin(baud);
    }

    void waitReady() override {
        while (!Serial) {
            delay(10);
        }
    }

    int available() override {
        return Serial.available();
    }

    int availableForWrite() override {
        return Serial.availableForWrite();
    }

    int peek() override {
        return Serial.peek();
    }

    int read() override {
        return Serial.read();
    }

    size_t readBytes(uint8_t* buffer, size_t length) override {
        return Serial.readBytes(buffer, length);
    }

    size_t write(uint8_t value) override {
        return Serial.write(value);
    }

    size_t write(const uint8_t* buffer, size_t length) override {
        return Serial.write(buffer, length);
    }

    size_t print(const char* text) override {
        return Serial.print(text);
    }

    size_t println(const char* text) override {
        return Serial.println(text);
    }

    size_t println() override {
        return Serial.println();
    }

    void flush() override {
        Serial.flush();
    }

    bool setRxBufferSize(size_t size) override {
        return Serial.setRxBufferSize(size);
    }

    void setTimeout(unsigned long timeoutMs) override {
        Serial.setTimeout(timeoutMs);
    }

    uint32_t baudRate() override {
        return Serial.baudRate();
    }

    void disableReboot() override {
        // enableReboot() is a TinyUSB USBCDC feature (auto-reset on DTR/1200bps
        // touch). On the ESP32-P4 Serial is an HWCDC (USB-Serial-JTAG) with no
        // such member and no CDC-triggered reboot, so this is a no-op there.
#if ARDUINO_USB_CDC_ON_BOOT && HAS_USB_STACK
        Serial.enableReboot(false);
#endif
    }

    void waitForPress() override {
        while (!Serial.available()) {}
    }
};
