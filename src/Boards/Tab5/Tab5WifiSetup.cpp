#ifdef DEVICE_TAB5

#include "Boards/Tab5/Tab5WifiSetup.h"

#include <Preferences.h>
#include <WiFi.h>
#include <M5Unified.h>

namespace {

constexpr const char* NVS_SSID_KEY = "ssid";
constexpr const char* NVS_PASS_KEY = "pass";

// Kept in an anonymous namespace so these helpers never collide with the
// identically-named globals in StickWifiSetup.cpp / DefaultWifiSetup.cpp.
bool loadWifiCredentials(String& ssid, String& password) {
    Preferences preferences;
    preferences.begin("wifi_settings", true);  // readonly
    ssid = preferences.getString(NVS_SSID_KEY, "");
    password = preferences.getString(NVS_PASS_KEY, "");
    preferences.end();
    return !ssid.isEmpty() && !password.isEmpty();
}

void showWifiMessage(const char* title, const char* description, uint16_t accentColor) {
    auto& lcd = M5.Display;
    lcd.fillScreen(TFT_BLACK);
    lcd.setTextColor(accentColor, TFT_BLACK);
    lcd.setTextSize(4);
    lcd.setCursor(24, 40);
    lcd.print(title);
    lcd.setTextColor(TFT_LIGHTGRAY, TFT_BLACK);
    lcd.setTextSize(3);
    lcd.setCursor(24, 110);
    lcd.print(description);
}

}  // namespace

bool setupTab5Wifi(IDeviceView& view) {
    (void)view;  // status is drawn directly on M5.Display

    String ssid;
    String password;

    if (!loadWifiCredentials(ssid, password)) {
        showWifiMessage("No saved WiFi", "Use USB Serial to setup", TFT_RED);
        delay(5000);
        return false;
    }

    showWifiMessage("Connecting", ssid.c_str(), TFT_GREEN);
    WiFi.begin(ssid.c_str(), password.c_str());

    for (int i = 0; i < 30; ++i) {
        if (WiFi.status() == WL_CONNECTED) {
            return true;
        }
        delay(800);
    }

    showWifiMessage("Failed to connect", "Use USB Serial to setup", TFT_RED);
    delay(4000);
    return false;
}

#endif
