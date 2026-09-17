#include "HorizontalSelector.h"

HorizontalSelector::HorizontalSelector(
    IDeviceView& display,
    IInput& input,
    IUtilityService& utilityService)
    : display(display), input(input), utilityService(utilityService) {}

int HorizontalSelector::select(
    const std::string& title,
    const std::vector<std::string>& options,
    const std::string& description1,
    const std::string& description2,
    uint32_t timeoutMs,
    int startIndex) {

    int currentIndex = startIndex;
    if (currentIndex < 0) currentIndex = 0;
    if (!options.empty() && currentIndex >= static_cast<int>(options.size())) {
        currentIndex = static_cast<int>(options.size()) - 1;
    }
    int lastIndex = -1;

    display.topBar(title, false, false);

    // When a timeout is set we auto-select the default option after that long
    // with NO input at all. The first key or tap cancels this for good -- from
    // then on we just wait for the user to make a real selection.
    bool autoSelect = (timeoutMs != 0);
    uint32_t deadline = autoSelect ? (utilityService.nowMs() + timeoutMs) : 0;

    while (true) {
        if (lastIndex != currentIndex) {
            display.horizontalSelection(options, currentIndex, description1, description2);
            lastIndex = currentIndex;
        }

        char key;
        if (autoSelect) {
            key = input.readChar();  // non-blocking while the auto-select window is open
            if (key == KEY_NONE) {
                if (utilityService.nowMs() >= deadline) {
                    // No key/tap the whole window, so currentIndex is still the
                    // (clamped) default -- auto-select it.
                    return currentIndex;
                }
                utilityService.sleepMs(10);
                continue;
            }
            autoSelect = false;  // first key/tap cancels the timer permanently; process this key below
        } else {
            key = input.handler();  // blocking (original behavior for button/touch boards)
        }

        switch (key) {
            case KEY_ARROW_LEFT:
                currentIndex = (currentIndex > 0) ? currentIndex - 1 : options.size() - 1;
                break;
             case KEY_ARROW_RIGHT:
            #if !defined(DEVICE_TDISPLAYS3)                
               currentIndex = (currentIndex < options.size() - 1) ? currentIndex + 1 : 0;
                break;
            case KEY_OK:
            #endif

                return currentIndex;
            default:
                break;
        }
    }
}

int HorizontalSelector::selectHeadless() {
    std::vector<std::string> options = {
        TerminalTypeEnumMapper::toString(TerminalTypeEnum::WiFiClient),
        TerminalTypeEnumMapper::toString(TerminalTypeEnum::WiFiAp),
        TerminalTypeEnumMapper::toString(TerminalTypeEnum::SerialPort),
    };

    int selected = 2;  // default: Serial
    const unsigned long longPressMs = 800;

    display.topBar("ESP32 BIT PIRATE", false, false);
    display.horizontalSelection(
        options,
        selected,
        "Terminal auto-select",
        "Short press: CONNECT  Long press: HOTSPOT"
    );

    // 3 sec window:
    // - no input: Serial
    // - short press: WiFi Connect
    // - long press: WiFi Hotspot
    const uint32_t timeout = utilityService.nowMs() + 3000;
    while (utilityService.nowMs() < timeout) {
        char c = input.readChar();
        if (c == KEY_OK) {
            const uint32_t pressStart = utilityService.nowMs();
            while (input.readChar() == KEY_OK) {
                if (utilityService.nowMs() - pressStart >= longPressMs) {
                    selected = 1; // WiFi Hotspot
                    display.horizontalSelection(
                        options,
                        selected,
                        "Terminal selected",
                        "Starting hotspot..."
                    );
                    utilityService.sleepMs(250);
                    return selected;
                }
                utilityService.sleepMs(10);
            }

            selected = 0; // WiFi Connect
            display.horizontalSelection(
                options,
                selected,
                "Terminal selected",
                "Connecting to WiFi..."
            );
            utilityService.sleepMs(250);
            break;
        }
        utilityService.sleepMs(10);
    }

    return selected;
}
