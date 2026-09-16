#ifdef DEVICE_TAB5

#include "Boards/Tab5/Tab5Board.h"
#include <M5Unified.h>

void Tab5Board::initialize() {
    auto cfg = M5.config();
    M5.begin(cfg);

    // The Tab5 MIPI-DSI panel is 720x1280 in its native (portrait) orientation.
    // Rotation 3 = landscape with the USB-C/ports edge oriented the right way up
    // (rotation 1 renders the UI upside down on this panel).
    deviceView.setRotation(3);
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
