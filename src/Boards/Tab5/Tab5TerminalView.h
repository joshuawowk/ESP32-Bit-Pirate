#pragma once

#if defined(DEVICE_TAB5)

#include <string>
#include <vector>
#include <deque>
#include <stdint.h>

#include <M5Unified.h>
#include <Enums/TerminalTypeEnum.h>
#include "Interfaces/ITerminalView.h"
#include "Data/InputKeys.h"
#include "States/GlobalState.h"

// On-screen scrolling terminal for the Tab5 standalone mode: an ANSI terminal
// emulator (adapted from CardputerTerminalView) rendering the Bit Pirate CLI on
// the 1280x720 MIPI-DSI panel. Paired with Tab5KeyboardInput (A164 keyboard).
class Tab5TerminalView : public ITerminalView {
public:
    ~Tab5TerminalView() override = default;

    void initialize() override;
    void welcome(TerminalTypeEnum& terminalType, std::string& terminalInfos) override;
    void print(const std::string& text) override;
    void print(const uint8_t data) override;
    void println(const std::string& text) override;
    void printPrompt(const std::string& mode = "HIZ") override;
    void waitPress() override;
    void clear() override;

private:
    // Terminal emulation
    void termReset();
    void termPutChar(char c);
    void termPutText(const char* s, size_t n);
    void termNewLine();
    void termCarriageReturn();
    void termBackspace();
    void termScrollUp();
    void termEraseInLine(int mode);
    void termEraseInDisplay(int mode);
    void termMoveCursorRel(int dx, int dy);
    void termMoveCursorAbs(int row1, int col1);

    // ANSI parser
    void ansiFeed(char c);
    void ansiReset();
    void ansiFinalizeCSI(char final);

    // Rendering
    void drawLine(const std::string& s, int16_t y, bool keepTrailingSpaces);
    void renderAll();
    void maybeRender();
    void recomputeMetrics();
    std::string htmlDecodeBasic(const std::string& s) const;
    std::string mapCodepointToASCII(uint32_t cp) const;
    void emitCodepoint(uint32_t cp);
    void feedFilteredByte(uint8_t data);
    void feedFilteredBytes(const uint8_t* data, size_t n);

private:
    // Screen buffer
    std::vector<std::string> lines;
    int rows = 30;
    int cols = 80;

    // Cursor
    int curRow = 0;
    int curCol = 0;

    // Parser state
    bool inEsc = false;
    bool inCSI = false;
    std::vector<int> csiParams;
    int csiParamAcc = -1;

    // Layout (pixels)
    int16_t originX = 0, originY = 0;
    int16_t charW = 12, charH = 20;
    int16_t scrW = 0, scrH = 0;
    uint8_t textSize = 2;

    // Double-buffer sprite
    LGFX_Sprite termSprite{ &M5.Display };
    bool spriteReady = false;

    // UTF-8 decode state
    uint32_t u8_cp = 0;
    int      u8_rem = 0;

    int16_t padX = 8;
    int16_t padY = 6;

    uint32_t lastRenderMs = 0;
    uint32_t frameIntervalMs = 16;
    bool     dirty = false;
    bool     instantRender = false;

    // Scrollback
    std::deque<std::string> history;
    size_t historyMax = 512;
    int scrollOffset = 0;
    bool padBeforeErase = false;
};

#endif
