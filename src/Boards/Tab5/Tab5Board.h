#pragma once

#ifdef DEVICE_TAB5

#include "Boards/Common/Views/M5DeviceView.h"
#include "Boards/Tab5/Tab5Input.h"
#include "Boards/Common/Serial/BoardHostSerial.h"

class Tab5Board final {
public:
    void initialize();
    IDeviceView& getDeviceView();
    IInput& getDeviceInput();
    IHostSerial& getHostSerial();

private:
    BoardHostSerial hostSerial;
    M5DeviceView deviceView;
    Tab5Input deviceInput;
};

#endif
