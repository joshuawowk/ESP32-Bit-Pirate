#pragma once

// -----------------------------------------------------------------------------
// Board / SoC capability flags
//
// Derived from the ESP-IDF SoC capability header and the active Arduino USB mode
// so that radio- and USB-dependent services can degrade gracefully on SoCs that
// lack a given peripheral (e.g. the ESP32-P4 on the M5Stack Tab5, which has no
// native 2.4 GHz radio and reaches Wi-Fi through a remote ESP32-C6 over SDIO).
//
// Reachable from anywhere via the -Isrc/Boards/Common include root.
// -----------------------------------------------------------------------------

#include "soc/soc_caps.h"

// --- Native Bluetooth / BLE controller on the main SoC ---
// S3: SOC_BT_SUPPORTED == 1. P4: absent/0 (no on-die radio). The Arduino BLE
// library guards its own classes on SOC_BT_SUPPORTED, so on the P4 the BLE
// types disappear and BluetoothService must fall back to a no-op stub.
#if defined(SOC_BT_SUPPORTED) && SOC_BT_SUPPORTED
  #define HAS_NATIVE_BLE 1
#else
  #define HAS_NATIVE_BLE 0
#endif

// --- Arduino TinyUSB device stack (USB HID / MSC "adapter" mode) ---
// The Arduino USB device classes (USBHIDKeyboard, USBMSC, ...) are only compiled
// when the OTG peripheral is present AND the sketch runs in TinyUSB device mode
// (ARDUINO_USB_MODE == 0). The Tab5 board ships ARDUINO_USB_MODE == 1
// (HWCDC / USB-Serial-JTAG), so those classes are absent -> UsbS3Service stubs.
#if defined(SOC_USB_OTG_SUPPORTED) && SOC_USB_OTG_SUPPORTED && defined(ARDUINO_USB_MODE) && (ARDUINO_USB_MODE == 0)
  #define HAS_USB_STACK 1
#else
  #define HAS_USB_STACK 0
#endif

// --- Native Wi-Fi radio on the main SoC ---
// S3: local radio (SOC_WIFI_SUPPORTED == 1) -> promiscuous / raw-injection /
// deauth are possible. P4: Wi-Fi is remote on the ESP32-C6 (esp_hosted), which
// cannot do monitor mode / raw TX, so those features are runtime no-ops. Wi-Fi
// station/AP/TCP still work through WiFi.h. Used to hide radio-only UI.
#if defined(SOC_WIFI_SUPPORTED) && SOC_WIFI_SUPPORTED
  #define HAS_NATIVE_WIFI_RADIO 1
#else
  #define HAS_NATIVE_WIFI_RADIO 0
#endif
