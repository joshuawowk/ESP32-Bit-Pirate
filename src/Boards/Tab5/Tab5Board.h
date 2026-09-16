#pragma once

#ifdef DEVICE_TAB5

#include "Boards/Tab5/Tab5DeviceView.h"
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
    Tab5DeviceView deviceView;
    Tab5Input deviceInput;
};

#endif
