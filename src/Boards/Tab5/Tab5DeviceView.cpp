#if defined(DEVICE_TAB5)

#include "Boards/Tab5/Tab5DeviceView.h"
#include "Data/WelcomeScreen.h"
#include "States/GlobalState.h"
#include <SPI.h>
#include <algorithm>

namespace {
constexpr uint16_t COL_BG      = 0x0000;  // black
constexpr uint16_t COL_PRIMARY = 0x05A3;
constexpr uint16_t COL_DARK    = 0x0841;
constexpr uint16_t COL_LIGHT   = 0xD69A;
constexpr uint16_t COL_TEXT    = 0xE71C;
constexpr uint16_t COL_TEXT_ALT= 0x7BEF;

// Horizontal-centred text at a given top-left Y.
void centerText(const char* t, int y) {
    auto& lcd = M5.Display;
    lcd.setCursor((lcd.width() - lcd.textWidth(t)) / 2, y);
    lcd.print(t);
}
void centerText(const std::string& t, int y) { centerText(t.c_str(), y); }
}  // namespace

void Tab5DeviceView::initialize() { M5.Display.fillScreen(COL_BG); }
SPIClass& Tab5DeviceView::getSharedSpiInstance() { return SPI; }
void* Tab5DeviceView::getScreen() { return &M5.Display; }
void Tab5DeviceView::clear() { M5.Display.fillScreen(COL_BG); }
void Tab5DeviceView::setRotation(uint8_t r) { M5.Display.setRotation(r); }
void Tab5DeviceView::setBrightness(uint8_t b) { M5.Display.setBrightness(b); }
uint8_t Tab5DeviceView::getBrightness() { return M5.Display.getBrightness(); }

void Tab5DeviceView::banner(const std::string& title) {
    auto& lcd = M5.Display;
    lcd.fillScreen(COL_BG);
    lcd.setTextSize(5);
    lcd.setTextColor(COL_PRIMARY, COL_BG);
    centerText(title, 40);
}

void Tab5DeviceView::logo() {
    auto& lcd = M5.Display;
    clear();
    GlobalState& state = GlobalState::getInstance();

    int imgX = (lcd.width() - WELCOME_IMAGE_WIDTH) / 2;
    int imgY = lcd.height() / 3 - WELCOME_IMAGE_HEIGHT / 2;
    lcd.setSwapBytes(true);
    lcd.pushImage(imgX, imgY, WELCOME_IMAGE_WIDTH, WELCOME_IMAGE_HEIGHT, WelcomeScreen);
    lcd.setSwapBytes(false);

    std::string text = std::string("ESP32 Bit Pirate  ") + state.getVersion();
    int boxW = lcd.width() * 3 / 5;
    int boxH = 72;
    int boxX = (lcd.width() - boxW) / 2;
    int boxY = lcd.height() * 2 / 3;
    lcd.fillRoundRect(boxX, boxY, boxW, boxH, 8, COL_DARK);
    lcd.drawRoundRect(boxX, boxY, boxW, boxH, 8, COL_PRIMARY);
    lcd.setTextSize(4);
    lcd.setTextColor(COL_TEXT, COL_DARK);
    centerText(text, boxY + (boxH - 32) / 2);
}

void Tab5DeviceView::welcome(TerminalTypeEnum& terminalType, std::string& terminalInfos) {
    if (terminalType == TerminalTypeEnum::SerialPort) {
        welcomeSerial(terminalInfos);
    } else if (terminalType == TerminalTypeEnum::WiFiAp) {
        welcomeHotspot(terminalInfos);
    } else {
        welcomeWeb(terminalInfos);
    }
}

void Tab5DeviceView::welcomeSerial(const std::string& baudStr) {
    banner("Serial (USB CDC)");
    auto& lcd = M5.Display;
    int boxW = lcd.width() * 3 / 5;
    int boxH = 96;
    int boxX = (lcd.width() - boxW) / 2;
    int boxY = lcd.height() / 2 - boxH / 2;
    lcd.fillRoundRect(boxX, boxY, boxW, boxH, 8, COL_DARK);
    lcd.drawRoundRect(boxX, boxY, boxW, boxH, 8, COL_PRIMARY);
    lcd.setTextSize(5);
    lcd.setTextColor(COL_TEXT, COL_DARK);
    centerText("Baud " + baudStr, boxY + (boxH - 40) / 2);
    lcd.setTextSize(3);
    lcd.setTextColor(COL_TEXT_ALT, COL_BG);
    centerText("Press any key in the terminal", boxY + boxH + 48);
}

