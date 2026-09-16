#if defined(DEVICE_TAB5)

#include "Tab5Input.h"
#include "Boards/Tab5/Tab5Keyboard.h"

Tab5Input::Tab5Input(bool pollKeyboard) : pollKeyboard(pollKeyboard) {}

char Tab5Input::mapTouch() {
    M5.update();

    auto detail = M5.Touch.getDetail();
    if (!detail.wasPressed()) {
        return KEY_NONE;
    }

    int width = M5.Display.width();
    if (width <= 0) {
        width = 720;
    }

    if (detail.x < width / 3) {
        return KEY_ARROW_LEFT;
    }
    if (detail.x > (2 * width) / 3) {
        return KEY_ARROW_RIGHT;
    }
    return KEY_OK;
}

char Tab5Input::readChar() {
    // Prefer the A164 hardware keyboard if present; fall back to touch zones.
    if (pollKeyboard) {
        char k = Tab5Keyboard::instance().readChar();
        if (k != KEY_NONE) {
            return k;
        }
    }
    return mapTouch();
}

char Tab5Input::handler() {
    char c = KEY_NONE;
    while ((c = readChar()) == KEY_NONE) {
        delay(10);
    }
    return c;
}

void Tab5Input::waitPress(uint32_t timeoutMs) {
    uint32_t start = millis();
    for (;;) {
        if (readChar() != KEY_NONE) {
            return;
        }
        if (timeoutMs > 0 && (millis() - start) >= timeoutMs) {
            return;
        }
        delay(5);
    }
}

#endif
