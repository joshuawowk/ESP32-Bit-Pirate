#if defined(DEVICE_TAB5)

#include "Boards/Tab5/Tab5TerminalView.h"

namespace {
constexpr uint16_t COL_BG   = 0x0000;  // black
constexpr uint16_t COL_TEXT = 0x07E0;  // green (classic terminal)
}

void Tab5TerminalView::initialize() {
    // M5.begin() was already called by Tab5Board; just take over the screen.
    M5.Display.setRotation(3);
    M5.Display.fillScreen(COL_BG);
    M5.Display.setTextColor(COL_TEXT);
    M5.Display.setTextWrap(false);
    M5.Display.setTextSize(textSize);

    scrW = M5.Display.width();
    scrH = M5.Display.height();

    // 1 bpp sprite in PSRAM (1280x720 -> ~115 KB)
    termSprite.setPsram(true);
    termSprite.setColorDepth(1);
    spriteReady = termSprite.createSprite(scrW, scrH);
    if (spriteReady) {
        termSprite.setPaletteColor(0, COL_BG);
        termSprite.setPaletteColor(1, COL_TEXT);
        termSprite.setTextWrap(false);
        termSprite.setTextSize(textSize);
    }

    originX = 0;
    originY = 0;

    recomputeMetrics();
    termReset();
    renderAll();
}

void Tab5TerminalView::welcome(TerminalTypeEnum& /*terminalType*/, std::string& /*terminalInfos*/) {
    clear();
    println("  ____  _ _     ____  _           _       ");
    println(" | __ )(_) |_  |  _ \\(_)_ __ __ _| |_ ___ ");
    println(" |  _ \\| | __| | |_) | | '__/ _` | __/ _ \\");
    println(" | |_) | | |_  |  __/| | | | (_| | ||  __/");
    println(" |____/|_|\\__| |_|   |_|_|  \\__,_|\\__\\___|");
    println("");
    println(" Standalone - A164 keyboard. Arrows scroll.");
    println(" Type 'mode' to start or 'help' for commands.");
    println("");
}

void Tab5TerminalView::print(const std::string& text) {
    if (text.empty()) { maybeRender(); return; }

    bool sawScroll = false;
    auto decoded = htmlDecodeBasic(text);

    std::string filtered;
    filtered.reserve(decoded.size());
    for (unsigned char b : decoded) {
        if (b == (unsigned char)CARDPUTER_SPECIAL_ARROW_UP) {
            int maxScroll = (int)history.size();
            if (scrollOffset < maxScroll) scrollOffset++;
            sawScroll = true;
            continue;
        }
        if (b == (unsigned char)CARDPUTER_SPECIAL_ARROW_DOWN) {
            if (scrollOffset > 0) scrollOffset--;
            sawScroll = true;
            continue;
        }
        if (scrollOffset > 0) { scrollOffset = 0; sawScroll = true; }
        filtered.push_back((char)b);
    }

    for (unsigned char b : filtered) feedFilteredByte(b);

    dirty = true;
    char last = filtered.empty() ? '\0' : filtered.back();
    if (sawScroll || instantRender || last == '\n' || last == '\r' || last == ' ') {
        renderAll();
        lastRenderMs = millis();
        dirty = false;
        instantRender = false;
    } else {
        maybeRender();
    }
}

void Tab5TerminalView::print(const uint8_t data) {
    feedFilteredByte(data);
    if (data == '\n' || data == '\r' || data == ' ' || instantRender) {
        renderAll();
        lastRenderMs = millis();
        dirty = false;
        instantRender = false;
    } else {
        dirty = true;
        maybeRender();
    }
}

void Tab5TerminalView::println(const std::string& text) {
    auto decoded = htmlDecodeBasic(text);
    feedFilteredBytes((const uint8_t*)decoded.data(), decoded.size());
    feedFilteredByte('\n');
    renderAll();
    lastRenderMs = millis();
    dirty = false;
}

void Tab5TerminalView::printPrompt(const std::string& mode) {
    instantRender = true;
    print(mode + "> ");
}

void Tab5TerminalView::waitPress() {
    print("\nPress any key to continue...\n");
}

