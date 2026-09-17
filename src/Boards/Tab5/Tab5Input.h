#pragma once

#if defined(DEVICE_TAB5)

#include <M5Unified.h>
#include <Arduino.h>
#include "Interfaces/IInput.h"
#include "Data/InputKeys.h"

// The Tab5 has no physical A/B/PWR buttons like the StickS3, so navigation for
// the boot-time selectors comes from the capacitive touch panel (GT911):
//   left third  -> KEY_ARROW_LEFT
//   right third -> KEY_ARROW_RIGHT
//   center      -> KEY_OK
class Tab5Input : public IInput {
public:
    // pollKeyboard=false gives a touch-only input (used as the standalone-mode
    // device input, where the A164 is the terminal input and must not be double-
    // read here -- and where a GPIO0-based input would clobber the keyboard bus).
    explicit Tab5Input(bool pollKeyboard = true);

    char handler() override;   // blocking read
    char readChar() override;  // non-blocking read
    void waitPress(uint32_t timeoutMs) override;

private:
    char mapTouch();
    bool pollKeyboard;
};

#endif
