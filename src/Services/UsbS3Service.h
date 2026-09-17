#pragma once

#include "Boards/Common/BoardCapabilities.h"
#include "Interfaces/IUsbS3Service.h"

#if HAS_USB_STACK

#include <Arduino.h>
#include <USB.h>
#include <USBMSC.h>
#include <SPI.h>
#include <SD.h>
#include <string>
#include <USBHIDMouse.h>
#include <USBHIDKeyboard.h>
#include <USBHIDGamepad.h>
#include <USBHIDSystemControl.h>
#include "usb/usb_host.h"
#include "usb/usb_types_ch9.h"
#include "usb/usb_types_stack.h"
#include "usb/usb_helpers.h"
#include "esp_log.h"
#include <sstream>

class UsbS3Service : public IUsbS3Service {
public:
    UsbS3Service();

    // Status
    bool isKeyboardActive() const;
    bool isStorageActive() const;
    bool isMouseActive() const;
    bool isGamepadActive() const;
    bool isHostActive() const;
    bool isSystemControlActive() const;

    // Keyboard actions
    void keyboardBegin();
    void keyboardSendString(const std::string& text);
    void keyboardSendChunkedString(const std::string& data, size_t chunkSize, unsigned long delayBetweenChunks);

    // Mass Storage mode
    void storageBegin(uint8_t cs, uint8_t clk, uint8_t miso, uint8_t mosi);

    // Mouse actions
    void mouseBegin();
    void mouseMove(int x, int y);
    void mouseClick(int button);
    void mouseRelease(int button);

    // Gamepad actions
    void gamepadBegin();
    void gamepadPress(const std::string& name);

    // Host
    bool usbHostBegin();
    std::string usbHostTick();
    void usbHostEnd();
    static void hostClientEventCb(const usb_host_client_event_msg_t *event_msg, void *arg);

    // System control
    void systemControlBegin();
    void systemControlEnd();
    void systemSleep();
    void systemWake();
    void systemPowerOff(uint32_t holdMs = 10);

    // Config
    void configure(const char* productStr, const char* manufacturerStr, const char* serialStr, uint16_t vid, uint16_t pid, const char* webUSBString);
    void reset();
    std::string getUsbSerialFromEfuseMac();

private:
    bool initialized;

    // HID
    bool gamepadActive = false;
    bool keyboardActive = false;
    bool mouseActive = false;
    bool systemControlActive = false;
    unsigned long hidInitTime;

    // Mass Storage
    SPIClass sdSPI;
    bool storageActive;
    static int32_t storageReadCallback(uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize);
    static int32_t storageWriteCallback(uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize);
    static bool usbStartStopCallback(uint8_t power_condition, bool start, bool load_eject);
    void setupStorageEvent();

    // Host
    bool stopTinyUsbDevice();
    bool hostInstalled = false;
    usb_host_client_handle_t hostClient = nullptr;
    uint8_t devAddr = 0;
    bool attachPending = false;
    bool detachPending = false;
    bool dumpedThisAttach = false;
    inline static const char* TAG_USBHOST = "UsbHost";
};

#else  // !HAS_USB_STACK

// -----------------------------------------------------------------------------
// No-op stub for SoCs/USB modes without the Arduino TinyUSB device stack
// (e.g. the ESP32-P4 Tab5, which runs USB in HWCDC / USB-Serial-JTAG mode).
// Keeps the class name so DependencyProvider links unchanged; USB HID / MSC /
// host "adapter" features report inactive and do nothing.
// -----------------------------------------------------------------------------

#include <cstddef>
#include <cstdint>
#include <string>

class UsbS3Service : public IUsbS3Service {
public:
    bool isKeyboardActive() const override { return false; }
    bool isStorageActive() const override { return false; }
    bool isMouseActive() const override { return false; }
    bool isGamepadActive() const override { return false; }
    bool isHostActive() const override { return false; }
    bool isSystemControlActive() const override { return false; }

    void keyboardBegin() override {}
    void keyboardSendString(const std::string&) override {}
    void keyboardSendChunkedString(const std::string&, size_t, unsigned long) override {}

    void storageBegin(uint8_t, uint8_t, uint8_t, uint8_t) override {}

    void mouseBegin() override {}
    void mouseMove(int, int) override {}
    void mouseClick(int) override {}
    void mouseRelease(int) override {}

    void gamepadBegin() override {}
    void gamepadPress(const std::string&) override {}

    bool usbHostBegin() override { return false; }
    std::string usbHostTick() override { return {}; }
    void usbHostEnd() override {}

    void systemControlBegin() override {}
    void systemControlEnd() override {}
    void systemSleep() override {}
    void systemWake() override {}
    void systemPowerOff(uint32_t = 10) override {}

    void configure(const char*, const char*, const char*, uint16_t, uint16_t, const char*) override {}
    void reset() override {}
    std::string getUsbSerialFromEfuseMac() override { return {}; }
};

#endif  // HAS_USB_STACK