void Tab5TerminalView::clear() {
    if (spriteReady) termSprite.fillScreen(0);
    M5.Display.fillScreen(COL_BG);
    u8_cp = 0; u8_rem = 0;
    history.clear();
    scrollOffset = 0;
    termReset();
    renderAll();
}

// --------------------- Terminal core ---------------------

void Tab5TerminalView::termReset() {
    lines.assign(rows, std::string(cols, ' '));
    curRow = 0;
    curCol = 0;
    ansiReset();
}

void Tab5TerminalView::termPutChar(char c) {
    if (c >= ' ') {
        lines[curRow][curCol] = c;
        curCol++;
        if (curCol >= cols) {
            curCol = 0;
            curRow++;
            if (curRow >= rows) {
                termScrollUp();
                curRow = rows - 1;
            }
        }
    }
}

void Tab5TerminalView::termPutText(const char* s, size_t n) {
    for (size_t i = 0; i < n; ++i) termPutChar(s[i]);
}

void Tab5TerminalView::termNewLine() {
    curCol = 0;
    curRow++;
    if (curRow >= rows) {
        termScrollUp();
        curRow = rows - 1;
    }
}

void Tab5TerminalView::termCarriageReturn() {
    curCol = 0;
}

void Tab5TerminalView::termBackspace() {
    if (curCol > 0) {
        curCol--;
        lines[curRow][curCol] = ' ';
    }
}

void Tab5TerminalView::termScrollUp() {
    history.push_back(std::move(lines[0]));
    if ((int)history.size() > (int)historyMax) history.pop_front();
    for (int r = 1; r < rows; ++r) lines[r - 1] = std::move(lines[r]);
    lines[rows - 1].assign(cols, ' ');
    if (scrollOffset > 0) scrollOffset++;
}

void Tab5TerminalView::termEraseInLine(int mode) {
    if (mode == 0) {
        for (int c = curCol; c < cols; ++c) lines[curRow][c] = ' ';
    } else if (mode == 1) {
        for (int c = 0; c <= curCol; ++c) lines[curRow][c] = ' ';
    } else if (mode == 2) {
        lines[curRow].assign(cols, ' ');
    }
}

void Tab5TerminalView::termEraseInDisplay(int mode) {
    if (mode == 2) {
        for (int r = 0; r < rows; ++r) lines[r].assign(cols, ' ');
        curRow = 0; curCol = 0;
        return;
    }
    if (mode == 0) {
        termEraseInLine(0);
        for (int r = curRow + 1; r < rows; ++r) lines[r].assign(cols, ' ');
    } else if (mode == 1) {
        for (int r = 0; r < curRow; ++r) lines[r].assign(cols, ' ');
        termEraseInLine(1);
    }
}

void Tab5TerminalView::termMoveCursorRel(int dx, int dy) {
    curRow += dy;
    curCol += dx;
    if (curRow < 0) curRow = 0;
    if (curRow >= rows) curRow = rows - 1;
    if (curCol < 0) curCol = 0;
    if (curCol >= cols) curCol = cols - 1;
}

void Tab5TerminalView::termMoveCursorAbs(int row1, int col1) {
    int r = row1 - 1;
    int c = col1 - 1;
    if (r < 0) r = 0; if (r >= rows) r = rows - 1;
    if (c < 0) c = 0; if (c >= cols) c = cols - 1;
    curRow = r;
    curCol = c;
}

// --------------------- ANSI parser ---------------------

void Tab5TerminalView::ansiReset() {
    inEsc = false;
    inCSI = false;
    csiParams.clear();
    csiParamAcc = -1;
}

void Tab5TerminalView::ansiFinalizeCSI(char final) {
    auto get = [&](size_t i, int def1) -> int {
        if (i < csiParams.size() && csiParams[i] >= 0) return csiParams[i] == 0 && def1 ? 1 : csiParams[i];
        return def1 ? 1 : 0;
    };

    switch (final) {
        case 'A': termMoveCursorRel(0, -get(0, 1)); break;
        case 'B': termMoveCursorRel(0,  get(0, 1)); break;
        case 'C': termMoveCursorRel( get(0, 1), 0); break;
        case 'D': termMoveCursorRel(-get(0, 1), 0); break;
        case 'K': { int mode = get(0, 0); termEraseInLine(mode); } break;
        case 'J': { int mode = get(0, 0); termEraseInDisplay(mode); } break;
        case 'H':
        case 'f': { int r = get(0, 1); int c = get(1, 1); termMoveCursorAbs(r, c); } break;
        default: break;
    }
}

