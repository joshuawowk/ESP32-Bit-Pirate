#if defined(DEVICE_TAB5)

#include "Boards/Tab5/Tab5KeyboardInput.h"
#include "Boards/Tab5/Tab5Keyboard.h"

Tab5KeyboardInput::Tab5KeyboardInput() {
    Tab5Keyboard::instance().begin();
}

char Tab5KeyboardInput::readChar() {
    return Tab5Keyboard::instance().readChar();
}

char Tab5KeyboardInput::handler() {
    char c = KEY_NONE;
    while ((c = Tab5Keyboard::instance().readChar()) == KEY_NONE) {
        delay(5);
    }
    return c;
}

void Tab5KeyboardInput::waitPress(uint32_t timeoutMs) {
    uint32_t start = millis();
    for (;;) {
        if (Tab5Keyboard::instance().readChar() != KEY_NONE) {
            return;
        }
        if (timeoutMs > 0 && (millis() - start) >= timeoutMs) {
            return;
        }
        delay(5);
    }
}

#endif
