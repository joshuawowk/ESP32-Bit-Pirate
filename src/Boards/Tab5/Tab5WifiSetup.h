#pragma once

#ifdef DEVICE_TAB5

#include <Interfaces/IDeviceView.h>

// Screen-based Wi-Fi client bring-up for the Tab5. Loads credentials stored in
// NVS ("wifi_settings"), shows connection status on the MIPI-DSI panel, and
// returns true once associated (Wi-Fi is served by the on-board ESP32-C6 over
// esp_hosted, so plain WiFi.begin() works transparently).
bool setupTab5Wifi(IDeviceView& view);

#endif