void Tab5TerminalView::ansiFeed(char c) {
    if (!inEsc) {
        switch (c) {
            case '\r': termCarriageReturn(); return;
            case '\n': termNewLine(); return;
            case '\b': termBackspace(); return;
            case '\t': { int spaces = 4 - (curCol % 4); while (spaces--) termPutChar(' '); return; }
            case 0x1B: inEsc = true; return;
            default:
                if (c >= ' ') termPutChar(c);
                padBeforeErase = (c == ' ');
                return;
        }
    } else {
        if (!inCSI) {
            if (c == '[') {
                inCSI = true; csiParams.clear(); csiParamAcc = -1; return;
            } else {
                ansiReset(); return;
            }
        } else {
            if (c >= '0' && c <= '9') {
                int d = c - '0';
                if (csiParamAcc < 0) csiParamAcc = d;
                else csiParamAcc = csiParamAcc * 10 + d;
                return;
            } else if (c == ';') {
                csiParams.push_back(csiParamAcc < 0 ? 0 : csiParamAcc);
                csiParamAcc = -1;
                return;
            } else {
                padBeforeErase = false;
                csiParams.push_back(csiParamAcc < 0 ? 0 : csiParamAcc);
                ansiFinalizeCSI(c);
                ansiReset();
                return;
            }
        }
    }
}

// --------------------- Rendering ---------------------

void Tab5TerminalView::drawLine(const std::string& s, int16_t y, bool keepTrailingSpaces) {
    size_t end = s.size();
    if (!keepTrailingSpaces) {
        while (end > 0 && s[end - 1] == ' ') --end;
    }
    if (spriteReady) {
        termSprite.setCursor(originX, y);
        termSprite.printf("%.*s", (int)end, s.c_str());
    } else {
        M5.Display.setCursor(originX, y);
        M5.Display.printf("%.*s", (int)end, s.c_str());
    }
}

void Tab5TerminalView::renderAll() {
    if (spriteReady) {
        termSprite.fillScreen(0);
        termSprite.setTextColor(1);
        termSprite.setTextSize(textSize);
    } else {
        M5.Display.fillScreen(COL_BG);
        M5.Display.setTextColor(COL_TEXT);
        M5.Display.setTextSize(textSize);
    }

    const int H = (int)history.size();
    if (scrollOffset > H) scrollOffset = H;

    const int total = H + rows;
    int endIdx = total - 1 - scrollOffset;
    if (endIdx < 0) endIdx = 0;
    int startIdx = endIdx - (rows - 1);
    if (startIdx < 0) startIdx = 0;

    int16_t y = originY;
    for (int i = 0; i < rows; ++i) {
        int idx = startIdx + i;
        bool isActiveBufferLine =
            (scrollOffset == 0) && (idx >= H) && ((idx - H) == curRow);
        if (idx < H) {
            drawLine(history[idx], y, false);
        } else {
            int li = idx - H;
            if (li >= 0 && li < rows) {
                drawLine(lines[li], y, isActiveBufferLine);
            }
        }
        y += charH;
    }

    if (scrollOffset == 0) {
        int16_t cx = originX + curCol * charW;
        int16_t cy = originY + curRow * charH + charH - 3;
        if (spriteReady) termSprite.fillRect(cx, cy, charW, 3, 1);
        else M5.Display.fillRect(cx, cy, charW, 3, COL_TEXT);
    }

    if (spriteReady) termSprite.pushSprite(0, 0);
}

void Tab5TerminalView::maybeRender() {
    if (!dirty) return;
    uint32_t now = millis();
    if (now - lastRenderMs >= frameIntervalMs) {
        renderAll();
        lastRenderMs = now;
        dirty = false;
    }
}

void Tab5TerminalView::recomputeMetrics() {
    charW = 6 * textSize;       // GFX base glyph is 6px wide at size 1
    charH = 8 * textSize + 4;   // 8px tall + line spacing

    int16_t usableW = scrW - 2 * padX;
    int16_t usableH = scrH - 2 * padY;
    if (usableW < charW) usableW = charW;
    if (usableH < charH) usableH = charH;

    cols = usableW / charW; if (cols > 80) cols = 80; if (cols < 20) cols = 20;
    rows = usableH / charH; if (rows < 4) rows = 4;

    int16_t leftover = usableH - rows * charH;

    originX = padX;
    originY = padY + leftover / 2;

    lines.assign(rows, std::string(cols, ' '));
    curRow = 0; curCol = 0;
}

