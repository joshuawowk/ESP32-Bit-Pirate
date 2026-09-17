#include "Configurators/TerminalTypeConfigurator.h"

TerminalTypeConfigurator::TerminalTypeConfigurator(HorizontalSelector& selector)
    : selector(selector) {}

TerminalTypeEnum TerminalTypeConfigurator::configure() {
    std::vector<std::string> options = {
        TerminalTypeEnumMapper::toString(TerminalTypeEnum::WiFiClient),
        TerminalTypeEnumMapper::toString(TerminalTypeEnum::WiFiAp),
        TerminalTypeEnumMapper::toString(TerminalTypeEnum::SerialPort),
        #if defined(DEVICE_CARDPUTER) || defined(DEVICE_TAB5)
            TerminalTypeEnumMapper::toString(TerminalTypeEnum::Standalone),
        #endif
    };

    int selected = 2; // Serial

    #if defined(DEVICE_M5STAMPS3) || defined(DEVICE_S3DEVKIT) || defined(DEVICE_CUSTOM)
        selected = selector.selectHeadless();
    #elif defined(DEVICE_TAB5)
        // Touch/keyboard menu. If left completely untouched for 30s it defaults
        // to Serial (the P4's USB-Serial-JTAG resets the chip whenever a terminal
        // connects, so this lets a connected serial/web host reach the CLI without
        // a re-tap). The first key or tap cancels the timer -- after that it waits
        // for a real selection instead of auto-picking whatever is highlighted.
        selected = selector.select(
            "ESP32 BIT PIRATE",
            options,
            "Select terminal type",
            "No input for 30s = Serial",
            30000,
            2  // default highlight: Serial
        );
    #else
        selected = selector.select(
            "ESP32 BIT PIRATE",
            options,
            "Select terminal type",
            ""
        );
    #endif

    switch (selected) {
        case 0: return TerminalTypeEnum::WiFiClient;
        case 1: return TerminalTypeEnum::WiFiAp;
        case 2: return TerminalTypeEnum::SerialPort;
        case 3: return TerminalTypeEnum::Standalone;
        default: return TerminalTypeEnum::None;
    }
}
