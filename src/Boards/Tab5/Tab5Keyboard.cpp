#if defined(DEVICE_TAB5)

#include "Boards/Tab5/Tab5Keyboard.h"
#include "Data/InputKeys.h"
#include "driver/i2c_master.h"

namespace {

constexpr uint8_t   A164_ADDR     = 0x6D;
constexpr gpio_num_t A164_SDA     = GPIO_NUM_0;   // Ext.Port1 dedicated bus
constexpr gpio_num_t A164_SCL     = GPIO_NUM_1;
constexpr uint32_t  A164_FREQ     = 400000;
constexpr int       A164_XFER_MS  = 20;

constexpr uint8_t   REG_SYS       = 0x00; // [2] = queued event count
constexpr uint8_t   REG_MODE      = 0x10; // 0=Normal 1=HID 2=Char
constexpr uint8_t   REG_HID_EVENT = 0x30; // {modifier, keycode}
constexpr uint8_t   REG_VERSION   = 0xF0;
constexpr uint8_t   MODE_HID      = 0x01;

// M5Unified claims the two HP I2C controllers (internal PMIC/touch bus and the
// Grove/Ext bus). Use the ESP-IDF i2c_master driver with i2c_port = -1 so it
// auto-allocates a free controller for the keyboard on GPIO0/1 -- this is what
// the working M5MonsterC5-Tab5 A164 driver does. (Arduino Wire1 on these pins
// fails with ESP_ERR_INVALID_STATE.)
i2c_master_bus_handle_t s_bus = nullptr;
i2c_master_dev_handle_t s_dev = nullptr;

// USB-HID usage code 0x00..0x38 -> {unshifted, shifted} ASCII.
const char KC2ASCII[0x39][2] = {
    {0,0},{0,0},{0,0},{0,0},
    {'a','A'},{'b','B'},{'c','C'},{'d','D'},{'e','E'},{'f','F'},{'g','G'},{'h','H'},
    {'i','I'},{'j','J'},{'k','K'},{'l','L'},{'m','M'},{'n','N'},{'o','O'},{'p','P'},
    {'q','Q'},{'r','R'},{'s','S'},{'t','T'},{'u','U'},{'v','V'},{'w','W'},{'x','X'},
    {'y','Y'},{'z','Z'},
    {'1','!'},{'2','@'},{'3','#'},{'4','$'},{'5','%'},{'6','^'},{'7','&'},{'8','*'},
    {'9','('},{'0',')'},
    {0,0},        // 0x28 Enter (handled below)
    {0,0},        // 0x29 Esc
    {0,0},        // 0x2A Backspace (handled below)
    {0,0},        // 0x2B Tab (handled below)
    {' ',' '},    // 0x2C Space
    {'-','_'},{'=','+'},{'[','{'},{']','}'},{'\\','|'},{'\\','|'}, // 0x2D..0x32
    {';',':'},{'\'','"'},{'`','~'},{',','<'},{'.','>'},{'/','?'},   // 0x33..0x38
};

bool readReg(uint8_t reg, uint8_t* buf, size_t n) {
    if (!s_dev) return false;
    return i2c_master_transmit_receive(s_dev, &reg, 1, buf, n, A164_XFER_MS) == ESP_OK;
}

void writeReg(uint8_t reg, uint8_t val) {
    if (!s_dev) return;
    uint8_t d[2] = { reg, val };
    i2c_master_transmit(s_dev, d, sizeof(d), A164_XFER_MS);
}

}  // namespace

Tab5Keyboard& Tab5Keyboard::instance() {
    static Tab5Keyboard kbd;
    return kbd;
}

void Tab5Keyboard::begin() {
    if (started) return;
    started = true;

    i2c_master_bus_config_t bus_cfg = {};
    bus_cfg.i2c_port = -1;  // auto-select a free controller
    bus_cfg.sda_io_num = A164_SDA;
    bus_cfg.scl_io_num = A164_SCL;
    bus_cfg.clk_source = I2C_CLK_SRC_DEFAULT;
    bus_cfg.glitch_ignore_cnt = 7;
    bus_cfg.flags.enable_internal_pullup = true;

    if (i2c_new_master_bus(&bus_cfg, &s_bus) != ESP_OK) {
        present = false;
        return;
    }

    i2c_device_config_t dev_cfg = {};
    dev_cfg.dev_addr_length = I2C_ADDR_BIT_LEN_7;
    dev_cfg.device_address = A164_ADDR;
    dev_cfg.scl_speed_hz = A164_FREQ;

    if (i2c_master_bus_add_device(s_bus, &dev_cfg, &s_dev) != ESP_OK) {
        present = false;
        return;
    }

    uint8_t ver = 0;
    present = readReg(REG_VERSION, &ver, 1);
    if (present) {
        writeReg(REG_MODE, MODE_HID);
    }
}

char Tab5Keyboard::readChar() {
    if (!started) begin();
    if (!present) return KEY_NONE;

    uint8_t sys[4] = {0};
    if (!readReg(REG_SYS, sys, 4)) return KEY_NONE;
    uint8_t events = sys[2];
    if (events == 0) return KEY_NONE;  // nothing queued

    uint8_t hid[2] = {0xFF, 0xFF};
    if (!readReg(REG_HID_EVENT, hid, 2)) return KEY_NONE;

    uint8_t modifier = hid[0];
    uint8_t keycode  = hid[1];
    if (keycode == 0x00 || keycode == 0xFF) return KEY_NONE;  // release / empty

    bool shift = (modifier & 0x22) != 0;  // LSHIFT | RSHIFT

    switch (keycode) {
        case 0x28: case 0x58: return KEY_OK;               // Enter / Keypad Enter
        case 0x2A:            return KEY_DEL;              // Backspace
        case 0x2B:            return KEY_TAB_CUSTOM;       // Tab
        case 0x4F:            return KEY_ARROW_RIGHT;      // Right
        case 0x50:            return KEY_ARROW_LEFT;       // Left
        case 0x51:            return CARDPUTER_SPECIAL_ARROW_DOWN;  // Down -> terminal scroll
        case 0x52:            return CARDPUTER_SPECIAL_ARROW_UP;    // Up   -> terminal scroll
        default:              break;
    }

    if (keycode <= 0x38) {
        char c = KC2ASCII[keycode][shift ? 1 : 0];
        if (c != 0) return c;
    }
    return KEY_NONE;
}

#endif