// --------------------- UTF-8 / HTML filters ---------------------

void Tab5TerminalView::feedFilteredBytes(const uint8_t* data, size_t n) {
    for (size_t i = 0; i < n; ++i) feedFilteredByte(data[i]);
}

void Tab5TerminalView::feedFilteredByte(uint8_t b) {
    if (u8_rem == 0) {
        if (b < 0x80) {
            ansiFeed(static_cast<char>(b));
        } else if ((b & 0xE0) == 0xC0) {
            u8_cp = (b & 0x1F); u8_rem = 1;
        } else if ((b & 0xF0) == 0xE0) {
            u8_cp = (b & 0x0F); u8_rem = 2;
        } else if ((b & 0xF8) == 0xF0) {
            u8_cp = (b & 0x07); u8_rem = 3;
        }
    } else {
        if ((b & 0xC0) == 0x80) {
            u8_cp = (u8_cp << 6) | (b & 0x3F);
            if (--u8_rem == 0) { emitCodepoint(u8_cp); u8_cp = 0; }
        } else {
            u8_cp = 0; u8_rem = 0;
            feedFilteredByte(b);
        }
    }
}

void Tab5TerminalView::emitCodepoint(uint32_t cp) {
    std::string repl = mapCodepointToASCII(cp);
    for (char ch : repl) ansiFeed(ch);
}

std::string Tab5TerminalView::mapCodepointToASCII(uint32_t cp) const {
    if (cp <= 0x7E) {
        if (cp >= 0x20) return std::string(1, static_cast<char>(cp));
        return "";
    }
    if (cp == 0x00A0) return " ";
    if (cp == 0x2018 || cp == 0x2019) return "'";
    if (cp == 0x201C || cp == 0x201D) return "\"";
    if (cp == 0x2013 || cp == 0x2014 || cp == 0x2212) return "-";
    if (cp == 0x2026) return "...";
    if (cp == 0x2022) return "*";
    if (cp == 0x2190) return "<-";
    if (cp == 0x2192) return "->";
    if (cp == 0x2191) return "^";
    if (cp == 0x2193) return "v";
    if (cp == 0x2713 || cp == 0x2705) return "v";
    if (cp == 0x2717 || cp == 0x274C) return "x";
    if (cp == 0x00B0) return " deg ";
    if (cp == 0x03BC || cp == 0x00B5) return "u";
    return "";
}

std::string Tab5TerminalView::htmlDecodeBasic(const std::string& s) const {
    std::string out; out.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i) {
        char ch = s[i];
        if (ch == '&') {
            size_t j = s.find(';', i + 1);
            if (j != std::string::npos && j - i <= 10) {
                std::string ent = s.substr(i + 1, j - (i + 1));
                if (ent == "amp")  { out.push_back('&'); i = j; continue; }
                if (ent == "lt")   { out.push_back('<'); i = j; continue; }
                if (ent == "gt")   { out.push_back('>'); i = j; continue; }
                if (ent == "quot") { out.push_back('"'); i = j; continue; }
                if (ent == "apos") { out.push_back('\''); i = j; continue; }
                if (ent == "nbsp") { out.push_back(' '); i = j; continue; }
                if (!ent.empty() && ent[0] == '#') {
                    uint32_t cp = 0; bool ok = false;
                    if (ent.size() > 1 && (ent[1] == 'x' || ent[1] == 'X')) {
                        try { cp = static_cast<uint32_t>(std::stoul(ent.substr(2), nullptr, 16)); ok = true; }
                        catch (...) { ok = false; }
                    } else {
                        try { cp = static_cast<uint32_t>(std::stoul(ent.substr(1), nullptr, 10)); ok = true; }
                        catch (...) { ok = false; }
                    }
                    if (ok) { out += mapCodepointToASCII(cp); i = j; continue; }
                }
            }
        }
        out.push_back(ch);
    }
    return out;
}

#endif
