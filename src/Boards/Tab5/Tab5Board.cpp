#ifdef DEVICE_TAB5

#include "Boards/Tab5/Tab5Board.h"
#include <M5Unified.h>

void Tab5Board::initialize() {
    auto cfg = M5.config();
    M5.begin(cfg);

    // The Tab5 MIPI-DSI panel is 720x1280 in its native (portrait) orientation.
    // Rotate to landscape so the shared M5DeviceView layout reads left-to-right.
    deviceView.setRotation(1);
    deviceView.setBrightness(255);

    deviceView.logo();
    deviceInput.waitPress(3000);
}

IDeviceView& Tab5Board::getDeviceView() {
    return deviceView;
}

IInput& Tab5Board::getDeviceInput() {
    return deviceInput;
}

IHostSerial& Tab5Board::getHostSerial() {
    return hostSerial;
}

#endif
