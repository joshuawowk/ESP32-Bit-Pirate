#pragma once

#include <string>
#include <vector>
#include "Interfaces/IDeviceView.h"
#include "Interfaces/IInput.h"
#include "Interfaces/IUtilityService.h"
#include "Data/InputKeys.h"
#include "Enums/TerminalTypeEnum.h"

class HorizontalSelector {
public:
    HorizontalSelector(IDeviceView& display, IInput& input, IUtilityService& utilityService);

    int select(
        const std::string& title,
        const std::vector<std::string>& options,
        const std::string& description1 = "",
        const std::string& description2 = "",
        uint32_t timeoutMs = 0,   // 0 = block until input (default). >0 = auto-select startIndex after no input.
        int startIndex = 0        // initially highlighted option (also the auto-select target on timeout)
    );

    int selectHeadless();

private:
    IDeviceView& display;
    IInput& input;
    IUtilityService& utilityService;
};