void Tab5DeviceView::welcomeWeb(const std::string& ipStr) {
    banner("Open in a browser");
    auto& lcd = M5.Display;
    std::string url = "http://" + ipStr;
    int boxW = lcd.width() * 3 / 5;
    int boxH = 96;
    int boxX = (lcd.width() - boxW) / 2;
    int boxY = lcd.height() / 2 - boxH / 2;
    lcd.fillRoundRect(boxX, boxY, boxW, boxH, 8, COL_DARK);
    lcd.drawRoundRect(boxX, boxY, boxW, boxH, 8, COL_PRIMARY);
    lcd.setTextSize(5);
    lcd.setTextColor(COL_TEXT, COL_DARK);
    centerText(url, boxY + (boxH - 40) / 2);
}

void Tab5DeviceView::welcomeHotspot(const std::string& ipStr) {
    GlobalState& state = GlobalState::getInstance();
    PinoutConfig config;
    config.setMode("HOTSPOT");
    config.setMappings({
        state.getActiveApName(),
        std::string("PW ") + state.getApPassword(),
        std::string("IP ") + ipStr,
        "CONNECT TO AP"
    });
    show(config);
}

void Tab5DeviceView::show(PinoutConfig& config) {
    auto& lcd = M5.Display;
    clear();

    auto mode = config.getMode();
    const auto& mappings = config.getMappings();

    lcd.setTextSize(5);
    lcd.setTextColor(COL_TEXT, COL_BG);
    centerText(std::string("Mode: ") + mode, 30);

    if (mappings.empty()) {
        lcd.setTextSize(4);
        lcd.setTextColor(COL_TEXT_ALT, COL_BG);
        centerText("Nothing to display", lcd.height() / 2);
        return;
    }

    int cols = (mappings.size() > 6) ? 2 : 1;
    int rows = (static_cast<int>(mappings.size()) + cols - 1) / cols;
    int gap = 14;
    int gridTop = 120;
    int gridBot = lcd.height() - 30;
    int cellH = std::min(96, (gridBot - gridTop) / std::max(1, rows) - gap);
    int cellW = (lcd.width() - 40 - (cols - 1) * gap) / cols;

    lcd.setTextSize(3);
    for (size_t i = 0; i < mappings.size(); ++i) {
        int c = static_cast<int>(i) % cols;
        int r = static_cast<int>(i) / cols;
        int x = 20 + c * (cellW + gap);
        int y = gridTop + r * (cellH + gap);
        lcd.fillRoundRect(x, y, cellW, cellH, 8, COL_DARK);
        lcd.drawRoundRect(x, y, cellW, cellH, 8, COL_PRIMARY);
        lcd.setTextColor(COL_TEXT, COL_DARK);
        lcd.setCursor(x + (cellW - lcd.textWidth(mappings[i].c_str())) / 2, y + (cellH - 24) / 2);
        lcd.print(mappings[i].c_str());
    }
}

void Tab5DeviceView::horizontalSelection(
    const std::vector<std::string>& options,
    uint16_t selectedIndex,
    const std::string& description1,
    const std::string& description2) {

    auto& lcd = M5.Display;
    lcd.fillScreen(COL_BG);
    int W = lcd.width();
    int H = lcd.height();

    // Top description
    lcd.setTextSize(4);
    lcd.setTextColor(COL_PRIMARY, COL_BG);
    centerText(description1, H / 4);

    // Selected option card (centre third)
    int cardW = W / 3;
    int cardH = 130;
    int cardX = (W - cardW) / 2;
    int cardY = H / 2 - cardH / 2;
    lcd.fillRoundRect(cardX, cardY, cardW, cardH, 12, COL_DARK);
    lcd.drawRoundRect(cardX, cardY, cardW, cardH, 12, COL_LIGHT);

    const std::string& option = options[selectedIndex];
    lcd.setTextSize(6);
    lcd.setTextColor(COL_TEXT, COL_DARK);
    lcd.setCursor(cardX + (cardW - lcd.textWidth(option.c_str())) / 2, cardY + (cardH - 48) / 2);
    lcd.print(option.c_str());

    // Left / right arrows aligned with the touch thirds (see Tab5Input)
    lcd.setTextSize(8);
    lcd.setTextColor(COL_PRIMARY, COL_BG);
    lcd.setCursor(W / 6 - 24, H / 2 - 32);
    lcd.print("<");
    lcd.setCursor(W * 5 / 6 - 24, H / 2 - 32);
    lcd.print(">");

    // Bottom description
    lcd.setTextSize(3);
    lcd.setTextColor(COL_TEXT_ALT, COL_BG);
    centerText(description2, H * 3 / 4);

    // Touch hint
    lcd.setTextSize(2);
    lcd.setTextColor(COL_TEXT_ALT, COL_BG);
    centerText("Tap left / centre / right", H - 40);
}

