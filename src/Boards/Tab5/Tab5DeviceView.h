#pragma once

#if defined(DEVICE_TAB5)

#include <M5Unified.h>
#include "Interfaces/IDeviceView.h"
#include "Enums/ModeEnum.h"
#include "Enums/TerminalTypeEnum.h"
#include "Models/PinoutConfig.h"

// Dedicated device view for the M5Stack Tab5's large MIPI-DSI panel
// (1280x720 in the landscape rotation used by Tab5Board). Unlike the shared
// M5DeviceView, every layout here is derived from M5.Display.width()/height()
// with large, touch-friendly elements. The horizontalSelection() left/right
// arrow zones line up with Tab5Input's left-third / right-third / center touch
// mapping so the boot selectors can be driven by tapping the screen.
class Tab5DeviceView : public IDeviceView {
public:
    void initialize() override;
    SPIClass& getSharedSpiInstance() override;
    void* getScreen() override;
    void logo() override;
    void welcome(TerminalTypeEnum& terminalType, std::string& terminalInfos) override;
    void show(PinoutConfig& config) override;
    void loading() override;
    void adapterMode(const std::string& adapterName, const std::string& description, const std::vector<std::string>& details) override;
    void clear() override;
    void drawLogicTrace(uint8_t pin, const std::vector<uint8_t>& buffer, uint8_t step) override;
    void drawAnalogicTrace(uint8_t pin, const std::vector<uint8_t>& buffer, uint8_t step) override;
    void drawWaterfall(
        const std::string& title,
        float startValue,
        float endValue,
        const char* unit,
        int rowIndex,
        int rowCount,
        int level
    ) override;
    void setRotation(uint8_t rotation) override;
    void setBrightness(uint8_t brightness) override;
    uint8_t getBrightness() override;
    void topBar(const std::string& title, bool submenu, bool searchBar) override;
    void horizontalSelection(
        const std::vector<std::string>& options,
        uint16_t selectedIndex,
        const std::string& description1,
        const std::string& description2) override;

private:
    void welcomeSerial(const std::string& baudStr);
    void welcomeWeb(const std::string& ipStr);
    void welcomeHotspot(const std::string& ipStr);
    void banner(const std::string& title);
};

#endif
