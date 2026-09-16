#pragma once

#if defined(DEVICE_TAB5)

#include <Arduino.h>
#include "Interfaces/IInput.h"
#include "Data/InputKeys.h"

// Terminal input for the Tab5 standalone mode: feeds typed characters from the
// A164 hardware keyboard into the CLI. Shares the Tab5Keyboard singleton with
// the board's touch input (only one is polled at a time per terminal type).
class Tab5KeyboardInput : public IInput {
public:
    Tab5KeyboardInput();

    char handler() override;   // blocking
    char readChar() override;  // non-blocking
    void waitPress(uint32_t timeoutMs) override;
};

#endif