void Tab5DeviceView::topBar(const std::string& title, bool submenu, bool searchBar) {
    auto& lcd = M5.Display;
    int barH = 64;
    lcd.fillRect(0, 0, lcd.width(), barH, COL_DARK);
    lcd.drawFastHLine(0, barH, lcd.width(), COL_PRIMARY);

    std::string text = title;
    if (searchBar) {
        text = title.empty() ? "Type to search" : title;
    }
    lcd.setTextSize(4);
    lcd.setTextColor(COL_TEXT, COL_DARK);
    centerText(text, (barH - 32) / 2);
}

void Tab5DeviceView::loading() {
    auto& lcd = M5.Display;
    clear();
    int bw = lcd.width() / 2;
    int bh = 120;
    int bx = (lcd.width() - bw) / 2;
    int by = (lcd.height() - bh) / 2;
    lcd.fillRoundRect(bx, by, bw, bh, 10, COL_DARK);
    lcd.drawRoundRect(bx, by, bw, bh, 10, COL_PRIMARY);
    lcd.setTextSize(5);
    lcd.setTextColor(COL_TEXT, COL_DARK);
    centerText("Loading...", by + (bh - 40) / 2);
}

void Tab5DeviceView::adapterMode(const std::string& adapterName, const std::string& description, const std::vector<std::string>& details) {
    auto& lcd = M5.Display;
    clear();

    lcd.setTextSize(5);
    lcd.setTextColor(COL_TEXT, COL_BG);
    centerText(adapterName, 40);

    int n = static_cast<int>(std::min<size_t>(details.size(), 8));
    int cols = 2;
    int gap = 18;
    int cellW = (lcd.width() - 60 - gap) / cols;
    int cellH = 60;
    int top = 150;
    lcd.setTextSize(3);
    for (int i = 0; i < n; ++i) {
        int c = i % cols;
        int r = i / cols;
        int x = 30 + c * (cellW + gap);
        int y = top + r * (cellH + gap);
        lcd.fillRoundRect(x, y, cellW, cellH, 8, COL_DARK);
        lcd.drawRoundRect(x, y, cellW, cellH, 8, COL_PRIMARY);
        lcd.setTextColor(COL_TEXT, COL_DARK);
        lcd.setCursor(x + (cellW - lcd.textWidth(details[i].c_str())) / 2, y + (cellH - 24) / 2);
        lcd.print(details[i].c_str());
    }

    lcd.setTextSize(3);
    lcd.setTextColor(COL_TEXT_ALT, COL_BG);
    centerText(description, lcd.height() - 120);
    centerText("Press any button to return", lcd.height() - 70);
}

void Tab5DeviceView::drawLogicTrace(uint8_t pin, const std::vector<uint8_t>& buffer, uint8_t step) {
    auto& lcd = M5.Display;
    int cw = lcd.width() - 40;
    int ch = 220;
    int midY = ch / 2;
    int amp = 70;

    M5Canvas canvas(&lcd);
    canvas.setPsram(true);
    canvas.setColorDepth(8);
    canvas.createSprite(cw, ch);
    canvas.fillSprite(COL_BG);

    int x0 = 0;
    for (size_t i = 1; i < buffer.size() && x0 < cw - step; ++i) {
        int y1 = buffer[i - 1] ? midY - amp : midY + amp;
        int y2 = buffer[i]     ? midY - amp : midY + amp;
        if (buffer[i] != buffer[i - 1]) {
            canvas.drawLine(x0, y1, x0 + step, y1, COL_PRIMARY);
            canvas.drawLine(x0 + step, y1, x0 + step, y2, COL_PRIMARY);
        } else {
            canvas.drawLine(x0, y1, x0 + step, y2, COL_PRIMARY);
        }
        x0 += step;
    }

    canvas.setTextColor(COL_TEXT);
    canvas.setTextSize(2);
    canvas.drawString("GPIO " + String(pin), 8, 6);
    canvas.pushSprite((lcd.width() - cw) / 2, (lcd.height() - ch) / 2);
    canvas.deleteSprite();
}

