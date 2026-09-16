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
    Tab5Input();

    char handler() override;   // blocking read
    char readChar() override;  // non-blocking read
    void waitPress(uint32_t timeoutMs) override;

private:
    char mapTouch();
};

#endif
