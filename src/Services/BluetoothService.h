#pragma once

#include "Boards/Common/BoardCapabilities.h"
#include "Interfaces/IBluetoothService.h"

#if HAS_NATIVE_BLE

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLESecurity.h>
#include "BLEHIDDevice.h"
#include "HIDTypes.h"
#include "Data/AsciiHid.h"

struct ScannedDevice {
    std::string name;
    std::string address;
    int rssi;
    std::string type;
    std::string adSummary;
};

class BluetoothService : public IBluetoothService {
private:
    BLEServer* server = nullptr;
    BLESecurity* security = nullptr;
    BLEHIDDevice* hid = nullptr;
    BLECharacteristic* mouseInput = nullptr;
    BLECharacteristic* keyboardInput = nullptr;
    bool connected = false;
    static const uint8_t HID_REPORT_MAP[];
    static constexpr uint16_t HID_REPORT_MAP_SIZE = 125;
    BluetoothMode mode = BluetoothMode::NONE;
    static BLEScan* bleScan;
    static std::string lastAdParsed;
    inline static uint32_t receivedFramesCount = 0;

public:
    class PassiveAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
    public:
        void onResult(BLEAdvertisedDevice advertisedDevice) override;
    };

    // begin / end server BT
    void startServer(const std::string& deviceName = "Bit-Pirate-Bluetooth") override;
    void stopServer() override;
    void releaseBtClassic() override;

    // Init client
    void init(const std::string& deviceName = "Bit-Pirate-Bluetooth") override;
    void deinit() override;

    // Pair as client
    void pairWithAddress(const std::string& addrStr) override;      // pair <addr>

    // Connexion
    void onConnect();
    void onDisconnect();
    bool isConnected() const override;

    // HID – Kb
    void sendKeyboardText(const std::string& text) override;
    void sendKeyboardReport(uint8_t modifier, const std::array<uint8_t, 6>& keys) override;

    // HID – Mouse
    void mouseMove(int16_t x, int16_t y) override;
    void clickMouse() override;  // Simule un clic
    void sendMouseReport(int16_t x, int16_t y, uint8_t buttons) override;

    // Utils
    void sendEmptyReports() override;
    bool spoofMacAddress(const std::string& macStr) override;
    std::string getMacAddress() override;
    BluetoothMode getMode() override;
    void switchToMode(BluetoothMode newMode) override;

    // Scan
    std::vector<std::string> scanDevices(int seconds = 10) override;
    std::vector<std::string> connectTo(const std::string& addr) override;

    // Bluetooth sniffing
    class PassiveBLEAdvertisedDeviceCallbacks;
    void startPassiveSniffing() override { startPassiveBluetoothSniffing(); }
    void stopPassiveSniffing() override { stopPassiveBluetoothSniffing(); }
    std::vector<std::string> getPassiveSniffLog() override { return getBluetoothSniffLog(); }
    static void startPassiveBluetoothSniffing();
    static void stopPassiveBluetoothSniffing();
    static std::vector<std::string> getBluetoothSniffLog();
    static bool isLikelyConnectable(BLEAdvertisedDevice& device);
    static std::string parseAdTypes(const uint8_t* payload, size_t len);
    static std::vector<std::string> bluetoothSniffLog;
    static portMUX_TYPE bluetoothSniffMux;
};

#else  // !HAS_NATIVE_BLE

// -----------------------------------------------------------------------------
// No-op stub for SoCs without a native Bluetooth controller (e.g. ESP32-P4).
// Keeps the class name so DependencyProvider links unchanged; every command
// returns "not available" behaviour (mode NONE, empty scans, never connected).
// -----------------------------------------------------------------------------

#include <array>
#include <cstdint>
#include <string>
#include <vector>

class BluetoothService : public IBluetoothService {
public:
    void startServer(const std::string& = "Bit-Pirate-Bluetooth") override {}
    void stopServer() override {}
    void releaseBtClassic() override {}

    void init(const std::string& = "Bit-Pirate-Bluetooth") override {}
    void deinit() override {}

    void pairWithAddress(const std::string&) override {}

    bool isConnected() const override { return false; }

    void sendKeyboardText(const std::string&) override {}
    void sendKeyboardReport(uint8_t, const std::array<uint8_t, 6>&) override {}

    void mouseMove(int16_t, int16_t) override {}
    void clickMouse() override {}
    void sendMouseReport(int16_t, int16_t, uint8_t) override {}

    void sendEmptyReports() override {}
    bool spoofMacAddress(const std::string&) override { return false; }
    std::string getMacAddress() override { return {}; }
    BluetoothMode getMode() override { return BluetoothMode::NONE; }
    void switchToMode(BluetoothMode) override {}

    std::vector<std::string> scanDevices(int = 10) override { return {}; }
    std::vector<std::string> connectTo(const std::string&) override { return {}; }

    void startPassiveSniffing() override {}
    void stopPassiveSniffing() override {}
    std::vector<std::string> getPassiveSniffLog() override { return {}; }
};

#endif  // HAS_NATIVE_BLE
