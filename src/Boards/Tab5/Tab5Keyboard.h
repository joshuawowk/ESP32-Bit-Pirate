#pragma once

#if defined(DEVICE_TAB5)

#include <Arduino.h>

// Reader for the M5Stack "Keyboard for Tab5" (A164): a 70-key STM32-based
// I2C keyboard at 0x6D on its own bus (SDA=GPIO0, SCL=GPIO1) via Ext.Port1.
// Runs the keyboard in HID mode and maps USB-HID usage codes to the Bit Pirate
// input sentinels (KEY_OK / KEY_DEL / KEY_ARROW_* / printable chars).
//
// Singleton so the touch input (Tab5Input) and the standalone terminal input
// (Tab5KeyboardInput) share one I2C bus init and one event queue.
//
// The A164 firmware is edge-driven: it queues one HID event on press
// ({modifier, keycode}) and one on release ({modifier, 0}), and never repeats a
// held key. Key repeat for the terminal-scroll arrows is therefore synthesized
// here, clocked off the press/release pair (see takePendingScroll).
class Tab5Keyboard {
public:
    static Tab5Keyboard& instance();

    void begin();                 // idempotent: init the bus, probe 0x6D, select HID mode
    bool isPresent() const { return present; }
    char readChar();              // KEY_NONE if nothing queued, else a mapped char

    // Hand over the scroll repeats that are already due but not yet returned by
    // readChar(), so the caller can apply them in one go. Returns 0 unless
    // `sentinel` is the scroll key currently held. Used by Tab5TerminalView to
    // scroll a whole burst per redraw: a full-screen redraw on the 1280x720
    // panel costs tens of ms, so one row per redraw would cap the scroll rate
    // at whatever the panel can push. Jumping several rows keeps the rate
    // wall-clock accurate instead of render-bound.
    int takePendingScroll(char sentinel);

private:
    Tab5Keyboard() = default;

    char pollEvent();             // drain at most one HID event, updating hold state
    char nextRepeat();            // synthesized repeat for a held scroll arrow
    void startRepeat(char sentinel);
    void stopRepeat();

    bool started = false;
    bool present = false;

    // Auto-repeat state for the terminal-scroll arrows.
    char     repeatChar     = 0;  // KEY_NONE == nothing held
    uint32_t holdStartMs    = 0;  // when the held key went down
    uint32_t nextRepeatMs   = 0;  // when the next repeat is due
    int      pendingRepeats = 0;  // repeats due but not yet handed out
};

#endif