void Tab5DeviceView::drawAnalogicTrace(uint8_t pin, const std::vector<uint8_t>& buffer, uint8_t step) {
    auto& lcd = M5.Display;
    int cw = lcd.width() - 40;
    int ch = 220;

    M5Canvas canvas(&lcd);
    canvas.setPsram(true);
    canvas.setColorDepth(8);
    canvas.createSprite(cw, ch);
    canvas.fillSprite(COL_BG);

    int x = 0;
    for (size_t i = 1; i < buffer.size() && x < cw - step; ++i) {
        int prev = ch - 1 - (buffer[i - 1] * (ch - 1) / 255);
        int curr = ch - 1 - (buffer[i]     * (ch - 1) / 255);
        canvas.drawLine(x, prev, x + step, curr, COL_PRIMARY);
        x += step;
    }

    canvas.setTextColor(COL_TEXT);
    canvas.setTextSize(2);
    canvas.drawString("GPIO " + String(pin), 8, 6);
    canvas.pushSprite((lcd.width() - cw) / 2, (lcd.height() - ch) / 2);
    canvas.deleteSprite();
}

void Tab5DeviceView::drawWaterfall(
    const std::string& title,
    float startValue,
    float endValue,
    const char* unit,
    int rowIndex,
    int rowCount,
    int level) {

    auto& lcd = M5.Display;
    const int W = lcd.width();
    const int H = lcd.height();

    const int headerH = 24;
    const int footerH = 24;
    const int graphY = headerH;
    const int graphH = H - headerH - footerH;
    const int midX = W / 2;
    const int barMaxPixels = midX - 4;

    if (level < 0) level = 0;
    if (level > 100) level = 100;
    int barPixels = (level * barMaxPixels) / 100;

    if (rowIndex == 0) {
        lcd.fillScreen(COL_BG);
        lcd.setTextSize(2);
        lcd.setTextColor(COL_TEXT, COL_BG);
        lcd.setCursor(4, 4);
        lcd.printf("%s", title.c_str());

        char bufStart[24];
        char bufEnd[24];
        if (unit && unit[0]) {
            snprintf(bufStart, sizeof(bufStart), "%.2f%s", startValue, unit);
            snprintf(bufEnd, sizeof(bufEnd), "%.2f%s", endValue, unit);
        } else {
            snprintf(bufStart, sizeof(bufStart), "%.2f", startValue);
            snprintf(bufEnd, sizeof(bufEnd), "%.2f", endValue);
        }
        lcd.setCursor(W - lcd.textWidth(bufStart) - 4, 4);
        lcd.printf("%s", bufStart);
        lcd.setCursor(W - lcd.textWidth(bufEnd) - 4, H - footerH + 4);
        lcd.printf("%s", bufEnd);

        lcd.fillRect(0, graphY, W, graphH, COL_BG);
        lcd.drawFastVLine(midX, graphY, graphH, COL_DARK);
    }

    if (rowCount <= 1) return;
    if (rowIndex < 0) rowIndex = 0;
    if (rowIndex > rowCount - 1) rowIndex = rowCount - 1;

    int y = graphY + (int)((int64_t)rowIndex * (graphH - 1) / (rowCount - 1));
    lcd.drawFastHLine(0, y, W, COL_BG);
    lcd.drawPixel(midX, y, COL_DARK);

    if (barPixels > 0) {
        int x0 = midX - barPixels;
        int w = barPixels * 2;
        if (x0 < 0) { w += x0; x0 = 0; }
        if (x0 + w > W) w = W - x0;
        if (w > 0) {
            lcd.drawFastHLine(x0, y, w, COL_PRIMARY);
        }
    }
}

#endif
